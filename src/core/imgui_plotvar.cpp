#include "imgui_plotvar.h"

void ImGui::PlotVar(const char* label, float value, float width, float height, float scale_min, float scale_max, size_t buffer_size)
{
    assert(label);
    if (buffer_size == 0)
        buffer_size = 120;

    ImGui::PushID(label);
    ImGuiID id = ImGui::GetID("");

    // Lookup O(log N)
    PlotVarData& pvd = g_PlotVarsMap[id];

    // Setup
    if (pvd.Data.capacity() != buffer_size)
    {
        pvd.Data.resize(buffer_size);
        memset(&pvd.Data[0], 0, sizeof(float) * buffer_size);
        pvd.DataInsertIdx = 0;
        pvd.LastFrame = -1;
    }

    // Insert (avoid unnecessary modulo operator)
    if (pvd.DataInsertIdx == buffer_size)
        pvd.DataInsertIdx = 0;
    //int display_idx = pvd.DataInsertIdx;
    if (value != FLT_MAX)
        pvd.Data[pvd.DataInsertIdx++] = value;

    // Draw
    int current_frame = ImGui::GetFrameCount();
    if (pvd.LastFrame != current_frame)
    {
        //char overlay[32];
        //sprintf(overlay, "%-3.4f", pvd.Data[display_idx]);
        ImGui::PlotLines("##plot", &pvd.Data[0], buffer_size, pvd.DataInsertIdx, NULL, scale_min, scale_max, ImVec2(width, height));
        //ImGui::SameLine();
        //ImGui::Text("%s\n%-3.4f", label, pvd.Data[display_idx]);	// Display last value in buffer
        pvd.LastFrame = current_frame;
    }

    ImGui::PopID();
}

void ImGui::PlotVarFlushOldEntries()
{
    int current_frame = ImGui::GetFrameCount();
    for (PlotVarsMap::iterator it = g_PlotVarsMap.begin(); it != g_PlotVarsMap.end(); )
    {
        PlotVarData& pvd = it->second;
        if (pvd.LastFrame < current_frame - MAX(400,(int)pvd.Data.size()))
            it = g_PlotVarsMap.erase(it);
        else
            ++it;
    }
}
