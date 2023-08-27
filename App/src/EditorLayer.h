#pragma once

#include "Engine/Layers/Layer.h"

#include "ImGui/Project/SetupProject.h"
#include "Project/Project.h"

namespace LM
{

    class EditorLayer : public Layer
    {
    public:
        void OnImGuiRender() override;

    protected:
        void OpenProject();
        void NewProject();
        void SaveProject();
        void SaveProjectAs();
        void CloseProject();

    protected:
        Ref<Project> m_Project;

        SetupProject m_SetupProjectWindow;
    };
}    // namespace LM
