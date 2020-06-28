// modified from https://github.com/Raikiri/LegitProfiler

#pragma once

#include <glm/glm.hpp>
#include <array>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include "imgui.h"

#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))

namespace ImGuiEx {

struct ProfilerTask
{
    double startTime;
    double endTime;
    std::string name;
    uint32_t color;
    double GetLength()
    {
        return endTime - startTime;
    }
};

class ProfilerGraph {

public:
    int frameWidth;
    int frameSpacing;

    uint32_t* colors;

    ProfilerGraph(size_t framesCount)
    {
        frames.resize(framesCount);
        for (auto &frame : frames)
            frame.tasks.reserve(100);
        frameWidth = 3;
        frameSpacing = 1;

        colors = new uint32_t[17];

        colors[0] = RGBA_LE(0x1abc9cffu);
        colors[1] = RGBA_LE(0xbdc3c7ffu);
        colors[2] = RGBA_LE(0xd35400ffu);
        colors[3] = RGBA_LE(0xc0392bffu);
        colors[4] = RGBA_LE(0x27ae60ffu);
        colors[5] = RGBA_LE(0x2980b9ffu);
        colors[6] = RGBA_LE(0x8e44adffu);
        colors[7] = RGBA_LE(0xf39c12ffu);
        colors[8] = RGBA_LE(0x2ecc71ffu);
        colors[9] = RGBA_LE(0xe67e22ffu);
        colors[10] = RGBA_LE(0x9b59b6ffu);
        colors[11] = RGBA_LE(0xe74c3cffu);
        colors[12] = RGBA_LE(0xf1c40fffu);
        colors[13] = RGBA_LE(0xecf0f1ffu);
        colors[14] = RGBA_LE(0x3498dbffu);
        colors[15] = RGBA_LE(0x16a085ffu);
        colors[16] = RGBA_LE(0xF2F5FAFFu);
    }

    void LoadFrameData(const ProfilerTask* tasks, size_t count)
    {

        auto &currFrame = frames[currFrameIndex];
        currFrame.tasks.resize(0);
        for (size_t taskIndex = 0; taskIndex < count; taskIndex++)
        {
            if (taskIndex == 0)
                currFrame.tasks.push_back(tasks[taskIndex]);
            else
            {
                if (tasks[taskIndex - 1].color != tasks[taskIndex].color || tasks[taskIndex - 1].name != tasks[taskIndex].name)
                {
                    currFrame.tasks.push_back(tasks[taskIndex]);
                }
                else
                {
                    currFrame.tasks.back().endTime = tasks[taskIndex].endTime;
                }
            }
        }
        currFrame.taskStatsIndex.resize(currFrame.tasks.size());

        for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++)
        {
            auto &task = currFrame.tasks[taskIndex];
            auto it = taskNameToStatsIndex.find(task.name);
            if (it == taskNameToStatsIndex.end())
            {
                taskNameToStatsIndex[task.name] = taskStats.size();
                TaskStats taskStat;
                taskStat.maxTime = -1.0;
                taskStats.push_back(taskStat);
            }
            currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex[task.name];
        }
        currFrameIndex = (currFrameIndex + 1) % frames.size();

        RebuildTaskStats(currFrameIndex, 300/*frames.size()*/);
    }

    void RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const glm::vec2 widgetPos = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
        RenderGraph(drawList, widgetPos, glm::vec2(graphWidth, height), frameIndexOffset);
        RenderLegend(drawList, widgetPos + glm::vec2(graphWidth, 0.0f), glm::vec2(legendWidth, height), frameIndexOffset);
        ImGui::Dummy(ImVec2(float(graphWidth + legendWidth), float(height)));
    }


private:
    void RebuildTaskStats(size_t endFrame, size_t framesCount)
    {
        for (auto &taskStat : taskStats)
        {
            taskStat.maxTime = -1.0f;
            taskStat.priorityOrder = size_t(-1);
            taskStat.onScreenIndex = size_t(-1);
        }

        for (size_t frameNumber = 0; frameNumber < framesCount; frameNumber++)
        {
            size_t frameIndex = (endFrame - 1 - frameNumber + frames.size()) % frames.size();
            auto &frame = frames[frameIndex];
            for (size_t taskIndex = 0; taskIndex < frame.tasks.size(); taskIndex++)
            {
                auto &task = frame.tasks[taskIndex];
                auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
                stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
            }
        }
        std::vector<size_t> statPriorities;
        statPriorities.resize(taskStats.size());
        for(size_t statIndex = 0; statIndex < taskStats.size(); statIndex++)
            statPriorities[statIndex] = statIndex;

        std::sort(statPriorities.begin(), statPriorities.end(), [this](size_t left, size_t right) {return taskStats[left].maxTime > taskStats[right].maxTime; });
        for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++)
        {
            size_t statIndex = statPriorities[statNumber];
            taskStats[statIndex].priorityOrder = statNumber;
        }
    }

    void RenderGraph(ImDrawList *drawList, glm::vec2 graphPos, glm::vec2 graphSize, size_t frameIndexOffset)
    {
        Rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
        float heightThreshold = 1.0f;
        //float maxFrameTime = 1.0f / 60.0f;

        for (size_t frameNumber = 0; frameNumber < frames.size(); frameNumber++)
        {
            size_t frameIndex = (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * frames.size()) % frames.size();

            glm::vec2 framePos = graphPos + glm::vec2(graphSize.x - 1 - frameWidth - (frameWidth + frameSpacing) * frameNumber, graphSize.y - 1);
            if (framePos.x < graphPos.x + 1)
                break;
            glm::vec2 taskPos = framePos + glm::vec2(0.0f, 0.0f);
            auto &frame = frames[frameIndex];

            float sumFrameTime = 0;
            for (const auto& task : frame.tasks)
                sumFrameTime += float(task.endTime - task.startTime);

            float lastEndHeight = 0;

            for (const auto& task : frame.tasks)
            {
                float taskStartHeight = lastEndHeight;
                float taskEndHeight = taskStartHeight + (float(task.endTime - task.startTime) / sumFrameTime) * graphSize.y;
                lastEndHeight = taskEndHeight;
                if (abs(taskEndHeight - taskStartHeight) > heightThreshold)
                    Rect(drawList, taskPos + glm::vec2(0.0f, -taskStartHeight), taskPos + glm::vec2(frameWidth, -taskEndHeight), task.color, true);
            }
        }
    }

    void RenderLegend(ImDrawList *drawList, glm::vec2 legendPos, glm::vec2 legendSize, size_t frameIndexOffset)
    {
        float markerLeftRectMargin = 3.0f;
        float markerLeftRectWidth = 5.0f;
        float markerMidWidth = 30.0f;
        float markerRightRectWidth = 10.0f;
        float markerRigthRectMargin = 3.0f;
        float markerRightRectHeight = 10.0f;
        float markerRightRectSpacing = 4.0f;
        float nameOffset = 30.0f;
        glm::vec2 textMargin = glm::vec2(5.0f, -3.0f);

        auto &currFrame = frames[(currFrameIndex - frameIndexOffset - 1 + 2 * frames.size()) % frames.size()];
        size_t maxTasksCount = size_t(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

        for (auto &taskStat : taskStats)
        {
            taskStat.onScreenIndex = size_t(-1);
        }

        float sumFrameTime = 0;
        for (const auto& task : currFrame.tasks)
            sumFrameTime += task.endTime;

        size_t tasksToShow = std::min<size_t>(taskStats.size(), maxTasksCount);
        size_t tasksShownCount = 0;
        float lastEndHeight = 0;

        for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++)
        {
            auto &task = currFrame.tasks[taskIndex];
            auto &stat = taskStats[currFrame.taskStatsIndex[taskIndex]];

            if (stat.priorityOrder >= tasksToShow)
                continue;

            if (stat.onScreenIndex == size_t(-1))
            {
                stat.onScreenIndex = tasksShownCount++;
            }
            else
                continue;
            float taskStartHeight = lastEndHeight;
            float taskEndHeight = taskStartHeight + (float(task.endTime) / sumFrameTime) * legendSize.y;
            lastEndHeight = taskEndHeight;

            glm::vec2 markerLeftRectMin = legendPos + glm::vec2(markerLeftRectMargin, legendSize.y);
            glm::vec2 markerLeftRectMax = markerLeftRectMin + glm::vec2(markerLeftRectWidth, 0.0f);
            markerLeftRectMin.y -= taskStartHeight;
            markerLeftRectMax.y -= taskEndHeight;

            glm::vec2 markerRightRectMin = legendPos + glm::vec2(markerLeftRectMargin + markerLeftRectWidth + markerMidWidth, legendSize.y - markerRigthRectMargin - (markerRightRectHeight + markerRightRectSpacing) * stat.onScreenIndex);
            glm::vec2 markerRightRectMax = markerRightRectMin + glm::vec2(markerRightRectWidth, -markerRightRectHeight);
            RenderTaskMarker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin, markerRightRectMax, task.color);

            uint32_t textColor = task.color;

            float taskTimeMs = float(task.endTime - task.startTime);
            std::ostringstream timeText;
            timeText.precision(2);
            timeText << std::fixed << std::string("[") << (taskTimeMs * 1000.0f);

            Text(drawList, markerRightRectMax + textMargin, textColor, timeText.str().c_str());
            Text(drawList, markerRightRectMax + textMargin + glm::vec2(nameOffset, 0.0f), textColor, (std::string("ms] ") + task.name).c_str());
        }

    }

    static void Rect(ImDrawList *drawList, glm::vec2 minPoint, glm::vec2 maxPoint, uint32_t col, bool filled = true)
    {
        if(filled)
            drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
        else
            drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
    }
    static void Text(ImDrawList *drawList, glm::vec2 point, uint32_t col, const char *text)
    {
        drawList->AddText(ImVec2(point.x, point.y), col, text);
    }
    static void Triangle(ImDrawList *drawList, std::array<glm::vec2, 3> points, uint32_t col, bool filled = true)
    {
        if (filled)
            drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
        else
            drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
    }
    static void RenderTaskMarker(ImDrawList *drawList, glm::vec2 leftMinPoint, glm::vec2 leftMaxPoint, glm::vec2 rightMinPoint, glm::vec2 rightMaxPoint, uint32_t col)
    {
        Rect(drawList, leftMinPoint, leftMaxPoint, col, true);
        Rect(drawList, rightMinPoint, rightMaxPoint, col, true);
        std::array<ImVec2, 4> points = {
            ImVec2(leftMaxPoint.x, leftMinPoint.y),
            ImVec2(leftMaxPoint.x, leftMaxPoint.y),
            ImVec2(rightMinPoint.x, rightMaxPoint.y),
            ImVec2(rightMinPoint.x, rightMinPoint.y)
        };
        drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
    }
    struct FrameData
    {
        std::vector<ProfilerTask> tasks;
        std::vector<size_t> taskStatsIndex;
    };

    struct TaskStats
    {
        double maxTime;
        size_t priorityOrder;
        size_t onScreenIndex;
    };

    std::vector<TaskStats> taskStats;
    std::map<std::string, size_t> taskNameToStatsIndex;

    std::vector<FrameData> frames;
    size_t currFrameIndex = 0;

}; // End ProfilerGraph

class ProfilersWindow {

public:
    ProfilersWindow():
        cpuGraph(300),
        gpuGraph(300)
    {
        stopProfiling = false;
        frameOffset = 0;
        frameWidth = 3;
        frameSpacing = 1;
        prevFpsFrameTime = std::chrono::system_clock::now();
        fpsFramesCount = 0;
        avgFrameTime = 1.0f;
    }
    void Render()
    {
        fpsFramesCount++;
        auto currFrameTime = std::chrono::system_clock::now();
        {
            float fpsDeltaTime = std::chrono::duration<float>(currFrameTime - prevFpsFrameTime).count();
            if (fpsDeltaTime > 0.5f)
            {
                this->avgFrameTime = fpsDeltaTime / float(fpsFramesCount);
                fpsFramesCount = 0;
                prevFpsFrameTime = currFrameTime;
            }
        }

        ImGui::SetNextWindowBgAlpha(0.3f);

        std::stringstream title;
        title.precision(2);
        title << std::fixed << "Profiler [Rendering at " << 1.0f / avgFrameTime << "fps\t" << avgFrameTime * 1000.0f << "ms]###ProfilerWindow";
        //###AnimatedTitle
        ImGui::Begin(title.str().c_str(), 0, ImGuiWindowFlags_NoScrollbar);
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
        int maxGraphHeight = 300;
        int availableGraphHeight = (int(canvasSize.y) - sizeMargin) / 2;
        int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
        int legendWidth = 300;
        int graphWidth = int(canvasSize.x) - legendWidth;
        gpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
        cpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);

        static bool displaySettings = false;
        if (displaySettings){
            ImGui::Columns(2);
            ImGui::Checkbox("Stop profiling", &stopProfiling);
            ImGui::DragInt("Frame offset", &frameOffset, 1.0f, 0, 400);
            ImGui::NextColumn();

            ImGui::SliderInt("Frame width", &frameWidth, 1, 4);
            ImGui::SliderInt("Frame spacing", &frameSpacing, 0, 2);
            ImGui::Columns(1);
        }
        if (!stopProfiling) frameOffset = 0;

        gpuGraph.frameWidth = frameWidth;
        gpuGraph.frameSpacing = frameSpacing;
        cpuGraph.frameWidth = frameWidth;
        cpuGraph.frameSpacing = frameSpacing;

        ImGui::End();
    }


    bool stopProfiling;
    int frameOffset;
    ProfilerGraph cpuGraph;
    ProfilerGraph gpuGraph;
    int frameWidth;
    int frameSpacing;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    TimePoint prevFpsFrameTime;
    size_t fpsFramesCount;
    float avgFrameTime;
};

}
