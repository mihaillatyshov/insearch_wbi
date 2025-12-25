#include "ScriptPopup.h"

#include "imgui.h"

#include "Engine/Utils/Log.hpp"
#include "Engine/Utils/utf8.h"

namespace LM
{
    void ScriptPopup::OpenPopup(const PythonCommand& _Command, const ScriptPopupProps& _Props)
    {
        m_IsScriptRuning = true;
        m_ScriptBuffer = "";
        m_Props = _Props;
        m_NeedOpenPopup = true;

        std::thread thread(
            [&](PythonCommand command) {
                command.Execute([&](const char* buffer) {
                    std::lock_guard lock(m_ScriptBufferMtx);
                    m_ScriptBuffer += buffer;
                });

                m_IsScriptRuning = false;
            },
            _Command);
        thread.detach();
    }

    void ScriptPopup::Draw()
    {
        if (m_NeedOpenPopup)
        {
            ImGui::OpenPopup(m_Props.WindowName.c_str());
            m_NeedOpenPopup = false;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGuiWindowFlags popupFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowSize(ImVec2(viewportSize.x * 0.8f, 0), ImGuiCond_Always);

        if (ImGui::BeginPopupModal(m_Props.WindowName.c_str(), NULL, popupFlags))
        {
            if (m_Props.PopupDesc)
            {
                m_Props.PopupDesc();
            }

            ImGui::Separator();

            if (ImGui::BeginChild("ChildL", ImVec2(0, viewportSize.y * 0.6f),
                                  ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY,
                                  ImGuiWindowFlags_HorizontalScrollbar))
            {
                ImGui::Text("\n");
                DrawScriptBuffer();
                ImGui::Text("\n");
            }
            ImGui::EndChild();

            ImGui::Separator();

            bool isScriptRunning = m_IsScriptRuning.load();

            if (m_Props.EndCallback && !isScriptRunning)
            {
                m_Props.EndCallback();
                m_Props.EndCallback = nullptr;
            }

            ImGui::BeginDisabled(isScriptRunning);
            if (ImGui::Button("Закрыть", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }
    }

    void ScriptPopup::DrawScriptBuffer()
    {
        std::lock_guard lock(m_ScriptBufferMtx);
        ImGui::TextUnformatted(m_ScriptBuffer.c_str());
    }
}    // namespace LM
