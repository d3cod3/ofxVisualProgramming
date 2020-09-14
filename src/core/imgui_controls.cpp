#include "imgui_controls.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

namespace ImGuiEx {

bool EnvelopeEditor(ImDrawList* drawList, float width, float height, float *_a, float *_h, float *_s, float *_r, int type){
    // visuals
    enum { LINE_WIDTH = 3 }; // handlers: small lines width
    enum { GRAB_RADIUS = 6 }; // handlers: circle radius

    float _d = *_a + *_h + *_r;

    ImVec2 Q_AHR[4] = { { 0, 1 }, { *_a / _d, 0.0f }, { (*_a + *_h) / _d, 0.5f }, { 1, 1 }};
    ImVec2 Q_ADSR[5] = { { 0, 1 }, { *_a / _d, 0.0f }, { (*_a + *_h) / _d, 1.0f-*_s }, { (*_a + *_h + (*_r/2)) / _d, 1.0f-*_s }, { 1, 1 } };

    ImGuiWindow* Window = ImGui::GetCurrentWindow();

    // prepare canvas
    const float dim = width > 0 ? width : ImGui::GetContentRegionAvailWidth();
    ImVec2 Canvas(dim, height);

    ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
    ImGui::ItemSize(bb);

    // background grid
    if(Canvas.x >= 4){
        for (int i = 0; i <= Canvas.x; i += static_cast<int>((Canvas.x / 4))) {
            drawList->AddLine(
                        ImVec2(bb.Min.x + i, bb.Min.y),
                        ImVec2(bb.Min.x + i, bb.Max.y - 1),
                        IM_COL32(255,255,255,20));
        }
    }

    if(Canvas.y >= 4){
        for (int i = 0; i <= Canvas.y; i += static_cast<int>((Canvas.y / 4))) {
            drawList->AddLine(
                        ImVec2(bb.Min.x, bb.Min.y + i),
                        ImVec2(bb.Max.x - 1, bb.Min.y + i),
                        IM_COL32(255,255,255,20));
        }
    }

    // draw lines and grabbers
    ImVec4 env_color(1.0f, 1.0f, 120.0f/255.0f, 1.0f);

    bool changed = false;

    //ImVec2 prevCursorPos = ImGui::GetCursorScreenPos();

    if(type == ImGuiEnvelopeEditorType_AHR){
        ImVec2 p1 = Q_AHR[0] * ((bb.Max-ImVec2(1,1)) - bb.Min) + bb.Min;
        ImVec2 p2 = Q_AHR[1] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p3 = Q_AHR[2] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p4 = Q_AHR[3] * ((bb.Max-ImVec2(1,1)) - bb.Min) + bb.Min;

        drawList->AddLine(p1, p2, ImColor(env_color), LINE_WIDTH);
        drawList->AddLine(p2, p3, ImColor(env_color), LINE_WIDTH);
        drawList->AddLine(p3, p4, ImColor(env_color), LINE_WIDTH);

        /*ImGui::SetCursorScreenPos( ImVec2(p2.x - 4, p2.y - 8) );
        ImGui::InvisibleButton( "circleGripP2", ImVec2( 8, 16 )  );

        if(ImGui::IsItemHovered()){
            drawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(env_color), 6);
        }

        static bool isDraggingCircleP2 = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingCircleP2){
                isDraggingCircleP2=true;
            }

            ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,0.0f),ImVec2(0.0f,0.0f),ImVec2(*_h,1.0f));
            p2.x = _pos.x;
            *_a = _pos.x;

            changed = true;
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingCircleP2) isDraggingCircleP2 = false;
        }*/

        drawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(env_color), 6);

        /*ImGui::SetCursorScreenPos( ImVec2(p3.x - 4, p3.y - 8) );
        ImGui::InvisibleButton( "circleGripP3", ImVec2( 8, 16 )  );

        if(ImGui::IsItemHovered()){
            drawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(env_color), 6);
        }

        static bool isDraggingCircleP3 = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingCircleP3){
                isDraggingCircleP3=true;
            }

            ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,0.0f),ImVec2(*_a,0.0f),ImVec2(1.0f,1.0f));
            p3.x = _pos.x;
            *_h = _pos.x;

            changed = true;
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingCircleP3) isDraggingCircleP3 = false;
        }*/

        drawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(env_color), 6);

        //ImGui::SetCursorScreenPos(prevCursorPos);

        //ImGui::Text("A %.2f%s\tH %.2f%s\tR %.2f%s", *_a, "%", *_h-*_a, "%", 1.0f-*_h, "%");

    }else if(type == ImGuiEnvelopeEditorType_ADSR){
        ImVec2 p1 = Q_ADSR[0] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p2 = Q_ADSR[1] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p3 = Q_ADSR[2] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p4 = Q_ADSR[3] * (bb.Max - bb.Min) + bb.Min;
        ImVec2 p5 = Q_ADSR[4] * (bb.Max - bb.Min) + bb.Min;

        drawList->AddLine(p1, p2, ImColor(env_color), LINE_WIDTH);
        drawList->AddLine(p2, p3, ImColor(env_color), LINE_WIDTH);
        drawList->AddLine(p3, p4, ImColor(env_color), LINE_WIDTH);
        drawList->AddLine(p4, p5, ImColor(env_color), LINE_WIDTH);

        /*ImGui::SetCursorScreenPos( ImVec2(p2.x - 4, p2.y - 8) );
        ImGui::InvisibleButton( "circleGripP2", ImVec2( 8, 16 )  );

        if(ImGui::IsItemHovered()){
            drawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(env_color), 6);
        }

        static bool isDraggingCircleP2 = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingCircleP2){
                isDraggingCircleP2=true;
            }

            ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,0.0f),ImVec2(0.0f,0.0f),ImVec2(*_h,1.0f));
            p2.x = _pos.x;
            *_a = _pos.x;

            changed = true;
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingCircleP2) isDraggingCircleP2 = false;
        }*/

        drawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(env_color), 6);

        /*ImGui::SetCursorScreenPos( ImVec2(p3.x - 4, p3.y - 8) );
        ImGui::InvisibleButton( "circleGripP3", ImVec2( 8, 16 )  );

        if(ImGui::IsItemHovered()){
            drawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(env_color), 6);
        }

        static bool isDraggingCircleP3 = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingCircleP3){
                isDraggingCircleP3=true;
            }

            ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,0.0f),ImVec2(*_a,0.0f),ImVec2(*_r,1.0f));
            p3.x = _pos.x;
            *_h = _pos.x;

            changed = true;
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingCircleP3) isDraggingCircleP3 = false;
        }*/

        drawList->AddCircleFilled(p3, GRAB_RADIUS, ImColor(env_color), 6);

        /*ImGui::SetCursorScreenPos( ImVec2(p4.x - 4, p4.y - 8) );
        ImGui::InvisibleButton( "circleGripP4", ImVec2( 8, 16 )  );

        if(ImGui::IsItemHovered()){
            drawList->AddCircleFilled(p4, GRAB_RADIUS, ImColor(env_color), 6);
        }

        static bool isDraggingCircleP4 = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingCircleP4){
                isDraggingCircleP4=true;
            }

            ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,0.0f),ImVec2(*_h,0.0f),ImVec2(1.0f,1.0f));
            p4.x = _pos.x;
            *_r = _pos.x;

            changed = true;
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingCircleP4) isDraggingCircleP4 = false;
        }*/

        drawList->AddCircleFilled(p4, GRAB_RADIUS, ImColor(env_color), 6);


        //ImGui::SetCursorScreenPos(prevCursorPos);

        //ImGui::Text("A %.2f%s\tD %.2f%s\tS %.2f%s\tR %.2f%s", *_a, "%", *_h-*_a, "%", *_r-*_h, "%", 1.0f-*_r, "%");

    }

    return changed;

}

bool Pad2D(ImDrawList* drawList, float width, float height,float *_x, float *_y){

    // visuals
    enum { LINE_WIDTH = 2 }; // handlers: small lines width
    enum { GRAB_RADIUS = 6 }; // handlers: circle radius

    ImGuiWindow* Window = ImGui::GetCurrentWindow();

    // prepare canvas
    const float dim = width > 0 ? width : ImGui::GetContentRegionAvailWidth();
    ImVec2 Canvas(dim, height);

    ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
    ImGui::ItemSize(bb);

    // background grid
    if(Canvas.x >= 4){
        for (int i = 0; i <= Canvas.x; i += static_cast<int>((Canvas.x / 4))) {
            drawList->AddLine(
                        ImVec2(bb.Min.x + i, bb.Min.y),
                        ImVec2(bb.Min.x + i, bb.Max.y - 1),
                        IM_COL32(255,255,255,20));
        }
    }

    if(Canvas.y >= 4){
        for (int i = 0; i <= Canvas.y; i += static_cast<int>((Canvas.y / 4))) {
            drawList->AddLine(
                        ImVec2(bb.Min.x, bb.Min.y + i),
                        ImVec2(bb.Max.x - 1, bb.Min.y + i),
                        IM_COL32(255,255,255,20));
        }
    }

    drawList->AddLine(ImVec2(bb.Min.x + (Canvas.x* *_x), bb.Min.y),ImVec2(bb.Min.x + (Canvas.x* *_x), bb.Max.y - 1),ImGui::GetColorU32(ImGuiCol_TextDisabled),LINE_WIDTH);
    drawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + (Canvas.y* *_y)),ImVec2(bb.Max.x - 1, bb.Min.y + (Canvas.y* *_y)),ImGui::GetColorU32(ImGuiCol_TextDisabled),LINE_WIDTH);


    // DRAG from circle
    bool changed = false;

    ImVec2 prevCursorPos = ImGui::GetCursorScreenPos();

    ImGui::SetCursorScreenPos( ImVec2(bb.Min.x + (Canvas.x* *_x) - 4, bb.Min.y + (Canvas.y* *_y) - 4) );
    ImGui::InvisibleButton( "circleGripBtn", ImVec2( 8, 8 )  );

    static bool isDraggingCircle = false;
    if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
        if(!isDraggingCircle){
            isDraggingCircle=true;
        }

        ImVec2 _pos = ImClamp(ImVec2((ImGui::GetMousePos().x - bb.Min.x) / Canvas.x,(ImGui::GetMousePos().y - bb.Min.y) / Canvas.y),ImVec2(0.0f,0.0f),ImVec2(1.0f,1.0f));
        *_x = _pos.x;
        *_y = _pos.y;

        changed = true;
    }
    else if(ImGui::IsItemDeactivated()){
        if(isDraggingCircle) isDraggingCircle = false;
    }

    drawList->AddCircleFilled(ImVec2(bb.Min.x + (Canvas.x* *_x),bb.Min.y + (Canvas.y* *_y)), GRAB_RADIUS, IM_COL32(255,255,255,245), 6);

    ImGui::SetCursorScreenPos(prevCursorPos);

    return changed;

}

bool KnobFloat(ImDrawList* draw_list, float width, ImU32 color, const char* label, float* p_value, float v_min, float v_max,float v_step) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float gap = width/6.0f;
    float radius_outer = width;
    float labelGap = width/1.15f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer + gap, pos.y + labelGap + radius_outer);
    float line_height = ImGui::GetTextLineHeight();

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
    if(*p_value >= v_max){
        t = 1.0f;
    }
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer*0.40f;
    float radius_inner_medium = radius_outer*0.82f;
    float line_width = width/5.0f;

    draw_list->AddText(ImVec2(pos.x + gap + (((radius_outer*2)-ImGui::CalcTextSize(label).x)/2.0f), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    draw_list->AddCircleFilled(center, radius_outer, IM_COL32(28,28,28,255), 32);

    draw_list->PathArcTo(center, radius_outer-4, ANGLE_MIN, angle,32);
    draw_list->PathArcTo(center, radius_inner_medium, angle, ANGLE_MIN,32);
    draw_list->PathStroke(color,true,line_width);

    draw_list->AddCircleFilled(center, radius_inner, is_active ? ImGui::GetColorU32(ImGuiCol_FrameBgActive) : is_hovered ? ImGui::GetColorU32(ImGuiCol_FrameBgHovered) : IM_COL32(28,28,28,255), 32);
    draw_list->AddLine(ImVec2(center.x + angle_cos*radius_inner, center.y + angle_sin*radius_inner), ImVec2(center.x + angle_cos*(radius_outer-line_width+1), center.y + angle_sin*(radius_outer-line_width+1)), color, line_width);

    char buffer[64];
    snprintf(buffer, sizeof buffer, "%.2f", *p_value);
    draw_list->AddText(ImVec2(pos.x + gap + (((radius_outer*2)-ImGui::CalcTextSize(buffer).x)/2.0f), pos.y + labelGap + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), buffer);

    /*if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x + gap - style.WindowPadding.x, pos.y + labelGap + radius_outer));
        ImGui::BeginTooltip();
        ImGui::PushItemWidth(radius_outer*2);
        ImGui::DragFloat("",p_value);
        ImGui::PopItemWidth();
        ImGui::EndTooltip();
    }*/


    return value_changed;
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
    state = ImGuiEx::SmartButton(label, size);
    ImGui::PopStyleColor(3);

    return state;
}

}
