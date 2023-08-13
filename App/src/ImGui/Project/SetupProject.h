#pragma once

#include "Engine/Core/Base.h"
#include "Project/Project.h"
#include <atomic>

namespace LM
{

    class SetupProject
    {
    public:
        void Draw(Ref<Project> _Project);

        inline void Open() { m_IsOpen = true; }
        inline void Close() { m_IsOpen = false; }

    protected:
        bool m_IsOpen = false;

        std::atomic_bool m_IsPythonRuning;
    };

}    // namespace LM
