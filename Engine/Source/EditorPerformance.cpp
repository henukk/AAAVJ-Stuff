#include "Globals.h"
#include "EditorPerformance.h"
#include "Application.h"

EditorPerformance::EditorPerformance()
{
    fpsHistory.assign(maxSamples, 0.0f);
    msHistory.assign(maxSamples, 0.0f);
    current = 0;
}

EditorPerformance::~EditorPerformance() = default;

void EditorPerformance::draw(const char* title, bool* p_open)
{
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    float width = ImGui::GetContentRegionAvail().x;

    float fps = app->getFPS();
    float avgMs = app->getAvgElapsedMs();

    fpsHistory[current] = fps;
    msHistory[current] = avgMs;

    current = (current + 1) % maxSamples;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.95f, 0.75f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));

    ImVec2 fpsGraphSize(width, 100);

    ImGui::PlotHistogram("##FPS", fpsHistory.data(), maxSamples, 0, nullptr, 0.0f, 120.0f, fpsGraphSize);

    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();

        char text[64];
        snprintf(text, sizeof(text), "Framerate %.1f", fps);

        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(
            p0.x + (p1.x - p0.x - textSize.x) * 0.5f,
            p0.y + 4.0f
        );

        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), text);
    }

    ImGui::Separator();
    ImVec2 msGraphSize(width, 80);

    ImGui::PlotHistogram("##MS", msHistory.data(), maxSamples, 0, nullptr, 0.0f, 40.0f, msGraphSize);

    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();

        char text[64];
        snprintf(text, sizeof(text), "Milliseconds %.2f", avgMs);

        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(
            p0.x + (p1.x - p0.x - textSize.x) * 0.5f,
            p0.y + 4.0f
        );

        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), text);
    }

    ImGui::PopStyleColor(2);

    ImGui::End();
}
