#include "ScriptPopup.h"

#include "imgui.h"

namespace LM
{
    void ScriptPopup::OpenPopup(const PythonCommand& _Command, const ScriptPopupProps& _Props)
    {
        m_IsScriptRuning = true;
        m_ScriptBuffer = "";
        m_Props = _Props;
        m_NeedOpenPopup = true;
        m_ScritpReturnCode = 0;

        std::thread thread(
            [&](PythonCommand command) {
                m_ScritpReturnCode = command.Execute([&](const char* buffer) {
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
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::EndChild();

            ImGui::Separator();

            bool isScriptRunning = m_IsScriptRuning.load();

            if (m_Props.EndCallback && !isScriptRunning)
            {
                m_Props.EndCallback(m_ScritpReturnCode);
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

    enum class ErrorTracebackStatus
    {
        kStarted,
        kInProgress,
        kEnded,
        kNone,
    };

    void GetColorSimple(std::string_view _Line, std::string_view _Prefix, const ImVec4& _PrefixColor, ImVec4& _Color,
                        ErrorTracebackStatus& _ErrorTracebackStatus)
    {
        if (_Line.starts_with(_Prefix))
        {
            _Color = _PrefixColor;
            if (_ErrorTracebackStatus == ErrorTracebackStatus::kInProgress)
            {
                _ErrorTracebackStatus = ErrorTracebackStatus::kEnded;
            }
        }
    }

    void ScriptPopup::DrawScriptBuffer()
    {
        std::lock_guard lock(m_ScriptBufferMtx);

        ImVec4 debugColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 infoColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 warnColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 errorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 errorTracebackColor = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);

        ErrorTracebackStatus errorTracebackStatus = ErrorTracebackStatus::kNone;
        bool isErrorTracebackOpened = false;

        ImVec4 currentColor = debugColor;

        std::istringstream stream(m_ScriptBuffer);
        std::string line;
        while (std::getline(stream, line))
        {
            GetColorSimple(line, "[DEBUG]", debugColor, currentColor, errorTracebackStatus);
            GetColorSimple(line, "[INFO]", infoColor, currentColor, errorTracebackStatus);
            GetColorSimple(line, "[WARN]", warnColor, currentColor, errorTracebackStatus);
            GetColorSimple(line, "[ERROR]", errorColor, currentColor, errorTracebackStatus);
            if (line.starts_with("[ERROR TRACE]"))
            {
                currentColor = errorTracebackColor;
                errorTracebackStatus = ErrorTracebackStatus::kStarted;
            }

            if (errorTracebackStatus == ErrorTracebackStatus::kStarted)
            {
                isErrorTracebackOpened = ImGui::TreeNodeEx("ERROR TRACEBACK", ImGuiTreeNodeFlags_Framed);
                errorTracebackStatus = ErrorTracebackStatus::kInProgress;
            }
            if (errorTracebackStatus == ErrorTracebackStatus::kEnded)
            {
                if (isErrorTracebackOpened)
                {
                    ImGui::TreePop();
                    isErrorTracebackOpened = false;
                }
                errorTracebackStatus = ErrorTracebackStatus::kNone;
            }

            if ((errorTracebackStatus == ErrorTracebackStatus::kInProgress && isErrorTracebackOpened) ||
                errorTracebackStatus == ErrorTracebackStatus::kNone)
            {
                // inside traceback or normal line
            }
            else
            {
                // outside traceback, skip line
                continue;
            }
            ImGui::TextColored(currentColor, "%s", line.c_str());
        }

        if (errorTracebackStatus == ErrorTracebackStatus::kEnded ||
            errorTracebackStatus == ErrorTracebackStatus::kInProgress)
        {
            if (isErrorTracebackOpened)
            {
                ImGui::TreePop();
                isErrorTracebackOpened = false;
            }
            errorTracebackStatus = ErrorTracebackStatus::kNone;
        }

        if (!m_IsScriptRuning)
        {
            if (m_ScritpReturnCode != 0)
            {
                ImGui::TextColored(errorColor, "\nСкрипт завершился с ошибками. Код возврата: %d",
                                   m_ScritpReturnCode.load());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.2f, 0.3f, 0.9f, 1.0f), "\nСкрипт успешно завершился. Код возврата: %d",
                                   m_ScritpReturnCode.load());
            }
        }
    }

}    // namespace LM
