#pragma once

// rcs.h — Recoil Compensation System
// Extracted from aim_common. All logic stays identical; only file boundary changes.
// aim_common.h still forwards the public interface via AimCommon:: aliases.

#include <cstdint>
#include "../../utils/math/vector/vector.h"

class C_CSPlayerPawn;

namespace RCS {

// ── State ────────────────────────────────────────────────────────────────────
// Tracks the SCALED punch already applied to the view (same units as
// GetScaledPunch). Step = SmoothRcsStep(), absolute lag = GetApplied().
// Smoothed toward absolutePunch each frame; Reset() on lock-drop / key-up.

QAngle_t SmoothStep(const QAngle_t& absolutePunch);
QAngle_t GetApplied();
void     Reset();

// ── Helpers (called by aim_common / aim) ────────────────────────────────────

// GetScaledPunch: raw aim-punch * menu rcs_scale_x/y * humanize factor.
// Returns false when not shooting or punch invalid.
bool GetScaledPunch(C_CSPlayerPawn* lp, QAngle_t& out);

// GetFirePunch: full GetRemovedAimPunch for fire-angle stamp.
// Does NOT apply rcs_scale; used by AF/trigger FIRE path.
bool GetFirePunch(C_CSPlayerPawn* lp, QAngle_t& out);

// ReadAimPunch: raw punch read (pattern + schema fallback).
bool ReadAimPunch(C_CSPlayerPawn* lp, QAngle_t& out);

// ReadShotsFired: m_iShotsFired via schema offset.
int  ReadShotsFired(C_CSPlayerPawn* lp);

// ApplyDeltaRcs: standalone delta step applied directly to view angles.
// Called by StandaloneRcs each CreateMove when aimbot is not locking.
bool ApplyDeltaRcs(C_CSPlayerPawn* lp);

// StandaloneRcs: gate check (Config::rcs_standalone, valid weapon) + ApplyDeltaRcs.
bool StandaloneRcs(C_CSPlayerPawn* lp);

// RcsHumanMul: sticky per-time compensation variance [0.90, 1.00].
// Returns 1.0 when humanize slider == 0 so existing configs are unchanged.
float HumanMul();

} // namespace RCS
