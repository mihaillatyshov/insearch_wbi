#pragma once

#include <string>

#include <Project/Project.h>

namespace LM
{

    class CreateNewProject
    {
    public:
        bool Draw();
        bool Create();

        inline void Open() { m_IsOpen = true; }
        void Close();

        Ref<Project> GetNewProject() const { return m_Project; }

    private:
        std::string m_Name;
        std::string m_Folder;

        bool m_IsOpen = false;

        Ref<Project> m_Project;
    };

}    // namespace LM
