#pragma once

#include <string>

namespace LM
{

    class SetupProject;

    class Serializer;

    class Project
    {
    public:
        Project();
        Project(std::string_view _FileName);

        inline const std::string& GetFileName() const { return m_FileName; }
        inline void SetFileName(std::string_view _FileName) { m_FileName = _FileName; }
        inline const std::string& GetAssetsPath() const { return m_AssetsPath; }
        std::string GetCatalogFilename() const { return m_AssetsPath + "catalog.pdf"; }

        friend SetupProject;
        friend Serializer;

    protected:
        std::string m_FileName;
        std::string m_AssetsPath;
        std::string m_CatalogBaseFileName;
    };

}    // namespace LM
