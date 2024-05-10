#pragma once

#include "IPageView.h"
#include "Project/Project.h"

#include <vector>

namespace LM
{

    class PageViewManager
    {
    public:
        static Ref<PageViewManager> Get();

        void DrawMenuItem();
        void DrawViewTopMenu();
        void DrawViews(Ref<Project> _Project);

        int SetPage(int _PageId);

    public:
        static inline const float kBntSizeCoef = 2.0f;

    protected:
        PageViewManager();

    protected:
        Ref<Project> m_Project;
        int m_PageId = 0;

        std::vector<Scope<IPageView>> m_Views;
    };

}    // namespace LM
