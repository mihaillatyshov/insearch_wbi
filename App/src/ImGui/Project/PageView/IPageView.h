#pragma once

#include "Project/Project.h"

#include <string>

namespace LM
{

    class IPageView
    {
    public:
        IPageView();

        void Draw(Ref<Project> _Project, int _PageId);

    protected:
        virtual std::string GetBasePath() const = 0;
        virtual std::string GetFileName() const = 0;
        virtual const char* GetWindowName() const = 0;

        virtual void DrawWindowContent() = 0;

        virtual void DrawTopMenuExtras() {};
        virtual void DrawExtras() {};

    protected:
        Ref<Project> m_Project;
        int m_PageId = 0;
        std::string m_BasePath;

        const float kLineThickness = 2.0f;
    };

}    // namespace LM
