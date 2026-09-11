#include "menu_ui.h"
#include "logo_texture.h"

#include <cstring>

namespace MenuUI {
namespace {

struct StyleStack {
    int colors = 0;
    int vars   = 0;
    void pushColor(ImGuiCol idx, ImVec4 c) { ImGui::PushStyleColor(idx, c); ++colors; }
    void pushVar(ImGuiStyleVar idx, float v) { ImGui::PushStyleVar(idx, v); ++vars; }
    void pushVar(ImGuiStyleVar idx, ImVec2 v) { ImGui::PushStyleVar(idx, v); ++vars; }
    void pop() {
        if (vars)   { ImGui::PopStyleVar(vars); vars = 0; }
        if (colors) { ImGui::PopStyleColor(colors); colors = 0; }
    }
};

StyleStack g_cardStack;
bool       g_cardOpen = false;
bool       g_cardHasItemWidth = false;
StyleStack g_stripStack;
bool       g_stripOpen = false;
StyleStack g_sideStack;
bool       g_sideOpen = false;
StyleStack g_contentStack;
bool       g_contentOpen = false;

ImDrawList* g_contentDl = nullptr;
struct PairTrack {
    int    frame = -1;
    bool   has   = false;
    ImVec2 min, max;
};
PairTrack g_pair;

struct Anim {
    float open = 0.f;
    int   tab  = 0;
};
Anim g_anim;

inline float Saturate(float x) { return (std::clamp)(x, 0.f, 1.f); }

static ImVec4 Darker(ImVec4 c, float amt) {
    return ImVec4(
        (std::max)(0.f, c.x - amt),
        (std::max)(0.f, c.y - amt),
        (std::max)(0.f, c.z - amt),
        c.w);
}
static ImVec4 Lighter(ImVec4 c, float amt) {
    return ImVec4(
        (std::min)(1.f, c.x + amt),
        (std::min)(1.f, c.y + amt),
        (std::min)(1.f, c.z + amt),
        c.w);
}
static ImVec4 Mix(ImVec4 a, ImVec4 b, float t) {
    t = (std::clamp)(t, 0.f, 1.f);
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}

struct ShellPalette {
    ImVec4 bg, side, card, frame, border, text, muted, track;
};

ShellPalette Palette() {
    ShellPalette p;
    p.bg     = Config::menu_bg;
    p.side   = Config::menu_sidebar_bg;
    p.card   = Config::menu_child_bg;
    p.border = Config::menu_border;
    p.text   = Config::menu_text;
    p.muted  = Config::menu_text_muted;
    p.frame  = Darker(p.card, 0.03f);
    p.track  = Darker(p.bg, 0.03f);
    if (p.border.w < 0.06f) p.border.w = 0.10f;
    if (p.border.w > 0.20f) p.border.w = 0.16f;
    p.text.w  = 1.f;
    p.muted.w = 1.f;

    const float op = (std::clamp)(Config::menu_opacity, 0.55f, 1.f);
    const float t = (op - 0.55f) / 0.45f;
    p.bg.w    = op;
    p.side.w  = 0.58f + 0.32f * t;
    p.card.w  = 0.38f + 0.40f * t;
    p.frame.w = 0.42f + 0.30f * t;
    p.track.w = p.frame.w;
    return p;
}

static int Alpha8(float a) {
    return (int)(std::clamp)(a * 255.f, 0.f, 255.f);
}

void PaintGloss(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding, float strength = 1.f) {
    if (!dl) return;
    const float w = max.x - min.x;
    const float h = max.y - min.y;
    if (w < 6.f || h < 6.f) return;
    strength = Saturate(strength) * Saturate(Config::menu_glass);
    if (strength <= 0.01f) {
        dl->AddRect(min, max, IM_COL32(255, 255, 255, 18), rounding, 0, 1.f);
        return;
    }

    const float sheenH = (std::clamp)(h * 0.22f, 6.f, rounding + 4.f);
    dl->PushClipRect(min, max, true);
    dl->AddRectFilled(
        ImVec2(min.x, min.y),
        ImVec2(max.x, min.y + sheenH),
        IM_COL32(255, 255, 255, Alpha8(0.09f * strength)),
        rounding, ImDrawFlags_RoundCornersTop);
    dl->PopClipRect();
    dl->AddRect(min, max,
        IM_COL32(255, 255, 255, Alpha8(0.08f * strength)), rounding, 0, 1.f);
}

bool DrawValueSlider(const char* label, const char* id, float* v, float vmin, float vmax,
                     const char* fmt, bool asInt) {
    if (!v || !(vmax > vmin))
        return false;

    const float avail = ImGui::GetContentRegionAvail().x;
    const float trackH = 14.f;
    const float barH = 3.f;
    const float grabR = 5.f;

    char val[48];
    if (asInt)
        std::snprintf(val, sizeof(val), fmt ? fmt : "%d", (int)*v);
    else
        std::snprintf(val, sizeof(val), fmt ? fmt : "%.3f", *v);

    const ImVec2 valSz = ImGui::CalcTextSize(val);

    ImGui::PushID(id ? id : label);

    const float x0 = ImGui::GetCursorPosX();
    ImGui::PushStyleColor(ImGuiCol_Text, WithA(TextMuted(), 0.95f));
    ImGui::TextUnformatted(label ? label : "");
    ImGui::PopStyleColor();
    ImGui::SameLine(0.f, 0.f);
    ImGui::SetCursorPosX(x0 + avail - valSz.x);
    ImGui::PushStyleColor(ImGuiCol_Text, TextBright());
    ImGui::TextUnformatted(val);
    ImGui::PopStyleColor();

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##track", ImVec2((std::max)(8.f, avail), trackH));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    bool changed = false;
    if (held) {
        const float t = Saturate((ImGui::GetIO().MousePos.x - p0.x) / (std::max)(1.f, avail));
        float nv = vmin + t * (vmax - vmin);
        if (asInt)
            nv = (float)(int)(nv + (nv >= 0.f ? 0.5f : -0.5f));
        nv = (std::clamp)(nv, vmin, vmax);
        if (nv != *v) {
            *v = nv;
            changed = true;
        }
    }

    const float frac = Saturate((*v - vmin) / (vmax - vmin));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float cy = p0.y + trackH * 0.5f;
    const float y0 = cy - barH * 0.5f;
    const float y1 = cy + barH * 0.5f;
    const float rr = barH * 0.5f;
    dl->AddRectFilled(ImVec2(p0.x, y0), ImVec2(p0.x + avail, y1),
        IM_COL32(255, 255, 255, hovered || held ? 26 : 18), rr);
    const float fillW = avail * frac;
    if (fillW > 0.5f) {
        const ImVec2 fa(p0.x, y0);
        const ImVec2 fb(p0.x + fillW, y1);
        dl->AddRectFilled(fa, fb, AccentU32(held ? 1.f : (hovered ? 0.92f : 0.80f)), rr);
        dl->AddRectFilledMultiColor(fa, ImVec2(fb.x, y0 + barH * 0.45f),
            IM_COL32(255, 255, 255, 40), IM_COL32(255, 255, 255, 40),
            IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 0));
    }

    const float gx = (std::clamp)(p0.x + fillW, p0.x + grabR, p0.x + avail - grabR);
    dl->AddCircleFilled(ImVec2(gx, cy), grabR, IM_COL32(236, 238, 242, 255));
    dl->AddCircle(ImVec2(gx, cy), grabR, AccentU32(held || hovered ? 0.90f : 0.55f), 0, 1.f);

    ImGui::PopID();
    return changed;
}

} // namespace

void AnimTick(bool openTarget) {
    if (!openTarget) {
        g_anim.open = 0.f;
        return;
    }
    float dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.f || dt > 0.05f)
        dt = 1.f / 60.f;
    g_anim.open = Saturate(g_anim.open + dt / 0.06f);
}

bool AnimVisible() { return g_anim.open > 0.001f; }
float OpenAlpha()  { return g_anim.open; }

void NotifyTab(int tab) {
    g_anim.tab = tab;
}

void ApplyTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;
    const ShellPalette p = Palette();
    const ImVec4 accent = Config::menu_accent;
    const float r = (std::clamp)((std::max)(Config::menu_rounding, 2.f), 2.f, 8.f);

    auto tint = [&](float a) { return ImVec4(accent.x, accent.y, accent.z, a); };

    c[ImGuiCol_WindowBg]             = p.bg;
    c[ImGuiCol_ChildBg]              = p.card;
    c[ImGuiCol_PopupBg]              = Darker(p.card, 0.02f);
    c[ImGuiCol_PopupBg].w            = (std::clamp)(p.card.w + 0.06f, 0.86f, 0.98f);
    c[ImGuiCol_Border]               = p.border;
    c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);

    c[ImGuiCol_Text]                 = p.text;
    c[ImGuiCol_TextDisabled]         = p.muted;

    c[ImGuiCol_FrameBg]              = ImVec4(1.f, 1.f, 1.f, 0.050f);
    c[ImGuiCol_FrameBgHovered]       = ImVec4(1.f, 1.f, 1.f, 0.078f);
    c[ImGuiCol_FrameBgActive]        = Mix(ImVec4(1.f, 1.f, 1.f, 0.08f), tint(1.f), 0.22f);
    c[ImGuiCol_FrameBgActive].w      = 0.12f;

    c[ImGuiCol_TitleBg] = c[ImGuiCol_TitleBgActive] = c[ImGuiCol_TitleBgCollapsed] = p.bg;
    c[ImGuiCol_MenuBarBg]            = p.side;

    c[ImGuiCol_ScrollbarBg]          = ImVec4(0, 0, 0, 0.18f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(1.f, 1.f, 1.f, 0.16f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.f, 1.f, 1.f, 0.28f);
    c[ImGuiCol_ScrollbarGrabActive]  = accent;

    c[ImGuiCol_CheckMark]            = accent;
    c[ImGuiCol_CheckboxSelectedBg]   = tint(0.55f);
    c[ImGuiCol_SliderGrab]           = accent;
    c[ImGuiCol_SliderGrabActive]     = Lighter(accent, 0.12f);

    c[ImGuiCol_Button]               = ImVec4(1.f, 1.f, 1.f, 0.065f);
    c[ImGuiCol_ButtonHovered]        = Mix(ImVec4(1.f, 1.f, 1.f, 0.10f), tint(1.f), 0.28f);
    c[ImGuiCol_ButtonHovered].w      = 0.18f;
    c[ImGuiCol_ButtonActive]         = tint(0.38f);

    c[ImGuiCol_Header]               = ImVec4(1.f, 1.f, 1.f, 0.04f);
    c[ImGuiCol_HeaderHovered]        = tint(0.18f);
    c[ImGuiCol_HeaderActive]         = tint(0.28f);

    c[ImGuiCol_Separator]            = ImVec4(1.f, 1.f, 1.f, 0.08f);
    c[ImGuiCol_SeparatorHovered]     = tint(0.40f);
    c[ImGuiCol_SeparatorActive]      = accent;

    c[ImGuiCol_ResizeGrip]           = ImVec4(1.f, 1.f, 1.f, 0.04f);
    c[ImGuiCol_ResizeGripHovered]    = tint(0.22f);
    c[ImGuiCol_ResizeGripActive]     = tint(0.36f);

    c[ImGuiCol_Tab]                  = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TabHovered]           = tint(0.14f);
    c[ImGuiCol_TabSelected]          = tint(0.12f);
    c[ImGuiCol_TabSelectedOverline]  = accent;
    c[ImGuiCol_TabDimmed]            = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TabDimmedSelected]    = tint(0.08f);

    c[ImGuiCol_TextSelectedBg]       = tint(0.28f);
    c[ImGuiCol_NavCursor]            = accent;
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.02f, 0.02f, 0.03f, 0.50f);

    c[ImGuiCol_TableHeaderBg]        = ImVec4(1.f, 1.f, 1.f, 0.04f);
    c[ImGuiCol_TableBorderStrong]    = ImVec4(1.f, 1.f, 1.f, 0.08f);
    c[ImGuiCol_TableBorderLight]     = ImVec4(1.f, 1.f, 1.f, 0.04f);
    c[ImGuiCol_TableRowBg]           = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt]        = ImVec4(1.f, 1.f, 1.f, 0.015f);

    style.WindowRounding    = r;
    style.ChildRounding     = (std::max)(3.f, r - 1.f);
    style.FrameRounding     = 4.f;
    style.PopupRounding     = (std::max)(4.f, r - 1.f);
    style.ScrollbarRounding = 4.f;
    style.GrabRounding      = 4.f;
    style.TabRounding       = 4.f;

    const bool compact = Config::menu_compact;
    const float dpi = Layout::DpiMul();
    style.WindowPadding     = ImVec2(0, 0);
    style.FramePadding      = compact ? ImVec2(6, 3) : ImVec2(8, 4);
    style.ItemSpacing       = compact ? ImVec2(6, 4) : ImVec2(8, 5);
    style.ItemInnerSpacing  = ImVec2(5, 3);
    style.CellPadding       = ImVec2(4, 3);
    style.IndentSpacing     = 10.0f;

    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    style.ScrollbarSize     = 5.0f;
    style.GrabMinSize       = 8.0f;
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);

    if (dpi > 1.001f) {
        style.FramePadding.x     = floorf(style.FramePadding.x * dpi);
        style.FramePadding.y     = floorf(style.FramePadding.y * dpi);
        style.ItemSpacing.x      = floorf(style.ItemSpacing.x * dpi);
        style.ItemSpacing.y      = floorf(style.ItemSpacing.y * dpi);
        style.ItemInnerSpacing.x = floorf(style.ItemInnerSpacing.x * dpi);
        style.ItemInnerSpacing.y = floorf(style.ItemInnerSpacing.y * dpi);
        style.CellPadding.x      = floorf(style.CellPadding.x * dpi);
        style.CellPadding.y      = floorf(style.CellPadding.y * dpi);
        style.IndentSpacing      = floorf(style.IndentSpacing * dpi);
        style.ScrollbarSize      = floorf(style.ScrollbarSize * dpi);
        style.GrabMinSize        = floorf(style.GrabMinSize * dpi);
    }
    ImGui::GetIO().FontGlobalScale = 1.f;

    style.AntiAliasedLines  = true;
    style.AntiAliasedFill   = true;
    style.Alpha             = 1.0f;
    style.DisabledAlpha     = 0.40f;
}

void ApplyPreset(int idx) {
    Config::menu_preset         = idx;
    Config::menu_compact        = true;
    Config::menu_widgets_follow = true;

    switch (idx) {
    case 0: // Midnight OLED (Pure deep black + electric cyan)
        Config::menu_bg         = ImVec4(0.015f, 0.015f, 0.020f, 0.94f);
        Config::menu_child_bg   = ImVec4(0.055f, 0.060f, 0.075f, 0.70f);
        Config::menu_sidebar_bg = ImVec4(0.010f, 0.010f, 0.015f, 0.88f);
        Config::menu_border     = ImVec4(0.20f, 0.25f, 0.35f, 0.22f);
        Config::menu_text       = ImVec4(0.95f, 0.97f, 1.00f, 1.f);
        Config::menu_text_muted = ImVec4(0.48f, 0.52f, 0.60f, 1.f);
        Config::menu_accent     = ImVec4(0.20f, 0.80f, 1.00f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.94f;
        Config::menu_glass      = 0.40f;
        break;
    case 1: // Steel Slate (Modern dark slate + ice sky blue)
    default:
        Config::menu_bg         = ImVec4(0.070f, 0.074f, 0.090f, 0.88f);
        Config::menu_child_bg   = ImVec4(0.130f, 0.138f, 0.160f, 0.60f);
        Config::menu_sidebar_bg = ImVec4(0.040f, 0.044f, 0.056f, 0.80f);
        Config::menu_border     = ImVec4(1.0f, 1.0f, 1.0f, 0.12f);
        Config::menu_text       = ImVec4(0.94f, 0.95f, 0.97f, 1.f);
        Config::menu_text_muted = ImVec4(0.56f, 0.58f, 0.64f, 1.f);
        Config::menu_accent     = ImVec4(0.42f, 0.68f, 0.92f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.88f;
        Config::menu_glass      = 0.65f;
        break;
    case 2: // Nordic Frost (Arctic charcoal + frost ice blue)
        Config::menu_bg         = ImVec4(0.110f, 0.125f, 0.160f, 0.90f);
        Config::menu_child_bg   = ImVec4(0.160f, 0.185f, 0.230f, 0.65f);
        Config::menu_sidebar_bg = ImVec4(0.080f, 0.095f, 0.125f, 0.85f);
        Config::menu_border     = ImVec4(0.55f, 0.70f, 0.85f, 0.18f);
        Config::menu_text       = ImVec4(0.92f, 0.95f, 0.98f, 1.f);
        Config::menu_text_muted = ImVec4(0.55f, 0.62f, 0.70f, 1.f);
        Config::menu_accent     = ImVec4(0.53f, 0.75f, 0.92f, 1.f);
        Config::menu_rounding   = 7.0f;
        Config::menu_opacity    = 0.90f;
        Config::menu_glass      = 0.75f;
        break;
    case 3: // Cyberpunk Neon (Deep obsidian violet + electric magenta purple)
        Config::menu_bg         = ImVec4(0.055f, 0.040f, 0.075f, 0.92f);
        Config::menu_child_bg   = ImVec4(0.110f, 0.080f, 0.150f, 0.62f);
        Config::menu_sidebar_bg = ImVec4(0.035f, 0.025f, 0.050f, 0.86f);
        Config::menu_border     = ImVec4(0.75f, 0.40f, 0.95f, 0.22f);
        Config::menu_text       = ImVec4(0.96f, 0.93f, 0.98f, 1.f);
        Config::menu_text_muted = ImVec4(0.62f, 0.52f, 0.68f, 1.f);
        Config::menu_accent     = ImVec4(0.78f, 0.35f, 0.98f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.92f;
        Config::menu_glass      = 0.70f;
        break;
    case 4: // Emerald Matrix (Deep dark moss + vivid emerald green)
        Config::menu_bg         = ImVec4(0.045f, 0.065f, 0.050f, 0.90f);
        Config::menu_child_bg   = ImVec4(0.090f, 0.130f, 0.100f, 0.60f);
        Config::menu_sidebar_bg = ImVec4(0.025f, 0.040f, 0.030f, 0.85f);
        Config::menu_border     = ImVec4(0.30f, 0.85f, 0.50f, 0.18f);
        Config::menu_text       = ImVec4(0.93f, 0.97f, 0.94f, 1.f);
        Config::menu_text_muted = ImVec4(0.50f, 0.64f, 0.54f, 1.f);
        Config::menu_accent     = ImVec4(0.24f, 0.88f, 0.54f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.90f;
        Config::menu_glass      = 0.60f;
        break;
    case 5: // Sunset Crimson (Volcanic dark slate + warm coral crimson)
        Config::menu_bg         = ImVec4(0.075f, 0.045f, 0.045f, 0.90f);
        Config::menu_child_bg   = ImVec4(0.145f, 0.090f, 0.090f, 0.60f);
        Config::menu_sidebar_bg = ImVec4(0.050f, 0.028f, 0.028f, 0.85f);
        Config::menu_border     = ImVec4(0.95f, 0.40f, 0.35f, 0.18f);
        Config::menu_text       = ImVec4(0.98f, 0.94f, 0.94f, 1.f);
        Config::menu_text_muted = ImVec4(0.68f, 0.54f, 0.54f, 1.f);
        Config::menu_accent     = ImVec4(0.96f, 0.38f, 0.32f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.90f;
        Config::menu_glass      = 0.65f;
        break;
    case 6: // Dracula Velvet (Dracula slate + iconic velvet rose pink)
        Config::menu_bg         = ImVec4(0.105f, 0.100f, 0.135f, 0.90f);
        Config::menu_child_bg   = ImVec4(0.160f, 0.150f, 0.205f, 0.65f);
        Config::menu_sidebar_bg = ImVec4(0.075f, 0.070f, 0.095f, 0.85f);
        Config::menu_border     = ImVec4(0.85f, 0.50f, 0.75f, 0.18f);
        Config::menu_text       = ImVec4(0.96f, 0.95f, 0.98f, 1.f);
        Config::menu_text_muted = ImVec4(0.62f, 0.58f, 0.68f, 1.f);
        Config::menu_accent     = ImVec4(0.96f, 0.45f, 0.72f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.90f;
        Config::menu_glass      = 0.65f;
        break;
    case 7: // Solar Luxury Gold (Warm obsidian charcoal + luxury amber gold)
        Config::menu_bg         = ImVec4(0.065f, 0.060f, 0.050f, 0.90f);
        Config::menu_child_bg   = ImVec4(0.135f, 0.125f, 0.105f, 0.60f);
        Config::menu_sidebar_bg = ImVec4(0.040f, 0.035f, 0.030f, 0.85f);
        Config::menu_border     = ImVec4(0.90f, 0.75f, 0.30f, 0.18f);
        Config::menu_text       = ImVec4(0.98f, 0.96f, 0.92f, 1.f);
        Config::menu_text_muted = ImVec4(0.66f, 0.62f, 0.52f, 1.f);
        Config::menu_accent     = ImVec4(0.98f, 0.78f, 0.22f, 1.f);
        Config::menu_rounding   = 6.0f;
        Config::menu_opacity    = 0.90f;
        Config::menu_glass      = 0.65f;
        break;
    }
    ApplyTheme();
}

void DrawWindowFrame(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding) {
    if (!dl) return;
    PaintGloss(dl, min, max, rounding, 1.f);
    const float inset = (std::max)(rounding, 5.f);
    dl->AddRectFilled(
        ImVec2(min.x + inset, min.y + 1.f),
        ImVec2(max.x - inset, min.y + 3.f),
        AccentU32(0.95f), 1.f);
}

void Section(const char* label) {
    if (!label) label = "";
    const bool compact = Config::menu_compact;
    ImGui::Dummy(ImVec2(0, compact ? 5.f : 7.f));
    ImGui::PushStyleColor(ImGuiCol_Text, WithA(TextMuted(), 0.95f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, compact ? 2.f : 3.f));
}

void Gap(float mult) {
    ImGui::Dummy(ImVec2(0, ImGui::GetStyle().ItemSpacing.y * mult));
}

void SoftSeparator() {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(p.x, p.y), ImVec2(p.x + w, p.y),
        IM_COL32(255, 255, 255, 18), 1.f);
    ImGui::Dummy(ImVec2(0, Config::menu_compact ? 5.f : 6.f));
}

void BeginCard(const char* id, float width, bool autoY) {
    const Layout L = Layout::Current();
    const ShellPalette p = Palette();
    float w = width;
    if (w > 0.f) {
        const float avail = ImGui::GetContentRegionAvail().x;
        if (w > avail) w = (std::max)(1.f, avail);
    }

    g_cardStack = {};
    g_cardStack.pushColor(ImGuiCol_ChildBg, p.card);
    g_cardStack.pushColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.06f));
    g_cardStack.pushVar(ImGuiStyleVar_ChildRounding, L.childRound);
    g_cardStack.pushVar(ImGuiStyleVar_WindowPadding, ImVec2(L.cardPad, L.cardPad));
    g_cardStack.pushVar(ImGuiStyleVar_ItemSpacing,
        Config::menu_compact ? ImVec2(6.f, 4.f) : ImVec2(8.f, 5.f));

    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (autoY) flags |= ImGuiChildFlags_AutoResizeY;

    ImGui::BeginChild(id, ImVec2(w, 0), flags, ImGuiWindowFlags_None);
    g_cardOpen = true;
    ImGui::PushItemWidth(-1.f);
    g_cardHasItemWidth = true;
}

void EndCard() {
    if (g_cardHasItemWidth) { ImGui::PopItemWidth(); g_cardHasItemWidth = false; }
    if (g_cardOpen) {
        ImVec2 min = ImGui::GetWindowPos();
        ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
        ImGui::EndChild();
        g_cardOpen = false;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (dl && (max.x - min.x) > 4.f && (max.y - min.y) > 4.f)
            PaintGloss(dl, min, max, Layout::Current().childRound, 0.45f);

        const int frm = ImGui::GetFrameCount();
        if (g_pair.frame != frm) { g_pair = PairTrack{}; g_pair.frame = frm; }
        if (!g_pair.has) {
            g_pair.has = true; g_pair.min = min; g_pair.max = max;
        } else if (std::fabsf(min.y - g_pair.min.y) < 3.f) {
            ImDrawList* pdl = g_contentDl ? g_contentDl : dl;
            const float hA = g_pair.max.y - g_pair.min.y;
            const float hB = max.y - min.y;
            ImVec2 eMin, eMax;
            if (hB > hA + 1.f) {
                eMin = ImVec2(g_pair.min.x, g_pair.max.y - 1.f);
                eMax = ImVec2(g_pair.max.x, g_pair.max.y + hB - hA);
            } else if (hA > hB + 1.f) {
                eMin = ImVec2(min.x, max.y - 1.f);
                eMax = ImVec2(max.x, max.y + hA - hB);
            } else {
                eMin = eMax = ImVec2(0, 0);
            }
            if (pdl && eMax.y > eMin.y + 1.f && eMax.x > eMin.x + 1.f) {
                const ShellPalette p = Palette();
                const float cr = Layout::Current().childRound;
                pdl->AddRectFilled(eMin, eMax, ToU32(p.card), cr, ImDrawFlags_RoundCornersBottom);
                const ImU32 bc = ToU32(ImVec4(1.f, 1.f, 1.f, 0.06f));
                pdl->AddLine(ImVec2(eMin.x, eMin.y), ImVec2(eMin.x, eMax.y), bc, 1.f);
                pdl->AddLine(ImVec2(eMax.x - 1.f, eMin.y), ImVec2(eMax.x - 1.f, eMax.y), bc, 1.f);
                pdl->AddLine(ImVec2(eMin.x, eMax.y - 1.f), ImVec2(eMax.x, eMax.y - 1.f), bc, 1.f);
            }
            g_pair.has = false;
        } else {
            g_pair.has = true; g_pair.min = min; g_pair.max = max;
        }
    }
    g_cardStack.pop();
}

void BeginStrip(const char* id) {
    const Layout L = Layout::Current();
    const ShellPalette p = Palette();
    g_stripStack = {};
    g_stripStack.pushColor(ImGuiCol_ChildBg, p.card);
    g_stripStack.pushColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.06f));
    g_stripStack.pushVar(ImGuiStyleVar_ChildRounding, L.childRound);
    g_stripStack.pushVar(ImGuiStyleVar_WindowPadding,
        ImVec2(L.compact ? 8.f : 10.f, L.compact ? 6.f : 8.f));
    ImGui::BeginChild(id, ImVec2(-1.f, 0.f),
        ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding,
        ImGuiWindowFlags_None);
    g_stripOpen = true;
}

void EndStrip() {
    if (g_stripOpen) {
        ImVec2 min = ImGui::GetWindowPos();
        ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
        ImGui::EndChild();
        g_stripOpen = false;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (dl && (max.x - min.x) > 4.f && (max.y - min.y) > 4.f)
            PaintGloss(dl, min, max, Layout::Current().childRound, 0.45f);
    }
    g_stripStack.pop();
}

bool Slider(const char* label, const char* id, float* v, float vmin, float vmax, const char* fmt) {
    return DrawValueSlider(label, id, v, vmin, vmax, fmt, false);
}

bool SliderInt(const char* label, const char* id, int* v, int vmin, int vmax, const char* fmt) {
    if (!v)
        return false;
    float fv = (float)*v;
    const bool changed = DrawValueSlider(label, id, &fv, (float)vmin, (float)vmax, fmt, true);
    if (changed)
        *v = (int)fv;
    return changed;
}

bool Combo(const char* label, const char* id, int* cur, const char* const items[], int count) {
    ImGui::PushStyleColor(ImGuiCol_Text, WithA(TextMuted(), 0.95f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-1.f);
    return ImGui::Combo(id, cur, items, count);
}

void Tip(const char* text) {
    if (text && text[0] && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", text);
}

bool FeatureToggle(const char* label, bool* v, const char* tip) {
    if (!v)
        return false;
    const bool compact = Config::menu_compact;
    const float rowH = compact ? 22.f : 24.f;
    const float swW = compact ? 32.f : 36.f;
    const float swH = compact ? 16.f : 18.f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const ImVec2 p0 = ImGui::GetCursorScreenPos();

    ImGui::PushID(label ? label : "ft");
    ImGui::InvisibleButton("##hit", ImVec2((std::max)(8.f, avail), rowH));
    const bool hovered = ImGui::IsItemHovered();
    const bool pressed = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    if (pressed)
        *v = !*v;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float midY = p0.y + rowH * 0.5f;
    const float swX = p0.x + avail - swW;
    const float swY = midY - swH * 0.5f;
    const float knob = swH - 4.f;
    const float rr = swH * 0.5f;
    const ImU32 track = *v
        ? AccentU32(hovered ? 0.95f : 0.82f)
        : IM_COL32(255, 255, 255, hovered ? 34 : 22);
    dl->AddRectFilled(ImVec2(swX, swY), ImVec2(swX + swW, swY + swH), track, rr);
    if (*v)
        dl->AddRectFilledMultiColor(
            ImVec2(swX, swY), ImVec2(swX + swW, swY + swH * 0.45f),
            IM_COL32(255, 255, 255, 36), IM_COL32(255, 255, 255, 36),
            IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 0));
    const float kx = swX + 2.f + (*v ? (swW - knob - 4.f) : 0.f);
    const ImVec2 kc(kx + knob * 0.5f, midY);
    dl->AddCircleFilled(kc, knob * 0.5f, IM_COL32(236, 238, 242, 250));

    if (label && label[0]) {
        const ImU32 tc = *v ? ToU32(TextBright()) : ToU32(WithA(TextMuted(), 0.95f));
        const ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(p0.x, midY - ts.y * 0.5f), tc, label);
    }

    if (hovered) {
        if (tip && tip[0])
            ImGui::SetTooltip("%s\nRight-click for settings.", tip);
        else
            ImGui::SetTooltip("Right-click for settings.");
    }
    ImGui::PopID();
    return pressed;
}

void PopupTitle(const char* title) {
    ImGui::PushStyleColor(ImGuiCol_Text, TextBright());
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    SoftSeparator();
}

bool NavIcon(const char* label, const char* iconUtf8, bool selected, const ImVec2& size, int index) {
    (void)index;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 0));

    char idBuf[64];
    std::snprintf(idBuf, sizeof(idBuf), "##nav_%s", label ? label : "x");
    const bool pressed = ImGui::Button(idBuf, size);
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const bool hovered = ImGui::IsItemHovered();
    const ImVec2 a(min.x + 2.f, min.y + 2.f);
    const ImVec2 b(max.x - 2.f, max.y - 2.f);
    const float rr = 6.f;

    if (hovered && !selected)
        dl->AddRectFilled(a, b, IM_COL32(255, 255, 255, 12), rr);
    if (selected)
        dl->AddRectFilled(ImVec2(a.x, a.y + 6.f), ImVec2(a.x + 2.f, b.y - 6.f),
            AccentU32(1.f), 1.f);

    const ImU32 iconCol = selected ? AccentU32(1.f)
        : (hovered ? ToU32(TextBright()) : ToU32(WithA(TextMuted(), 0.78f)));

    if (iconUtf8 && iconUtf8[0] && g_MenuIconFont) {
        const float iconSz = 16.f;
        const ImVec2 isz = g_MenuIconFont->CalcTextSizeA(iconSz, FLT_MAX, 0.f, iconUtf8);
        const float cx = floorf((min.x + max.x) * 0.5f - isz.x * 0.5f);
        float cy = floorf((min.y + max.y) * 0.5f - isz.y * 0.5f);
        if (Config::menu_sidebar_labels && label && label[0]) {
            ImFont* font = ImGui::GetFont();
            // Draw at native atlas size - downscaling to a fixed 12px blurs glyphs.
            // Cap at 18 so labels never collide with the icon row (navBtnH 48).
            const float ls = (std::min)(ImGui::GetFontSize(), 18.f);
            const ImVec2 lsz = font->CalcTextSizeA(ls, FLT_MAX, 0.f, label);
            const float lx = floorf((min.x + max.x) * 0.5f - lsz.x * 0.5f);
            const float ly = floorf(max.y - lsz.y - 4.f);
            dl->AddText(font, ls, ImVec2(lx, ly), iconCol, label);
            cy = floorf(min.y + (max.y - min.y) * 0.42f - isz.y * 0.5f);
        }
        dl->AddText(g_MenuIconFont, iconSz, ImVec2(cx, cy), iconCol, iconUtf8);
    }

    if (label && label[0] && !Config::menu_sidebar_labels
        && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("%s", label);

    return pressed;
}

bool NavButton(const char* label, const char* iconUtf8, bool selected, const ImVec2& size, int index) {
    if (size.x > 0.f && size.x <= 80.f && iconUtf8 && iconUtf8[0])
        return NavIcon(label, iconUtf8, selected, size, index);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 1.f, 1.f, 0.05f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 1.f, 0.08f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 0));

    char idBuf[64];
    std::snprintf(idBuf, sizeof(idBuf), "##nav_%s", label ? label : "x");
    const bool pressed = ImGui::Button(idBuf, size);
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const bool hovered = ImGui::IsItemHovered();
    if (selected)
        dl->AddRectFilled(ImVec2(min.x + 2.f, min.y + 6.f),
            ImVec2(min.x + 4.f, max.y - 6.f), AccentU32(1.f), 1.f);
    const ImU32 col = selected || hovered ? ToU32(TextBright()) : ToU32(TextMuted());
    const ImU32 iconCol = selected ? AccentU32(1.f) : col;

    float x = min.x + 10.f;
    const float midY = (min.y + max.y) * 0.5f;
    if (iconUtf8 && iconUtf8[0] && g_MenuIconFont) {
        const float iconSz = 16.f;
        const ImVec2 isz = g_MenuIconFont->CalcTextSizeA(iconSz, FLT_MAX, 0.f, iconUtf8);
        dl->AddText(g_MenuIconFont, iconSz, ImVec2(x, midY - isz.y * 0.5f), iconCol, iconUtf8);
        x += isz.x + 8.f;
    }
    if (label && label[0]) {
        ImFont* font = ImGui::GetFont();
        const float fs = ImGui::GetFontSize();
        const ImVec2 tsz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, label);
        dl->AddText(font, fs, ImVec2(x, midY - tsz.y * 0.5f), col, label);
    }
    return pressed;
}

bool NavButton(const char* label, bool selected, const ImVec2& size, int index) {
    return NavButton(label, nullptr, selected, size, index);
}

void SubNav(const char* const* labels, int count, int* selected) {
    if (!labels || count <= 0 || !selected) return;

    static const char* s_selKey = nullptr;
    static int s_selVal = -1;
    if (s_selKey != labels[0] || s_selVal != *selected) {
        s_selKey = labels[0];
        s_selVal = *selected;
        ImGui::SetScrollY(0.f);
    }

    const Layout L = Layout::Current();
    const float h = L.subNavH;
    const float avail = ImGui::GetContentRegionAvail().x;
    const ImVec2 trackMin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddLine(
        ImVec2(trackMin.x, trackMin.y + h - 0.5f),
        ImVec2(trackMin.x + avail, trackMin.y + h - 0.5f),
        IM_COL32(255, 255, 255, 16), 1.f);

    const float baseW = floorf(avail / (float)count);
    const float lastW = (std::max)(1.f, avail - baseW * (count - 1));

    ImGui::SetCursorScreenPos(trackMin);
    ImGui::BeginGroup();
    for (int i = 0; i < count; ++i) {
        if (i > 0) ImGui::SameLine(0, 0.f);
        const float w = (i == count - 1) ? lastW : baseW;
        const bool sel = (*selected == i);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 1.f, 1.f, 0.04f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 1.f, 0.06f));
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? TextBright() : TextMuted());

        char idBuf[64];
        std::snprintf(idBuf, sizeof(idBuf), "%s##sub_%d", labels[i], i);
        if (ImGui::Button(idBuf, ImVec2(w, h)))
            *selected = i;

        if (sel) {
            const ImVec2 bmin = ImGui::GetItemRectMin();
            const ImVec2 bmax = ImGui::GetItemRectMax();
            dl->AddRectFilled(
                ImVec2(bmin.x + 10.f, bmax.y - 2.f),
                ImVec2(bmax.x - 10.f, bmax.y),
                AccentU32(0.95f), 1.f);
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
    }
    ImGui::EndGroup();
    ImGui::SetCursorScreenPos(ImVec2(trackMin.x, trackMin.y + h + (Config::menu_compact ? 5.f : 7.f)));
    ImGui::Dummy(ImVec2(0, 0));
}

void BeginSidebar(const Layout& L, float height) {
    const ShellPalette p = Palette();
    g_sideStack = {};
    g_sideStack.pushColor(ImGuiCol_ChildBg, p.side);
    g_sideStack.pushColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.05f));
    g_sideStack.pushVar(ImGuiStyleVar_ChildRounding, L.childRound);
    g_sideStack.pushVar(ImGuiStyleVar_WindowPadding,
        L.compact ? ImVec2(4.f, 6.f) : ImVec2(6.f, 8.f));
    g_sideStack.pushVar(ImGuiStyleVar_ItemSpacing,
        ImVec2(0.f, L.compact ? 3.f : 4.f));

    ImGui::BeginChild("##sidebar", ImVec2(L.sidebar, height),
        ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
        ImGuiWindowFlags_NoScrollbar);
    g_sideOpen = true;
}

void EndSidebar() {
    if (g_sideOpen) { ImGui::EndChild(); g_sideOpen = false; }
    g_sideStack.pop();
}

void BeginContent(const Layout& L, float contentW, float contentH) {
    const ShellPalette p = Palette();
    g_contentStack = {};
    g_contentStack.pushVar(ImGuiStyleVar_WindowPadding, ImVec2(L.contentPad, L.contentPad));
    g_contentStack.pushVar(ImGuiStyleVar_ChildRounding, L.childRound);
    g_contentStack.pushColor(ImGuiCol_ChildBg, WithA(p.bg, 0.0f));
    g_contentStack.pushColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    ImGui::BeginChild("##content", ImVec2(contentW, contentH),
        ImGuiChildFlags_AlwaysUseWindowPadding,
        ImGuiWindowFlags_None);
    g_contentOpen = true;
    g_contentDl = ImGui::GetWindowDrawList();
}

void EndContent() {
    if (g_contentOpen) { ImGui::EndChild(); g_contentOpen = false; }
    g_contentStack.pop();
}

namespace {
bool s_headerDrag = false;
}

void BeginHeader(const Layout& L, float windowW) {
    const ImVec2 wp = ImGui::GetWindowPos();
    const float h = L.headerH;
    ImGui::SetCursorPos(ImVec2(0.f, 0.f));
    ImGui::InvisibleButton("##win_drag", ImVec2(windowW, h));
    const bool hover = ImGui::IsItemHovered();
    const bool held  = ImGui::IsItemActive();

    if (hover && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        const ImGuiIO& io = ImGui::GetIO();
        const ImVec2 ws = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2((io.DisplaySize.x - ws.x) * 0.5f, (io.DisplaySize.y - ws.y) * 0.5f));
        s_headerDrag = false;
    } else if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        s_headerDrag = true;
    }
    if (s_headerDrag && held) {
        const ImGuiIO& io = ImGui::GetIO();
        const ImVec2 w0 = ImGui::GetWindowPos();
        ImGui::SetWindowPos(ImVec2(w0.x + io.MouseDelta.x, w0.y + io.MouseDelta.y));
    } else if (!held) {
        s_headerDrag = false;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(
        ImVec2(wp.x + L.shellPad, wp.y + h),
        ImVec2(wp.x + windowW - L.shellPad, wp.y + h),
        IM_COL32(255, 255, 255, 18), 1.f);

    ImGui::SetCursorPos(ImVec2(L.contentPad, (h - ImGui::GetFontSize()) * 0.5f));
}

void HeaderBrand(const char* name, const char* accentSuffix, const char* badge) {
    // Try to draw the logo image instead of plain text.
    {
        ID3D11ShaderResourceView* srv = LogoTexture::GetSRV();
        if (srv) {
            const Layout L = Layout::Current();
            // Scale to fit inside the header height with a small margin.
            const float imgH = (std::max)(12.f, L.headerH - 6.f);
            const ImVec2 sz  = LogoTexture::GetSize();
            const float  imgW = (sz.y > 0.f)
                ? std::floor(sz.x / sz.y * imgH)
                : imgH;
            // Vertically center inside the header row.
            const ImVec2 cp = ImGui::GetCursorPos();
            ImGui::SetCursorPosY(cp.y + (L.headerH - imgH) * 0.5f - ImGui::GetStyle().WindowPadding.y);
            ImGui::Image(reinterpret_cast<ImTextureID>(srv), ImVec2(imgW, imgH));
            // Restore Y for any SameLine items that follow.
            ImGui::SetCursorPosY(cp.y);
            // badge still rendered if provided
            if (badge && badge[0]) {
                ImGui::SameLine(0, 8.f);
                ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
                ImGui::TextUnformatted(badge);
                ImGui::PopStyleColor();
            }
            return;
        }
    }
    // Fallback: plain text brand.
    if (name && name[0]) {
        ImGui::PushStyleColor(ImGuiCol_Text, TextBright());
        ImGui::TextUnformatted(name);
        ImGui::PopStyleColor();
    }
    if (accentSuffix && accentSuffix[0]) {
        ImGui::SameLine(0, 6.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Accent());
        ImGui::TextUnformatted(accentSuffix);
        ImGui::PopStyleColor();
    }
    if (badge && badge[0]) {
        ImGui::SameLine(0, 8.f);
        ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
        ImGui::TextUnformatted(badge);
        ImGui::PopStyleColor();
    }
}

void HeaderRightHint(const char* text) {
    if (!text || !text[0]) return;
    const Layout L = Layout::Current();
    const ImVec2 ws = ImGui::GetWindowSize();
    const ImVec2 ts = ImGui::CalcTextSize(text);
    ImGui::SetCursorPos(ImVec2(ws.x - ts.x - L.contentPad, (L.headerH - ts.y) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, WithA(TextMuted(), 0.85f));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

} // namespace MenuUI
