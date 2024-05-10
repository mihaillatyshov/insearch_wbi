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

        if (ImGui::Begin(U8("Настройка проекта")))
        {
            if (ImGui::TreeNodeEx(U8("Базовые пути"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                ImGui::Text(U8("Папка проекта: %s"), _Project->GetFolder().c_str());

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx(U8("Настройки каталога"), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawCatalog(_Project);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx(U8("Обрезание по паттерну"),
                                  ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
            {
                DrawImgsByCutPattern(_Project);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx(U8("Генерация первого Excel"),
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
            GenCatalogRawImages(_Project);
        }

        if (_Project->GetCatalogNeedRebuild() && _Project->GetCatalogBaseFilename() != std::string())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), U8("Каталог нужно перегенерировать"));
        }
    }

    void SetupProject::DrawImgsByCutPattern(Ref<Project> _Project)
    {
        if (_Project->GetCatalogTopLeftPattern().IsExists())
        {
            ImGui::Text(U8("Верхний левый паттерн"));
            ImGui::Text(U8("На странице: %d"), _Project->GetCatalogTopLeftPattern().PageId);
        }

        if (_Project->GetCatalogBotRightPattern().IsExists())
        {
            ImGui::Text(U8("Нижний правый паттерн"));
            ImGui::Text(U8("На странице: %d"), _Project->GetCatalogBotRightPattern().PageId);
        }

        if (ImGui::Button(U8("Обрезать каталог по паттернам")))
        {
            GenImgsByCutPattern(_Project);
        }

        if (_Project->GetIsCatalogGenerated() && _Project->GetImgsByCutPatternNeedRebuild())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), U8("Картинки нужно обрезать"));
        }
    }

    void SetupProject::DrawGenRawExcel(Ref<Project> _Project)
    {
        bool useCutPatterntImgs = _Project->GetRawExcelUseCutPatternImgs();
        if (ImGui::Checkbox(U8("Использовать обрезанные картинки"), &useCutPatterntImgs))
        {
            _Project->SetRawExcelUseCutPatternImgs(useCutPatterntImgs);
        }

        if (ImGui::Button(U8("Сгенерировать Excel")))
        {
            GenRawExcel(_Project);
        }

        if (_Project->GetIsImgsByCutPatternGenerated() && _Project->GetRawExcelNeedRebuild())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), U8("Файлы Excel нужно перегенерировать"));
        }
    }

    void SetupProject::GenCatalogRawImages(Ref<Project> _Project)
    {
        if (_Project->GetCatalogBaseFilename() == std::string())
        {
            Overlay::Get()->Start(U8("Каталог не выбран!"));
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

        ScriptPopup::Get()->OpenPopup(pythonCommand,
                                      { U8("Генерация картинок из PDF"),
                                        []() {
                                            ImGui::Text(U8("Работает скрипт преобразования pdf в картинки"));
                                            ImGui::Text(U8("Это может занять несколько минут"));
                                            ImGui::Text(U8("После его завершения можно закрыть это окно"));
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
            Overlay::Get()->Start(U8("Левый верхний паттерн отсутствует!"));
            return;
        }

        if (!_Project->GetCatalogBotRightPattern().IsExists())
        {
            Overlay::Get()->Start(U8("Правый нижний паттерн отсутствует!"));
            return;
        }

        if (!_Project->GetIsCatalogGenerated())
        {
            Overlay::Get()->Start(U8("Каталог не сгенерирован!"));
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

        ScriptPopup::Get()->OpenPopup(pythonCommand,
                                      { U8("Обрезание картинок каталога"),
                                        []() {
                                            ImGui::Text(U8("Работает скрипт обрезания картинок"));
                                            ImGui::Text(U8("Это может занять несколько минут"));
                                            ImGui::Text(U8("После его завершения можно закрыть это окно"));
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
            Overlay::Get()->Start(U8("Обрезаные картинки по паттерну не сгенерированы!"));
            return;
        }

        if (!_Project->GetIsCatalogGenerated() && !useCutPatterntImgs)
        {
            Overlay::Get()->Start(U8("Каталог не сгенерирован!"));
            return;
        }

        FileSystemUtils::RemoveAllInFolder(_Project->GetRawExcelPath());
        // TODO: Delete RawExcel Versions ???

        PythonCommand pythonCommand("./assets/scripts/extract_tables_to_xlsx.py");
        pythonCommand.AddArg(useCutPatterntImgs ? _Project->GetCutByPatternImgsPath() : _Project->GetRawImgPath());
        pythonCommand.AddArg(_Project->GetProjectFilename());
        pythonCommand.AddArg(_Project->GetRawExcelPath());
        pythonCommand.AddArg(6);

        ScriptPopup::Get()->OpenPopup(pythonCommand,
                                      { U8("Генерация Excel из картинок"),
                                        []() {
                                            ImGui::Text(U8("Работает скрипт генерации Excel"));
                                            ImGui::Text(U8("Это может занять несколько минут"));
                                            ImGui::Text(U8("После его завершения можно закрыть это окно"));
                                        },
                                        [=]() {
                                            _Project->OnGenRawExcel();
                                            Project::Save(_Project);
                                        } });
    }

}    // namespace LM
