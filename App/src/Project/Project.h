#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "Engine/Core/Base.h"

#include "Catalog.h"
#include "Processing/GenImgsByCutPattern.h"
#include "Processing/GenRawExcel.h"

namespace LM
{

    class Serializer;

    class Project
    {
    public:
        static Ref<Project> New(std::string_view _Folder, std::string_view _Name);
        static Ref<Project> Open();
        static Ref<Project> Open(std::string _FileName);
        static bool Save(Ref<Project> _Project);

        // =========== Paths ==================================================
        inline const std::string& GetFolder() const { return m_Folder; }
        inline void SetFolder(std::string_view _Folder) { m_Folder = _Folder; }
        const std::string GetPathInFolder(std::string_view _Path) const;

        inline std::string GetProjectFilename() const { return GetPathInFolder(s_ProjectFileName); }
        inline std::string GetCatalogFilename() const { return GetPathInFolder("data/catalog/catalog.pdf"); }
        inline std::string GetTmpPath() const { return GetPathInFolder("tmp/"); }

        std::string GetRawImgPath() const;
        std::string GetRawImgPrevPath() const;
        std::string GetCutByPatternImgsPath() const;
        std::string GetCutByPatternImgsPrevPath() const;
        std::string GetRawExcelPath() const;

        // =========== Catalog ================================================
        inline std::string GetCatalogBaseFilename() const { return m_Catalog.BaseFileName; }
        inline void SetCatalogBaseFilename(std::string_view _FileName)
        {
            m_Catalog.BaseFileName = _FileName;
            m_Catalog.NeedRebuild = m_Catalog.IsNeedRebuild(m_LastBuildCatalog);
        }

        inline bool GetCatalogNeedRebuild() const { return m_Catalog.NeedRebuild; }

        inline bool GetIsCatalogGenerated() const { return m_Catalog.IsGenerated; }

        inline bool GetCatalogSplitPages() const { return m_Catalog.SplitPages; }
        inline void SetCatalogSplitPages(bool _SplitPages)
        {
            m_Catalog.SplitPages = _SplitPages;
            m_Catalog.NeedRebuild = m_Catalog.IsNeedRebuild(m_LastBuildCatalog);
        }

        inline int GetCatalogImgQuality() const { return m_Catalog.ImgQuality; }
        inline void SetCatalogImgQuality(int _ImgQuality)
        {
            m_Catalog.ImgQuality = glm::clamp(_ImgQuality, 1, 8);
            m_Catalog.NeedRebuild = m_Catalog.IsNeedRebuild(m_LastBuildCatalog);
        }

        // =========== Catalog Cut Pattern ====================================
        inline const CatalogCutPattern& GetCatalogTopLeftPattern() const { return m_Catalog.TopLeftCutPattern; }
        inline void SetCatalogTopLeftPattern(const CatalogCutPattern& _CutPattern)
        {
            m_Catalog.TopLeftCutPattern = _CutPattern;
            m_GenImgsByCutPattern.NeedRebuild = true;
        }
        inline const CatalogCutPattern& GetCatalogBotRightPattern() const { return m_Catalog.BotRightCutPattern; }
        inline void SetCatalogBotRightPattern(const CatalogCutPattern& _CutPattern)
        {
            m_Catalog.BotRightCutPattern = _CutPattern;
            m_GenImgsByCutPattern.NeedRebuild = true;
        }

        inline bool GetIsImgsByCutPatternGenerated() const { return m_GenImgsByCutPattern.IsGenerated; }
        inline bool GetImgsByCutPatternNeedRebuild() const { return m_GenImgsByCutPattern.NeedRebuild; }

        // =========== Raw Exclel ====================================
        inline bool GetIsRawExcelGenerated() const { return m_GenRawExcel.IsGenerated; }
        inline bool GetRawExcelNeedRebuild() const { return m_GenRawExcel.NeedRebuild; }

        inline bool GetRawExcelUseCutPatternImgs() const { return m_GenRawExcel.UseCutPattern; }
        inline void SetRawExcelUseCutPatternImgs(bool _UseCutPatternImgs)
        {
            m_GenRawExcel.UseCutPattern = _UseCutPatternImgs;
            m_GenRawExcel.NeedRebuild = true;
        }

        // =========== Catalog Exclude Pages ==================================
        const std::vector<uint32_t>& GetGeneratedCatalogExcludePages() const { return m_GeneratedCatalogExcludePages; }
        bool IsPageInGeneratedCatalogExcludePages(uint32_t _PageId) const;
        bool AddGeneratedCatalogExcludePage(uint32_t _PageId);
        bool RemoveGeneratedCatalogExcludePage(uint32_t _PageId);

        // =========== Generation Events ==================================
        void OnGenCatalogRawImgs();
        void OnGenImgsByCutPattern();
        void OnGenRawExcel();

    public:
        friend Serializer;

        static inline const Ref<Project> s_ProjectNotOpen = Ref<Project>();
        static inline const std::string s_ProjectFileName = "project.lmproj";

        std::string GetPathInFolderAndCreateDirs(std::string_view _Path) const;

    private:
        Project(std::string_view _Folder = std::string());

    protected:
        std::string m_Folder;

        Catalog m_Catalog;
        Catalog m_LastBuildCatalog;

        GenRawExcel m_GenRawExcel;
        GenImgsByCutPattern m_GenImgsByCutPattern;

        std::vector<uint32_t> m_GeneratedCatalogExcludePages;
    };

}    // namespace LM
