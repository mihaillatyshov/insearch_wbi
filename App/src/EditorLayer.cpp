#include "EditorLayer.h"

#include <fstream>
#include <iomanip>

#include <imgui.h>

#include "Engine/Core/Application.h"
#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/json.hpp"
#include "Engine/Utils/utf8.h"

#include "ImGui/Overlays/Overlay.h"
#include "ImGui/Overlays/ScriptPopup.h"
#include "ImGui/Tables/Table.h"
#include "Serializer/Serializer.h"

namespace LM
{

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

        m_SetupProjectWindow.Draw(m_Project);
        Overlay::Get()->Draw();
        ScriptPopup::Get()->Draw();

        ImGui::ShowDemoWindow();
    }

    void EditorLayer::OpenProject()
    {
        if (Ref<Project> project = Project::Open(); project != Project::s_ProjectNotOpen)
        {
            m_Project = project;
            m_SetupProjectWindow.Open();
        }
    }

    void EditorLayer::NewProject()
    {
        if (Ref<Project> project = Project::New(); project != Project::s_ProjectNotOpen)
        {
            m_Project = project;
            m_SetupProjectWindow.Open();
        }
    }

    void EditorLayer::SaveProject() { Project::Save(m_Project); }

    void EditorLayer::SaveProjectAs()
    {
        // TODO: Save and copy assets
    }

    void EditorLayer::CloseProject() { m_Project = Project::s_ProjectNotOpen; }

}    // namespace LM
