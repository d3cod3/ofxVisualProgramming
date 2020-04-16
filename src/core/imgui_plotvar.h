// imgui_plotvar.h
// https://github.com/ocornut/imgui/wiki/plot_var_example

#include "imgui.h"

#include <map>

struct PlotVarData
{
    ImGuiID             ID;
    ImVector<float>     Data;
    int                 DataInsertIdx;
    int                 LastFrame;

    PlotVarData() : ID(0), DataInsertIdx(0), LastFrame(-1) {}
};

typedef std::map<ImGuiID, PlotVarData> PlotVarsMap;
static PlotVarsMap	g_PlotVarsMap;

namespace ImGui{
    // Plot value over time
    // Call with 'value == FLT_MAX' to draw without adding new value to the buffer
    void PlotVar(const char* label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120, float height=40);

    void PlotVarFlushOldEntries();
}


