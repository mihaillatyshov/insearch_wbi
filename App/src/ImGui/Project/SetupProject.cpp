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
#include "Managers/TextureManager.h"
#include "Python/PythonCommand.h"
#include "Utils/FileSystemUtils.h"

namespace LM
{

    const FileDialogs::Filter kFileDialogsFilter { "InSearch Project (*.pdf)", "*.pdf" };

    SetupProject::SetupProject() { }

    void SetupProject::Draw(Ref<Project> _Project)
    {
        if (!m_IsOpen || _Project == Project::s_ProjectNotOpen)
        {
            return;
        }

        if (ImGui::Begin("Настройка проекта"))
        {
            if (ImGui::TreeNodeEx("Базовые пути", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                ImGui::Text("Папка проекта: %s", _Project->GetFolder().c_str());

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Настройки каталога", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawCatalog(_Project);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Обрезание по паттерну", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawImgsByCutPattern(_Project);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Генерация первого Excel",
                                  ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawGenRawExcel(_Project);

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void SetupProject::DrawCatalog(Ref<Project> _Project)
    {
        ImGui::Text("Оригинал каталога: %s", _Project->GetCatalogBaseFilename().c_str());
        if (ImGui::Button("Изменить оригинал каталога"))
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
                        Format("Не удалось изменить файл каталога: \n{} \nПричина: {}", filename, err.what()));
                    LOGE("File copy error (", filename, "),    ", "filesystem error: ", err.what());
                }
            }
        }

        bool splitPages = _Project->GetCatalogSplitPages();
        if (ImGui::Checkbox("Разделить страницу на две картинки", &splitPages))
        {
            _Project->SetCatalogSplitPages(splitPages);
        }

        int multi = _Project->GetCatalogImgQuality();
        if (ImGui::SliderInt("Качество картинки", &multi, 1, 6))
        {
            _Project->SetCatalogImgQuality(multi);
        }

        ImGui::Spacing();

        if (ImGui::Button("Сгенерировать картинки из PDF"))
        {
            GenCatalogRawImages(_Project);
        }

        if (_Project->GetCatalogNeedRebuild() && _Project->GetCatalogBaseFilename() != std::string())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Каталог нужно перегенерировать");
        }
    }

    void SetupProject::DrawImgsByCutPattern(Ref<Project> _Project)
    {
        if (_Project->GetCatalogTopLeftPattern().IsExists())
        {
            ImGui::Text("Верхний левый паттерн");
            ImGui::Text("На странице: %d", _Project->GetCatalogTopLeftPattern().PageId);
        }

        if (_Project->GetCatalogBotRightPattern().IsExists())
        {
            ImGui::Text("Нижний правый паттерн");
            ImGui::Text("На странице: %d", _Project->GetCatalogBotRightPattern().PageId);
        }

        if (ImGui::Button("Обрезать каталог по паттернам"))
        {
            GenImgsByCutPattern(_Project);
        }

        if (_Project->GetIsCatalogGenerated() && _Project->GetImgsByCutPatternNeedRebuild())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Картинки нужно обрезать");
        }
    }

    void SetupProject::DrawGenRawExcel(Ref<Project> _Project)
    {
        bool useCutPatterntImgs = _Project->GetRawExcelUseCutPatternImgs();
        if (ImGui::Checkbox("Использовать обрезанные картинки", &useCutPatterntImgs))
        {
            _Project->SetRawExcelUseCutPatternImgs(useCutPatterntImgs);
        }

        if (ImGui::Button("Сгенерировать Excel"))
        {
            GenRawExcel(_Project);
        }

        if (_Project->GetIsImgsByCutPatternGenerated() && _Project->GetRawExcelNeedRebuild())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Файлы Excel нужно перегенерировать");
        }
    }

    void SetupProject::GenCatalogRawImages(Ref<Project> _Project)
    {
        if (_Project->GetCatalogBaseFilename() == std::string())
        {
            Overlay::Get()->Start("Каталог не выбран!");
            return;
        }

        FileSystemUtils::RemoveAllInFolder(_Project->GetRawImgPath());
        FileSystemUtils::RemoveAllInFolder(_Project->GetRawImgPrevPath());

        PythonCommand pythonCommand("./assets/scripts/prepare_img_raw.py");
        pythonCommand.AddArg(_Project->GetCatalogFilename());
        pythonCommand.AddArg(_Project->GetRawImgPath());
        pythonCommand.AddArg(_Project->GetRawImgPrevPath());
        pythonCommand.AddArg(_Project->GetCatalogImgQuality());
        pythonCommand.AddArg(_Project->GetCatalogSplitPages());

        ScriptPopup::Get()->OpenPopup(pythonCommand, { "Генерация картинок из PDF",
                                                       []() {
                                                           ImGui::Text("Работает скрипт преобразования pdf в картинки");
                                                           ImGui::Text("Это может занять несколько минут");
                                                           ImGui::Text("После его завершения можно закрыть это окно");
                                                       },
                                                       [=]() {
                                                           _Project->OnGenCatalogRawImgs();
                                                           Project::Save(_Project);
                                                           TextureManager::RemoveAll();
                                                       } });
    }

    void SetupProject::GenImgsByCutPattern(Ref<Project> _Project)
    {
        if (!_Project->GetCatalogTopLeftPattern().IsExists())
        {
            Overlay::Get()->Start("Левый верхний паттерн отсутствует!");
            return;
        }

        if (!_Project->GetCatalogBotRightPattern().IsExists())
        {
            Overlay::Get()->Start("Правый нижний паттерн отсутствует!");
            return;
        }

        if (!_Project->GetIsCatalogGenerated())
        {
            Overlay::Get()->Start("Каталог не сгенерирован!");
            return;
        }

        FileSystemUtils::RemoveAllInFolder(_Project->GetCutByPatternImgsPath());
        FileSystemUtils::RemoveAllInFolder(_Project->GetCutByPatternImgsPrevPath());
        // TODO: Delete ImgsByCutPattern Versions ???

        PythonCommand pythonCommand("./assets/scripts/cut_by_pattern.py");
        pythonCommand.AddArg(_Project->GetRawImgPath());
        pythonCommand.AddArg(_Project->GetProjectFilename());
        pythonCommand.AddArg(_Project->GetTmpPath());
        pythonCommand.AddArg(_Project->GetCutByPatternImgsPath());
        pythonCommand.AddArg(_Project->GetCutByPatternImgsPrevPath());
        pythonCommand.AddArg(_Project->GetCatalogSplitPages());
        pythonCommand.AddArg("0.99");

        ScriptPopup::Get()->OpenPopup(pythonCommand, { "Обрезание картинок каталога",
                                                       []() {
                                                           ImGui::Text("Работает скрипт обрезания картинок");
                                                           ImGui::Text("Это может занять несколько минут");
                                                           ImGui::Text("После его завершения можно закрыть это окно");
                                                       },
                                                       [=]() {
                                                           _Project->OnGenImgsByCutPattern();
                                                           Project::Save(_Project);
                                                           TextureManager::RemoveAll();
                                                       } });
    }

    void SetupProject::GenRawExcel(Ref<Project> _Project)
    {
        bool useCutPatterntImgs = _Project->GetRawExcelUseCutPatternImgs();
        if (!_Project->GetIsImgsByCutPatternGenerated() && useCutPatterntImgs)
        {
            Overlay::Get()->Start("Обрезаные картинки по паттерну не сгенерированы!");
            return;
        }

        if (!_Project->GetIsCatalogGenerated() && !useCutPatterntImgs)
        {
            Overlay::Get()->Start("Каталог не сгенерирован!");
            return;
        }

        FileSystemUtils::RemoveAllInFolder(_Project->GetRawExcelPath());
        // TODO: Delete RawExcel Versions ???

        PythonCommand pythonCommand("./assets/scripts/extract_tables_to_xlsx.py");
        pythonCommand.AddArg(useCutPatterntImgs ? _Project->GetCutByPatternImgsPath() : _Project->GetRawImgPath());
        pythonCommand.AddArg(_Project->GetProjectFilename());
        pythonCommand.AddArg(_Project->GetRawExcelPath());
        pythonCommand.AddArg(6);

        ScriptPopup::Get()->OpenPopup(pythonCommand, { "Генерация Excel из картинок",
                                                       []() {
                                                           ImGui::Text("Работает скрипт генерации Excel");
                                                           ImGui::Text("Это может занять несколько минут");
                                                           ImGui::Text("После его завершения можно закрыть это окно");
                                                       },
                                                       [=]() {
                                                           _Project->OnGenRawExcel();
                                                           Project::Save(_Project);
                                                       } });
    }

}    // namespace LM
