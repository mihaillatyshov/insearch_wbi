#pragma once

#include <FileBrowser/ImGuiFileBrowser.h>

namespace LM
{

    class OpenProject
    {
    public:
        bool Draw();
        bool Create();

        inline void Open() { m_IsOpen = true; }
        void Close();
    private:
        bool m_IsOpen = false;
    };

}    // namespace LM
