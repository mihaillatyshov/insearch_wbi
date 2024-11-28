#include "CreateNewProject.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nfd.hpp>

#include <Engine/Utils/ConsoleLog.h>
#include <Engine/Utils/utf8.h>

#include <iostream>

namespace LM
{

    bool CreateNewProject::Draw()
    {
        bool isCreated = false;
        if (!m_IsOpen)
        {
            return isCreated;
        }

        if (ImGui::Begin("Create Project"))
        {
            ImGui::InputText("Название проекта", &m_Name);

            ImGui::Text("Путь к проекту: %s ", m_Folder.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Обзор . . ."))
            {
                nfdu8char_t* outPath;
                nfdresult_t result = NFD::PickFolder(outPath);
                if (result == NFD_OKAY)
                {
                    m_Folder = outPath;
                    std::cout << outPath << std::endl;
                    NFD::FreePath(outPath);
                }
                else if (result == NFD_CANCEL)
                {
                    puts("User pressed cancel.");
                }
                else
                {
                    printf("Error: %s\n", NFD::GetError());
                }
            }

            if (ImGui::Button("Создать"))
            {
                isCreated = Create();
            }
        }
        ImGui::End();

        return isCreated;
    }

    bool CreateNewProject::Create()
    {
        if (m_Name.empty())
        {
            return false;
        }

        if (m_Folder.empty())
        {
            return false;
        }

        m_Project = Project::New(m_Folder, m_Name);
        return m_Project != Project::s_ProjectNotOpen;
    }

    void CreateNewProject::Close()
    {
        m_IsOpen = false;
        m_Project = nullptr;
    }

}    // namespace LM
