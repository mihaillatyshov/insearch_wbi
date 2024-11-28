#include "EditorLayer.h"

#include "ImGui/Constructions/SelectConstructionFromTree.hpp"
#include "ImGui/Overlays/Overlay.h"
#include "ImGui/Overlays/ScriptPopup.h"
#include "ImGui/Project/PageView/PageViewManager.h"
#include "ImGui/Tables/Table.h"
#include "Serializer/Serializer.h"

#include "Engine/Core/Application.h"
#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/json.hpp"
#include "Engine/Utils/utf8.h"

#include <imgui.h>

#include <fstream>
#include <iomanip>

namespace LM
{

    const std::string kLastProjectPath = "./assets/last_project.json";
    const std::string kJsonLastProjectPath = "folder";

    EditorLayer::EditorLayer()
    {
        std::ifstream infile(kLastProjectPath);
        if (!infile.is_open())
        {
            LOGI("Last project file not found");
            return;
        }

        nlohmann::json json;
        infile >> json;

        if (Ref<Project> project = Project::Open(json[kJsonLastProjectPath]); project != Project::s_ProjectNotOpen)
        {
            LOGI("Opened last project: ", project->GetCatalogFilename());
            m_Project = project;
            m_SetupProjectWindow.Open();
        }

        SelectConstructionFromTree::LoadTreeFromDefaultFile();
    }

    void EditorLayer::OnImGuiRender()
    {
        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                            ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
        {
            ImGui::PopStyleVar(2);
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                {
                    OpenProject();
                }

                if (ImGui::MenuItem("New Project", "Ctrl+N"))
                {
                    NewProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    SaveProject();
                }

                if (ImGui::MenuItem("Save Project As...", "Ctrl+Shift+S"))
                {
                    SaveProjectAs();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Close", "Ctrl+F4"))
                {
                    CloseProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                {
                    Application::Get().Close();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (ImGui::Begin("Test Table"))
        {
            Table::DrawTable();
        }
        ImGui::End();

        if (m_CreateNewProject.Draw())
        {
            m_Project = m_CreateNewProject.GetNewProject();
            m_CreateNewProject.Close();
            m_SetupProjectWindow.Open();
            SaveLastProjectPath();
        }

        m_SetupProjectWindow.Draw(m_Project);
        PageViewManager::Get()->DrawViews(m_Project);
        Overlay::Get()->Draw();
        ScriptPopup::Get()->Draw();

        ImGui::ShowDemoWindow();

        static SelectConstructionFromTree constrTreeTest;
        if (auto construction = constrTreeTest(); !construction.empty())
        {
            LOGW("Constr tree test message on select: ", construction);
        }
    }

    void EditorLayer::OpenProject()
    {
        if (Ref<Project> project = Project::Open(); project != Project::s_ProjectNotOpen)
        {
            m_Project = project;
            m_SetupProjectWindow.Open();
            SaveLastProjectPath();
        }
    }

    void EditorLayer::NewProject()
    {
        m_CreateNewProject.Open();
        /*if (Ref<Project> project = Project::New(); project != Project::s_ProjectNotOpen)
        {
            m_Project = project;
            m_SetupProjectWindow.Open();
        }*/
    }

    void EditorLayer::SaveProject()
    {
        Project::Save(m_Project);
        SaveLastProjectPath();
    }

    void EditorLayer::SaveProjectAs()
    {
        // TODO: Save and copy assets
    }

    void EditorLayer::CloseProject()
    {
        ClearLastProjectPath();
        m_Project = Project::s_ProjectNotOpen;
    }

    void EditorLayer::SaveLastProjectPath()
    {
        if (m_Project == Project::s_ProjectNotOpen)
        {
            return;
        }

        std::ofstream fout(kLastProjectPath);
        if (!fout.is_open())
        {
            LOGW("Can't save last project path");
            return;
        }

        nlohmann::json json;
        json[kJsonLastProjectPath] = m_Project->GetPathInFolder(Project::s_ProjectFileName);

        fout << std::setw(4) << json;
    }

    void EditorLayer::ClearLastProjectPath()
    {
        if (m_Project == Project::s_ProjectNotOpen)
        {
            return;
        }

        std::filesystem::remove(m_Project->GetPathInFolder(Project::s_ProjectFileName));
    }

}    // namespace LM
