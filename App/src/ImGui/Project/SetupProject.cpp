#include "SetupProject.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <imgui.h>

#include "Engine/Utils/FileDialogs.h"
#include "Engine/Utils/utf8.h"

#include <Python.h>

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
            ImGui::Text(U8("Папка ассетов: %s"), _Project->m_AssetsPath.c_str());
            ImGui::Text(U8("Оригинал каталога: %s"), _Project->m_CatalogBaseFileName.c_str());
            if (ImGui::Button(U8("Изменить оригинал каталога")))
            {
                if (std::string filename = FileDialogs::OpenFile(kFileDialogsFilter); filename != std::string())
                {
                    if (std::filesystem::copy_file(filename, _Project->GetCatalogFilename()))
                    {
                        _Project->m_CatalogBaseFileName = filename;
                    }
                }
            }

            if (ImGui::Button("Test Python"))
            {
                m_IsPythonRuning = true;
                ImGui::OpenPopup("Test Python");

                std::thread thread([&]() {
                    Py_Initialize();

                    PyRun_SimpleString("from time import time,ctime,sleep\n"
                                       "print('[Python]: Today is',ctime(time()))\n"
                                       "sleep(5)\n");

                    std::cout << "Py_FinalizeEx: " << Py_FinalizeEx() << std::endl;
                    m_IsPythonRuning = false;
                });
                thread.detach();
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Test Python", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(U8("Работает скрипт преобразования pdf в картинки"));
                ImGui::Text(U8("Это может занять несколько минут"));
                ImGui::Text(U8("После его завершения можно закрыть это окно"));

                ImGui::Text("\n");
                ImGui::Separator();

                ImGui::BeginDisabled(m_IsPythonRuning);
                if (ImGui::Button(U8("Закрыть"), ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndDisabled();

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

}    // namespace LM
