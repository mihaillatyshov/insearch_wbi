#include "Overlay.h"

#include <imgui.h>

namespace LM
{

    Overlay::Overlay() : m_ImGuiFunction([]() {}), m_EndTimePoint(std::chrono::system_clock::now() - 1s) { }

    void Overlay::Start(const std::string& _Text, std::chrono::seconds _Duration)
    {
        Start([_Text]() { ImGui::Text(_Text.c_str()); }, _Duration);
    }

    void Overlay::Start(std::function<void(void)> _ImGuiFunction, std::chrono::seconds _Duration)
    {
        m_ImGuiFunction = _ImGuiFunction;
        m_EndTimePoint = std::chrono::system_clock::now() + _Duration;
    }

    void Overlay::Draw()
    {
        if (std::chrono::system_clock::now() >= m_EndTimePoint)
        {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                        ImGuiWindowFlags_NoMove;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos { work_pos.x + work_size.x / 2.0f, work_pos.y + work_size.y / 2.0f };
        ImVec2 window_pos_pivot { 0.5f, 0.5f };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.80f);

        if (ImGui::Begin("##Overlay", nullptr, window_flags))
        {
            m_ImGuiFunction();
        }
        ImGui::End();
    }

}    // namespace LM
