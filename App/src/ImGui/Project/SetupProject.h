#pragma once

#include <atomic>
#include <mutex>
#include <string>

#include "Engine/Core/Base.h"
#include "Project/Project.h"

namespace LM
{

    class SetupProject
    {
    public:
        SetupProject();

        void Draw(Ref<Project> _Project);

        inline void Open() { m_IsOpen = true; }
        inline void Close() { m_IsOpen = false; }

    protected:
        void DrawCatalog(Ref<Project> _Project);
        void DrawImgsByCutPattern(Ref<Project> _Project);
        void DrawGenRawExcel(Ref<Project> _Project);

        void GenCatalogRawImages(Ref<Project> _Project);
        void GenImgsByCutPattern(Ref<Project> _Project);
        void GenRawExcel(Ref<Project> _Project);
    protected:
        bool m_IsOpen = false;
    };

}    // namespace LM
