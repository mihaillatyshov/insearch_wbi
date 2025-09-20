#pragma once

#include "Engine/Textures/Texture2D.h"
#include "ImGui/Project/CreateNewProject.h"
#include "ImGui/Project/SetupProject.h"
#include "Project/Project.h"

#include "Engine/Layers/Layer.h"

namespace LM
{

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        ~EditorLayer();

        void OnImGuiRender() override;

    protected:
        void OpenProject();
        void NewProject();
        void SaveProject();
        void SaveProjectAs();
        void CloseProject();

        void SaveLastProjectPath();
        void ClearLastProjectPath();

    protected:
        Ref<Project> m_Project;

        SetupProject m_SetupProjectWindow;
        CreateNewProject m_CreateNewProject;

        Ref<Texture2D> m_AppLogoLight;

        bool m_IsDrawImGuiDemoWindow = false;
        bool m_IsDrawSelectConstructionWindow = false;
        bool m_IsDrawTestTableWindow = false;
    };
}    // namespace LM
