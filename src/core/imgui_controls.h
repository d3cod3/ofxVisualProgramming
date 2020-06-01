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

namespace ImGui {

IMGUI_API int EnvelopeEditor(float width, float height, float points[6], int type=ImGuiEnvelopeEditorType_ADSR);

// Minimal implementation from: https://github.com/ocornut/imgui/issues/942
IMGUI_API bool KnobFloat(const char* label, float* p_value, float v_min, float v_max, float v_step=50.f);

// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
IMGUI_API void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor=1.f,const ImVec4* pOptionalMainColor=NULL, const ImVec4* pOptionalBackdropColor=NULL,int circle_count=8, const float speed=1.f);

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label, float indicatorRadiusFactor=1.f, float indicatorRadiusThicknessFactor=1.f, const ImVec4* pOptionalColor=NULL);

SmartButtonState SmartButton(const char* label, ImVec2 size = {0, 0});

SmartButtonState BangButton(const char* label, ImVec4& color, ImVec2 size = {0, 0});

}
