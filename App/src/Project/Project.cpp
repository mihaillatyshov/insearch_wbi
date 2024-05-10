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
            Overlay::Get()->Start(Format(U8("�� ������� ��������� ������: \n{}"), _FileName));
            return false;
        }
        fout << std::setw(4) << _Json;

        return true;
    }

    Project::Project(std::string_view _Folder) : m_Folder(_Folder) { }

    Ref<Project> Project::New(std::string_view _Folder, std::string_view _Name)
    {
        std::filesystem::path folder = std::filesystem::path(_Folder) / std::filesystem::path(_Name);
        Ref<Project> project = Ref<Project>(new Project(folder.string()));

        if (std::filesystem::create_directories(project->m_Folder) && Save(project))
        {
            project->m_LastBuildCatalog = Catalog::CreateDefaultLastBuildCatalog();
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

        return Open(filename);
    }

    Ref<Project> Project::Open(std::string _FileName)
    {
        Ref<Project> project = Ref<Project>(new Project());

        std::ifstream infile(_FileName);
        if (!infile.is_open())
        {
            Overlay::Get()->Start(Format(U8("�� ������� ������� ������: \n{}"), _FileName));
            return s_ProjectNotOpen;
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            if (!Serializer::DeSerialize(project, json))
            {
                Overlay::Get()->Start(Format(U8("�� ������ ������ �������: \n{}"), _FileName));
                return s_ProjectNotOpen;
            }
            project->m_LastBuildCatalog =
                project->m_Catalog.NeedRebuild ? Catalog::CreateDefaultLastBuildCatalog() : project->m_Catalog;
            Overlay::Get()->Start(Format(U8("������ ������� ������: \n{}"), _FileName));

            return project;
        }
        catch (...)
        {
            Overlay::Get()->Start(Format(U8("������ �� ����� ������ ������� json: \n{}"), _FileName));
        }

        return s_ProjectNotOpen;
    }

    bool Project::Save(Ref<Project> _Project)
    {
        if (_Project == s_ProjectNotOpen || _Project->GetFolder().empty())
        {
            return false;
        }

        return WriteJsonToFile(_Project->GetPathInFolder(s_ProjectFileName), Serializer::Serialize(_Project));
    }

    const std::string Project::GetPathInFolder(std::string_view _Path) const
    {
        return (std::filesystem::path(m_Folder) / std::filesystem::path(_Path)).string();
    }

    std::string Project::GetPathInFolderAndCreateDirs(std::string_view _Path) const
    {
        std::string path = GetPathInFolder(_Path);
        std::filesystem::create_directories(path);
        return path;
    }

    std::string Project::GetRawImgPath() const { return GetPathInFolderAndCreateDirs("data/catalog/raw_img/"); }

    std::string Project::GetRawImgPrevPath() const
    {
        return GetPathInFolderAndCreateDirs("data/catalog/prev_raw_img/");
    }

    std::string Project::GetCutByPatternImgsPath() const
    {
        return GetPathInFolderAndCreateDirs("data/catalog/cut_by_pattern_img/");
    }

    std::string Project::GetCutByPatternImgsPrevPath() const
    {
        return GetPathInFolderAndCreateDirs("data/catalog/prev_cut_by_pattern_img/");
    }

    std::string Project::GetRawExcelPath() const { return GetPathInFolderAndCreateDirs("data/catalog/raw_xlsx/"); }

    bool Project::IsPageInGeneratedCatalogExcludePages(uint32_t _PageId) const
    {
        const auto itPageId =
            std::find(m_GeneratedCatalogExcludePages.begin(), m_GeneratedCatalogExcludePages.end(), _PageId);
        return itPageId != m_GeneratedCatalogExcludePages.end();
    }

    bool Project::AddGeneratedCatalogExcludePage(uint32_t _PageId)
    {
        if (IsPageInGeneratedCatalogExcludePages(_PageId))
        {
            return false;
        }

        m_GeneratedCatalogExcludePages.push_back(_PageId);
        std::sort(m_GeneratedCatalogExcludePages.begin(), m_GeneratedCatalogExcludePages.end());

        return true;
    }

    bool Project::RemoveGeneratedCatalogExcludePage(uint32_t _PageId)
    {
        if (!IsPageInGeneratedCatalogExcludePages(_PageId))
        {
            return false;
        }

        m_GeneratedCatalogExcludePages.erase(
            std::find(m_GeneratedCatalogExcludePages.begin(), m_GeneratedCatalogExcludePages.end(), _PageId));

        return true;
    }

    void Project::OnGenCatalogRawImgs()
    {
        m_LastBuildCatalog = m_Catalog;
        m_Catalog.IsGenerated = true;
        m_Catalog.NeedRebuild = false;
        m_GenImgsByCutPattern.NeedRebuild = true;
    }

    void Project::OnGenImgsByCutPattern()
    {
        m_GenImgsByCutPattern.IsGenerated = true;
        m_GenImgsByCutPattern.NeedRebuild = false;
        m_GenImgsByCutPattern.Version = 0;

        if (m_GenRawExcel.UseCutPattern)
        {
            m_GenRawExcel.NeedRebuild = true;
        }
    }

    void Project::OnGenRawExcel()
    {
        m_GenRawExcel.IsGenerated = true;
        m_GenRawExcel.NeedRebuild = false;
        m_GenRawExcel.Version = 0;
    }

}    // namespace LM
