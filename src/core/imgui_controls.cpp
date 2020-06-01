#include "imgui_controls.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

namespace ImGui {

int EnvelopeEditor(float width, float height, float P[6], int type){
    // visuals
    enum { LINE_WIDTH = 3 }; // handlers: small lines width
    enum { GRAB_RADIUS = 6 }; // handlers: circle radius
    enum { AREA_CONSTRAINED = true }; // should grabbers be constrained to grid area?


    ImVec2 Q_AHR[4] = { { 0, 1 }, { P[0], P[1] }, { P[2], P[3] }, { 1, 1 }};
    ImVec2 Q_ADSR[5] = { { 0, 1 }, { P[0], P[1] }, { P[2], P[3] }, { P[4], P[5] }, { 1, 1 } };

    const ImGuiStyle& Style = GetStyle();
    const ImGuiIO& IO = GetIO();
    ImDrawList* DrawList = GetWindowDrawList();
    ImGuiWindow* Window = GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    // prepare canvas
    const float avail = GetContentRegionAvailWidth();
    const float dim = width > 0 ? width : avail;
    ImVec2 Canvas(dim, height);

    ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
    ItemSize(bb);

    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

    // background grid
    for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4)) {
        DrawList->AddLine(
                    ImVec2(bb.Min.x + i, bb.Min.y),
                    ImVec2(bb.Min.x + i, bb.Max.y),
                    GetColorU32(ImGuiCol_TextDisabled));
    }
    for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4)) {
        DrawList->AddLine(
                    ImVec2(bb.Min.x, bb.Min.y + i),
                    ImVec2(bb.Max.x, bb.Min.y + i),
                    GetColorU32(ImGuiCol_TextDisabled));
    }

    // draw lines and grabbers
    float luma = IsItemActive() || IsItemHovered() ? 0.5f : 1.0f;
    ImVec4 line_white(GetStyle().Colors[ImGuiCol_Text]);
    ImVec4 grab_white(1.0f, 1.0f, 1.0f, luma);

    if(type == ImGuiEnvelopeEditorType_AHR){
        ImVec2 p1 = Q_AHR[0] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p2 = Q_AHR[1] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p3 = Q_AHR[2] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p4 = Q_AHR[3] * (bb.Max - bb.Min) + bb.Min;

        DrawList->AddLine(p1, p2, ImColor(line_white), LINE_WIDTH);
        DrawList->AddLine(p2, p3, ImColor(line_white), LINE_WIDTH);
        DrawList->AddLine(p3, p4, ImColor(line_white), LINE_WIDTH);

        DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(grab_white), 6);
        DrawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(grab_white), 6);


    }else if(type == ImGuiEnvelopeEditorType_ADSR){
        ImVec2 p1 = Q_ADSR[0] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p2 = Q_ADSR[1] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p3 = Q_ADSR[2] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p4 = Q_ADSR[3] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p5 = Q_ADSR[4] * (bb.Max - bb.Min) + bb.Min;

        DrawList->AddLine(p1, p2, ImColor(line_white), LINE_WIDTH);
        DrawList->AddLine(p2, p3, ImColor(line_white), LINE_WIDTH);
        DrawList->AddLine(p3, p4, ImColor(line_white), LINE_WIDTH);
        DrawList->AddLine(p4, p5, ImColor(line_white), LINE_WIDTH);

        DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(grab_white), 6);
        DrawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(grab_white), 6);
        DrawList->AddCircleFilled(p4, GRAB_RADIUS, ImColor(grab_white), 6);
    }

    return 0;
}

bool KnobFloat(const char* label, float* p_value, float v_min, float v_max,float v_step) {
    //@ocornut https://github.com/ocornut/imgui/issues/942
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float radius_outer = 20.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);
    float line_height = ImGui::GetTextLineHeight();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    ImGui::InvisibleButton(label, ImVec2(radius_outer*2, radius_outer*2 + line_height + style.ItemInnerSpacing.y));
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();
    if (is_active && io.MouseDelta.x != 0.0f)   {
        if (v_step<=0) v_step=50.f;
        float step = (v_max - v_min) / v_step;
        *p_value += io.MouseDelta.x * step;

        if (*p_value < v_min) *p_value = v_min;
        if (*p_value > v_max) *p_value = v_max;
        value_changed = true;
    }
    else if (is_hovered && (io.MouseDoubleClicked[0] || io.MouseClicked[2])) {
        *p_value = (v_max + v_min) * 0.5f;  // reset value
        value_changed = true;
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer*0.40f;
    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(ImVec2(center.x + angle_cos*radius_inner, center.y + angle_sin*radius_inner), ImVec2(center.x + angle_cos*(radius_outer-2), center.y + angle_sin*(radius_outer-2)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%.3f", *p_value);
        ImGui::EndTooltip();
    }

    return value_changed;
}

// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor,
                            const ImVec4* pOptionalMainColor, const ImVec4* pOptionalBackdropColor,
                            int circle_count,const float speed) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const ImGuiStyle& style = GetStyle();

    if (circle_count<=0) circle_count = 12;
    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (!pOptionalMainColor)        pOptionalMainColor = &style.Colors[ImGuiCol_Button];
    if (!pOptionalBackdropColor)    pOptionalBackdropColor = &style.Colors[ImGuiCol_ButtonHovered];

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicatorRadiusPixels / 8.f;
    indicatorRadiusPixels-= 2.0f*circle_radius;
    const ImRect bb(pos, ImVec2(pos.x + indicatorRadiusPixels*2.f+4.f*circle_radius,
                                pos.y + indicatorRadiusPixels*2.f+4.f*circle_radius));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id)) {
        return;
    }
    const float base_num_segments = circle_radius*1.f;
    const double t = g.Time;
    const float degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const float sinx = -ImSin(degree_offset * i);
        const float cosx = ImCos(degree_offset * i);
        const float growth = ImMax(0.0f, ImSin((float)(t*(double)(speed*3.0f)-(double)(i*degree_offset))));
        ImVec4 color;
        color.x = pOptionalMainColor->x * growth + pOptionalBackdropColor->x * (1.0f - growth);
        color.y = pOptionalMainColor->y * growth + pOptionalBackdropColor->y * (1.0f - growth);
        color.z = pOptionalMainColor->z * growth + pOptionalBackdropColor->z * (1.0f - growth);
        color.w = 1.0f;
        float grown_circle_radius = circle_radius*(1.0f + growth);
        int num_segments = (int)(base_num_segments*grown_circle_radius);
        if (num_segments<4) num_segments=4;
        window->DrawList->AddCircleFilled(ImVec2(pos.x+2.f*circle_radius + indicatorRadiusPixels*(1.0f+sinx),
                                                 pos.y+2.f*circle_radius + indicatorRadiusPixels*(1.0f+cosx)),
                                          grown_circle_radius,
                                          GetColorU32(color),num_segments);
    }
}

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label,float indicatorRadiusFactor, float indicatorRadiusThicknessFactor, const ImVec4* pOptionalColor) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (indicatorRadiusThicknessFactor<=0.f) indicatorRadiusThicknessFactor = 1.f;
    if (!pOptionalColor)    pOptionalColor = &style.Colors[ImGuiCol_Button];
    const ImU32 color = GetColorU32(*pOptionalColor);

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;
    float indicatorThicknessPixels = indicatorRadiusThicknessFactor*indicatorRadiusPixels*0.6f;
    if (indicatorThicknessPixels>indicatorThicknessPixels*0.4f) indicatorThicknessPixels=indicatorThicknessPixels*0.4f;
    indicatorRadiusPixels-=indicatorThicknessPixels;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size(indicatorRadiusPixels*2.f, (indicatorRadiusPixels + style.FramePadding.y)*2.f);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return;

    // Render
    window->DrawList->PathClear();

    //int num_segments = indicatorRadiusPixels/8.f;
    //if (num_segments<4) num_segments=4;

    int num_segments = 30;

    int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

    const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

    const ImVec2 centre = ImVec2(pos.x+indicatorRadiusPixels, pos.y+indicatorRadiusPixels+style.FramePadding.y);

    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * indicatorRadiusPixels,
                                            centre.y + ImSin(a+g.Time*8) * indicatorRadiusPixels));
    }

    window->DrawList->PathStroke(color, false, indicatorThicknessPixels);
}

SmartButtonState SmartButton(const char* label, ImVec2 size) {
    bool released = ImGui::Button(label, size);

    // Order of these are important
    if (released) return SmartButtonState_Released;
    if (ImGui::IsItemActive()) return SmartButtonState_Pressed;
    if (ImGui::IsItemHovered()) return SmartButtonState_Hovered;
    return SmartButtonState_None;
}

SmartButtonState BangButton(const char* label, ImVec4& color, ImVec2 size) {
    SmartButtonState state;
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    state = ImGui::SmartButton(label, size);
    ImGui::PopStyleColor(3);

    return state;
}

}
