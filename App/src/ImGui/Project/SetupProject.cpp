#include "SetupProject.h"

#include <filesystem>
#include <fstream>
#include <thread>

#include <imgui.h>

#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/FileDialogs.h"
#include "Engine/Utils/utf8.h"

#include "ImGui/Overlays/Overlay.h"
#include "ImGui/Overlays/ScriptPopup.h"
#include "Python/PythonCommand.h"

namespace LM
{

    const FileDialogs::Filter kFileDialogsFilter { "InSearch Project (*.pdf)", "*.pdf" };

    void SetupProject::Draw(Ref<Project> _Project)
    {
        if (!m_IsOpen || _Project == Ref<Project>())
        {
            return;
        }

        if (ImGui::Begin(U8("Настройка проекта")))
        {
            if (ImGui::TreeNodeEx(U8("Базовые пути"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                ImGui::Text(U8("Файл проекта: %s"), _Project->GetFileName().c_str());
                ImGui::Text(U8("Папка ассетов: %s"), _Project->GetAssetsPath().c_str());

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx(U8("Настройки каталога"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawCatalog(_Project);
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void SetupProject::DrawCatalog(Ref<Project> _Project)
    {
        ImGui::Text(U8("Оригинал каталога: %s"), _Project->GetCatalogBaseFilename().c_str());
        if (ImGui::Button(U8("Изменить оригинал каталога")))
        {
            if (std::string filename = FileDialogs::OpenFile(kFileDialogsFilter); filename != std::string())
            {
                try
                {
                    std::filesystem::remove(_Project->GetCatalogFilename());
                    if (std::filesystem::copy_file(filename, _Project->GetCatalogFilename()))
                    {
                        _Project->SetCatalogBaseFilename(filename);
                    }
                }
                catch (const std::filesystem::filesystem_error& err)
                {
                    Overlay::Get()->Start(
                        Format(U8("Не удалось изменить файл каталога: \n{} \nПричина: {}"), filename, err.what()));
                    LOGE("File copy error (", filename, "),    ", "filesystem error: ", err.what());
                }
            }
        }

        bool splitPages = _Project->GetCatalogSplitPages();
        if (ImGui::Checkbox(U8("Разделить страницу на две картинки"), &splitPages))
        {
            _Project->SetCatalogSplitPages(splitPages);
        }

        int multi = _Project->GetCatalogImgQuality();
        if (ImGui::SliderInt(U8("Качество картинки"), &multi, 1, 6))
        {
            _Project->SetCatalogImgQuality(multi);
        }

        ImGui::Spacing();

        if (ImGui::Button(U8("Сгенерировать картинки из PDF")))
        {
            for (const auto& entry : std::filesystem::directory_iterator(_Project->GetRawImgPath()))
            {
                std::filesystem::remove_all(entry.path());
            }

            PythonCommand pythonCommand(std::string(RES_FOLDER) + "assets/scripts/prepare_img_raw.py");
            pythonCommand.AddArg(_Project->GetCatalogFilename());
            pythonCommand.AddArg(_Project->GetRawImgPath());
            pythonCommand.AddArg(_Project->GetCatalogImgQuality());
            pythonCommand.AddArg(_Project->GetCatalogSplitPages());

            ScriptPopup::Get()->OpenPopup({ U8("Генерация картинок из PDF"),
                                            []() {
                                                ImGui::Text(U8("Работает скрипт преобразования pdf в картинки"));
                                                ImGui::Text(U8("Это может занять несколько минут"));
                                                ImGui::Text(U8("После его завершения можно закрыть это окно"));
                                            },
                                            pythonCommand });
        }
        if (_Project->GetCatalogNeedRebuild())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), U8("Каталог нужно перегенерировать")); 
        }
    }

    void SetupProject::DrawPythonBuffer()
    {
        std::lock_guard lock(m_PythonBufferMtx);
        ImGui::Text(m_PythonBuffer.c_str());
    }

}    // namespace LM
