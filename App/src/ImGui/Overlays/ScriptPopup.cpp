#include "ScriptPopup.h"

#include "imgui.h"

#include "Engine/Utils/utf8.h"
#include "Engine/Utils/ConsoleLog.h"

namespace LM
{
    void ScriptPopup::OpenPopup(const ScriptOpenPopupProps& _Props)
    {
        m_IsScriptRuning = true;
        m_ScriptBuffer = "";
        m_Props = _Props;
        m_NeedOpenPopup = true;

        std::thread thread(
            [&](PythonCommand command) {
                command.Execute([&](const char* buffer) {
                    std::lock_guard lock(m_ScriptBufferMtx);
                    m_ScriptBuffer = buffer;
                });

                m_IsScriptRuning = false;
            },
            _Props.Command);
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
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGuiWindowFlags popupFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

        if (ImGui::BeginPopupModal(m_Props.WindowName.c_str(), NULL, popupFlags))
        {
            if (m_Props.PopupDesc)
            {
                m_Props.PopupDesc();
            }

            ImGui::Separator();
            ImGui::Text("\n");
            DrawScriptBuffer();
            ImGui::Text("\n");
            ImGui::Separator();

            ImGui::BeginDisabled(m_IsScriptRuning);
            if (ImGui::Button(U8("Закрыть"), ImVec2(120, 0)))
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
        ImGui::Text(m_ScriptBuffer.c_str());
    }
}    // namespace LM
