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
        void Draw(Ref<Project> _Project);

        inline void Open() { m_IsOpen = true; }
        inline void Close() { m_IsOpen = false; }

    protected:
        void DrawCatalog(Ref<Project> _Project);

        void DrawPythonBuffer();

    protected:
        bool m_IsOpen = false;

        std::atomic_bool m_IsPythonRuning;
        std::mutex m_PythonBufferMtx;
        std::string m_PythonBuffer;
    };

}    // namespace LM
