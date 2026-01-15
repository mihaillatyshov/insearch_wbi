#include "IPageView.h"

#include "PageViewManager.h"

#include <imgui.h>

namespace LM
{

    IPageView::IPageView() { }

    void IPageView::Draw()
    {
        if (ImGui::Begin(GetWindowName()))
        {
            PageViewManager::GetCurrent()->DrawViewTopMenu();
            DrawTopMenuExtras();

            DrawWindowContent();

            DrawExtras();
        }
        ImGui::End();
    }

    void IPageView::SetContext(Ref<Project> _Project, int _PageId)
    {
        m_Project = _Project;
        m_PageId = _PageId;
        m_BasePath = GetBasePath();
    }

    void IPageView::ClearContext() { m_Project = Project::s_ProjectNotOpen; }

}    // namespace LM
