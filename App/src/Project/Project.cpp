#include "Project/Project.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>

#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/FileDialogs.h"
#include "Engine/Utils/json.hpp"
#include "Engine/Utils/utf8.h"

#include "ImGui/Overlays/Overlay.h"
#include "Serializer/Serializer.h"

namespace LM
{

    const FileDialogs::Filter kFileDialogsFilter { "InSearch Project (*.lmproj)", "*.lmproj" };

    static bool WriteJsonToFile(std::string_view _FileName, nlohmann::json _Json)
    {
        std::ofstream fout(_FileName.data());
        if (!fout.is_open())
        {
            Overlay::Get()->Start(Format(U8("Не удалось сохранить проект: \n{}"), _FileName));
            return false;
        }
        fout << std::setw(4) << _Json;

        return true;
    }

    Project::Project(std::string_view _FileName) : m_FileName(_FileName) { }

    Ref<Project> Project::New()
    {
        std::string filename = FileDialogs::SaveFile(kFileDialogsFilter);
        if (filename == std::string())
        {
            return s_ProjectNotOpen;
        }

        Ref<Project> project = Ref<Project>(new Project(filename));

        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        project->m_AssetsPath =
            std::string(RES_FOLDER) + "assets/projects/" + std::format("{:%Y_%m_%d__%H_%M_%OS}", time) + "/";

        if (Save(project))
        {
            std::filesystem::create_directories(project->m_AssetsPath);
            return project;
        }

        return s_ProjectNotOpen;
    }

    Ref<Project> Project::Open()
    {
        std::string filename = FileDialogs::OpenFile(kFileDialogsFilter);
        if (filename == std::string())
        {
            return s_ProjectNotOpen;
        }

        Ref<Project> project = Ref<Project>(new Project(filename));

        std::ifstream infile(filename);
        if (!infile.is_open())
        {
            Overlay::Get()->Start(Format(U8("Не удалось открыть проект: \n{}"), filename));
            return s_ProjectNotOpen;
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            if (!Serializer::DeSerialize(project, json))
            {
                Overlay::Get()->Start(Format(U8("Не верный формат проекта: \n{}"), filename));
                return s_ProjectNotOpen;
            }
            Overlay::Get()->Start(Format(U8("Проект успешно открыт: \n{}"), filename));
            return project;
        }
        catch (...)
        {
            Overlay::Get()->Start(Format(U8("Ошибка во время чтения формата json: \n{}"), filename));
        }

        return s_ProjectNotOpen;
    }

    bool Project::Save(Ref<Project> _Project)
    {
        if (_Project == s_ProjectNotOpen || _Project->GetFileName() == std::string())
        {
            return false;
        }

        return WriteJsonToFile(_Project->GetFileName(), Serializer::Serialize(_Project));
    }

    std::string Project::GetRawImgPath() const
    {
        std::string imgPath = m_AssetsPath + "raw_img/";
        std::filesystem::create_directories(imgPath);
        return imgPath;
    }

}    // namespace LM
