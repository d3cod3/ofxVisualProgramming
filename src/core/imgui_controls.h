#pragma once

#include "imgui.h"

typedef int ImGuiEnvelopeEditorType;
enum ImGuiEnvelopeEditorType_ {
    ImGuiEnvelopeEditorType_AHR           = 0,
    ImGuiEnvelopeEditorType_ADSR          = 1
};

typedef int SmartButtonState;
enum SmartButtonState_ {
    SmartButtonState_None       = 0,
    SmartButtonState_Hovered    = 1,
    SmartButtonState_Pressed    = 2,
    SmartButtonState_Released   = 3
};

//--------------------------------------------------

namespace ImGuiEx {

bool EnvelopeEditor(ImDrawList* drawList, float width, float height, float *_a, float *_h, float *_s, float *_r, int type=ImGuiEnvelopeEditorType_ADSR);

bool Pad2D(ImDrawList* drawList, float width, float height,float *_x, float *_y);

// Minimal implementation modified from: https://github.com/ocornut/imgui/issues/942
bool KnobFloat(ImDrawList* draw_list, float width, ImU32 color, const char* label, float* p_value, float v_min, float v_max, float v_step=50.f);

SmartButtonState SmartButton(const char* label, ImVec2 size = {0, 0});

SmartButtonState BangButton(const char* label, ImVec4& color, ImVec2 size = {0, 0});

}
