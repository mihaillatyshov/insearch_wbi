#include "IPageView.h"

#include "PageViewManager.h"

#include <imgui.h>

namespace LM
{

    IPageView::IPageView() { }

    void IPageView::Draw(Ref<Project> _Project, int _PageId)
    {
        m_Project = _Project;
        m_PageId = _PageId;
        m_BasePath = GetBasePath();

        if (ImGui::Begin(GetWindowName()))
        {
            PageViewManager::Get()->DrawViewTopMenu();
            DrawTopMenuExtras();

            DrawWindowContent();

            DrawExtras();
        }
        ImGui::End();

        m_Project = Project::s_ProjectNotOpen;
    }

}    // namespace LM
