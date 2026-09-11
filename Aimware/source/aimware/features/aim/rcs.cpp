#include "rcs.h"
#include "aim_common.h"

#include "../../../cs2/entity/C_CSPlayerPawn/C_CSPlayerPawn.h"
#include "../../../cs2/entity/C_CSWeaponBase/C_CSWeaponBase.h"
#include "../../config/config.h"
#include "../../utils/memory/memsafe/memsafe.h"
#include "../../utils/memory/patternscan/patternscan.h"
#include "../../offsets/offsets.h"
#include "../hitchance/hitchance.h"

#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <cstdint>

// ── Forward declarations for helpers that live in aim_common.cpp ─────────────
// NowMs and the noise PRNG are defined in aim_common (anonymous namespace).
// We access them via the public AimCommon:: symbols declared in aim_common.h.

namespace RCS {

// ── Static state ─────────────────────────────────────────────────────────────
namespace {
    QAngle_t s_applied{};           // scaled punch already in the view
    float    s_humanMul    = 1.f;   // sticky compensation ratio
    uint64_t s_humanMulMs  = 0;     // last refresh timestamp

    // Pattern-resolved GetRemovedAimPunch (same as aim_common path)
    using FnGetRemovedAimPunch = void*(__fastcall*)(void* pawn, QAngle_t* out, char flag);
    static FnGetRemovedAimPunch s_getRemovedAimPunch = nullptr;
    static bool                 s_punchResolved      = false;

    void ResolveGetRemovedAimPunch()
    {
        if (s_punchResolved) return;
        s_punchResolved = true;
        auto* p = M::FindPattern("client",
            "40 53 48 83 EC 20 48 8B 89 ? ? ? ? 48 8B DA E8");
        if (!p)
            p = M::FindPattern("client",
                "40 53 48 83 EC 20 48 8B 89 B8 14 00 00 48 8B DA");
        s_getRemovedAimPunch = reinterpret_cast<FnGetRemovedAimPunch>(p);
    }

    // Simple xorshift noise — separate seed from aim_common so the two PRNG
    // streams don't couple (RCS humanize period != smooth humanize period).
    uint32_t s_noiseSeed = 0xC4A3B12Eu;
    float NoiseUnit()
    {
        uint32_t x = s_noiseSeed;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        s_noiseSeed = x ? x : 0xC4A3B12Eu;
        return (static_cast<float>(s_noiseSeed & 0xFFFFFFu) /
                static_cast<float>(0xFFFFFFu)) * 2.f - 1.f;
    }

} // namespace (private)

// ── Public API ───────────────────────────────────────────────────────────────

float HumanMul()
{
    if (Config::aimbot_humanize < 0.01f)
        return 1.f;

    const float hum = std::clamp(Config::aimbot_humanize, 0.f, 100.f) * 0.01f;
    const uint64_t now = AimCommon::NowMs();
    const uint64_t interval = 100ull
        + static_cast<uint64_t>((NoiseUnit() * 0.5f + 0.5f) * 60.f);

    if (s_humanMulMs == 0 || now >= s_humanMulMs + interval) {
        s_humanMulMs = now;
        s_humanMul = std::clamp(
            1.f - std::fabs(NoiseUnit()) * (0.10f * hum),
            0.90f, 1.f);
    }
    return s_humanMul;
}

bool ReadAimPunch(C_CSPlayerPawn* lp, QAngle_t& out)
{
    if (!lp) return false;
    ResolveGetRemovedAimPunch();

    // Primary: game GetRemovedAimPunch
    if (s_getRemovedAimPunch) {
        QAngle_t punch{};
        bool ok = false;
        __try {
            void* ret = s_getRemovedAimPunch(lp, &punch, 1);
            (void)ret;
            ok = punch.IsValid();
            if (ok) out = punch;
        } __except (EXCEPTION_EXECUTE_HANDLER) { ok = false; }
        if (ok) return true;
    }

    // Fallback: schema AimPunchServices::m_predictableBaseAngle
    __try {
        const uintptr_t base   = reinterpret_cast<uintptr_t>(lp);
        const uint32_t svcOff  = Offset::m_pAimPunchServices();
        const uint32_t angOff  = Offset::m_predictableBaseAngle();
        if (!svcOff || !angOff) return false;
        void* services = *reinterpret_cast<void**>(base + svcOff);
        if (!services || !Mem::IsUserPtr(services)) return false;
        out = *reinterpret_cast<QAngle_t*>(
            reinterpret_cast<uintptr_t>(services) + angOff);
        return out.IsValid();
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

int ReadShotsFired(C_CSPlayerPawn* lp)
{
    if (!lp) return 0;
    int shots = 0;
    __try {
        const uint32_t off = Offset::m_iShotsFired();
        if (!off) return 0;
        shots = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(lp) + off);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
    return shots;
}

bool GetScaledPunch(C_CSPlayerPawn* lp, QAngle_t& out)
{
    QAngle_t punch{};
    if (!HitChance::ReadAimPunch(lp, punch) && !ReadAimPunch(lp, punch))
        return false;
    if (!punch.IsValid() || ReadShotsFired(lp) < 1)
        return false;

    const float sx  = std::clamp(Config::rcs_scale_x, 0.f, 1.f); // yaw
    const float sy  = std::clamp(Config::rcs_scale_y, 0.f, 1.f); // pitch
    const float hum = HumanMul(); // 1.0 when humanize off

    constexpr float kPunchScale = AimCommon::kPunchScale;
    out.x = std::clamp(punch.x, -12.f, 12.f) * kPunchScale * sy * hum;
    out.y = std::clamp(punch.y, -12.f, 12.f) * kPunchScale * sx * hum;
    out.z = 0.f;
    return out.IsValid();
}

bool GetFirePunch(C_CSPlayerPawn* lp, QAngle_t& out)
{
    out = {};
    QAngle_t punch{};
    if (!HitChance::ReadAimPunch(lp, punch) && !ReadAimPunch(lp, punch))
        return false;
    if (!punch.IsValid()) return false;

    constexpr float kPunchScale = AimCommon::kPunchScale;
    out.x = std::clamp(punch.x, -12.f, 12.f) * kPunchScale;
    out.y = std::clamp(punch.y, -12.f, 12.f) * kPunchScale;
    out.z = 0.f;
    return out.IsValid();
}

// ── Smooth tracker ────────────────────────────────────────────────────────────

QAngle_t SmoothStep(const QAngle_t& absolutePunch)
{
    const float sm   = std::clamp(Config::rcs_smooth, 0.f, 20.f);
    const float frac = (sm <= 0.01f) ? 1.f : (1.f / (1.f + sm * 0.5f)); // 20 -> ~0.09

    QAngle_t delta{
        absolutePunch.x - s_applied.x,
        absolutePunch.y - s_applied.y,
        0.f
    };
    // Cap one-frame jump (teleport punch after lock drop / hitch)
    delta.x = std::clamp(delta.x, -2.5f, 2.5f);
    delta.y = std::clamp(delta.y, -2.5f, 2.5f);

    QAngle_t step{ delta.x * frac, delta.y * frac, 0.f };
    s_applied.x += step.x;
    s_applied.y += step.y;
    s_applied.z  = 0.f;
    return step;
}

QAngle_t GetApplied() { return s_applied; }

void Reset()
{
    s_applied  = {};
    s_humanMul = 1.f;
    s_humanMulMs = 0;
}

// ── Delta / Standalone ────────────────────────────────────────────────────────

bool ApplyDeltaRcs(C_CSPlayerPawn* lp)
{
    if (!lp) return false;

    QAngle_t absScaled{};
    if (!GetScaledPunch(lp, absScaled)) {
        // Not shooting — keep baseline for next spray, clear lag state
        QAngle_t raw{};
        if (HitChance::ReadAimPunch(lp, raw) || ReadAimPunch(lp, raw)) {
            AimCommon::g_oldPunch = raw;
            AimCommon::g_hadPunch = true;
        } else {
            AimCommon::g_hadPunch = false;
        }
        Reset();
        return false;
    }

    const int  shots   = ReadShotsFired(lp);
    const bool lmbHeld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!lmbHeld || shots < 1) {
        AimCommon::g_hadPunch = false;
        Reset();
        return false;
    }

    // Seed first sample - do not apply full absolute as delta (view yank)
    if (!AimCommon::g_hadPunch) {
        AimCommon::g_oldPunch = absScaled;
        AimCommon::g_hadPunch = true;
        s_applied             = absScaled; // snap lag tracker
        return false;
    }

    // Recovery: punch settling - track, don't fight recovery
    {
        const float dx = absScaled.x - AimCommon::g_oldPunch.x;
        const float dy = absScaled.y - AimCommon::g_oldPunch.y;
        if (dx < -0.01f && std::fabs(absScaled.x) + 0.02f < std::fabs(AimCommon::g_oldPunch.x)
            && std::fabs(dy) < 0.02f) {
            AimCommon::g_oldPunch = absScaled;
            s_applied             = absScaled;
            return false;
        }
    }

    const QAngle_t step = SmoothStep(absScaled);
    AimCommon::g_oldPunch = absScaled;

    if (std::fabs(step.x) < 0.00005f && std::fabs(step.y) < 0.00005f)
        return false;

    QAngle_t view{};
    if (!AimCommon::GetViewAngles(view))
        return false;

    view.x -= step.x;
    view.y -= step.y;
    view.z  = 0.f;
    view.Normalize();
    view.x = std::clamp(view.x, -89.f, 89.f);
    if (!view.IsValid()) return false;

    AimCommon::SetViewAngles(view);
    return true;
}

bool StandaloneRcs(C_CSPlayerPawn* lp)
{
    if (!Config::rcs_standalone || !lp) return false;

    C_CSWeaponBase* pWpn = lp->GetActiveWeapon();
    if (!pWpn || pWpn->IsNonGunWeapon()) {
        AimCommon::g_hadPunch = false;
        Reset();
        return false;
    }
    return ApplyDeltaRcs(lp);
}

} // namespace RCS
