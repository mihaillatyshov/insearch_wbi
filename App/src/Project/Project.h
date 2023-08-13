#pragma once

#include "Catalog.h"

#include <string>

namespace LM
{

    class Serializer;

    class Project
    {
    public:
        Project();
        Project(std::string_view _FileName);

        inline const std::string& GetFileName() const { return m_FileName; }
        inline void SetFileName(std::string_view _FileName) { m_FileName = _FileName; }

        inline const std::string& GetAssetsPath() const { return m_AssetsPath; }
        inline std::string GetCatalogFilename() const { return m_AssetsPath + "catalog.pdf"; }

        // =========== Catalog ===============================
        inline std::string GetCatalogBaseFilename() const { return m_Catalog.CatalogBaseFileName; }
        inline void SetCatalogBaseFilename(std::string_view _FileName)
        {
            m_Catalog.CatalogBaseFileName = _FileName;
            m_Catalog.NeedRebuild = true;
        }

        inline bool GetCatalogNeedRebuild() const { return m_Catalog.NeedRebuild; };

        inline bool GetCatalogSplitPages() const { return m_Catalog.SplitPages; }
        inline void SetCatalogSplitPages(bool SplitPages)
        {
            m_Catalog.SplitPages = true;
            m_Catalog.NeedRebuild = true;
        }

        inline int GetImgQuality() const { return m_Catalog.ImgQuality; }

        //
        std::string GetRawImgPath() const;

    public:
        friend Serializer;

    protected:
        std::string m_FileName;
        std::string m_AssetsPath;
        Catalog m_Catalog;
    };

}    // namespace LM
