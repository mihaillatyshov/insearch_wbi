#pragma once

#include <filesystem>

namespace LM
{

    class DatabaseDataLoader
    {
    private:
        static inline const std::filesystem::path kInsearchEditorDataScriptPath =
            "./assets/scripts/InsearchEditorData.py";

    public:
        void Draw();

        bool& GetIsOpen() { return m_IsOpen; }

    private:
        void LoadInsearchEditorData();

    private:
        bool m_IsOpen = false;
    };

}    // namespace LM
