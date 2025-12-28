#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Processing/XlsxPageViewData.h"

#include "Engine/Core/Base.h"

#include "Catalog.h"
#include "Processing/GenImgsByCutPattern.h"
#include "Processing/GenRawExcel.h"

namespace LM
{

    struct SerializerGetPropertiesAll;

    enum class ProjectVariant
    {
        kPdfTablesWithOcr = 0,
        kPdfTablesWithoutOcr = 1,
        kExcelTables = 2,
    };

    inline std::string ConvertProjectVariantToString(ProjectVariant _Type)
    {
        switch (_Type)
        {
            case ProjectVariant::kPdfTablesWithOcr: return "Из Pdf с использованием Ocr";
            case ProjectVariant::kPdfTablesWithoutOcr: return "Из Pdf без Ocr";
            case ProjectVariant::kExcelTables: return "Из папки Excel";
            default: return "Unknown";
        }
    }

    // TODO: Make folders as std::filesystem::path
    class Project
    {
    public:
        struct VariantExcelTables
        {
            explicit VariantExcelTables(const Project* _Owner) : m_Owner(_Owner) { }

            std::filesystem::path GetBasePath() const { return m_Owner->GetPathInFolderAndCreateDirs("data/excel/"); }
            std::filesystem::path GetXlsxStartupPath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_startup/");
            }
            std::filesystem::path GetXlsxAddExtraInfoPath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info/");
            }
            std::filesystem::path GetXlsxAddExtraInfoWithProcessedImagesPath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info_with_processed_images/");
            }
            std::filesystem::path GetXlsxForServerImportPath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_for_server_import/");
            }
            std::filesystem::path GetXlsxAddExtraInfoYg1Path() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info_yg1-shop/");
            }
            std::filesystem::path GetImgsPerPagePath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_per_page/");
            }
            std::filesystem::path GetImgsSimpleRulePath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_simple_rule/");
            }
            std::filesystem::path GetImgsProcessedPath() const
            {
                return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_processed/");
            }

        private:
            const Project* m_Owner;
        };

    public:
        static Ref<Project> New(std::string_view _Folder, std::string_view _Name);
        static Ref<Project> Open();
        static Ref<Project> Open(std::string _FileName);
        static bool Save(Ref<Project> _Project);

        inline ProjectVariant GetType() const { return m_Type; }
        inline void SetType(ProjectVariant _Type) { m_Type = _Type; }

        // =========== Paths ==================================================
        inline const std::string& GetFolder() const { return m_Folder; }
        inline void SetFolder(std::string_view _Folder) { m_Folder = _Folder; }
        const std::string GetPathInFolder(std::string_view _Path) const;
        std::string GetPathInFolderAndCreateDirs(std::string_view _Path) const;

        inline std::string GetProjectFilename() const { return GetPathInFolder(s_ProjectFileName); }
        inline std::filesystem::path GetDataFolder() const { return GetPathInFolderAndCreateDirs("data"); }
        inline std::filesystem::path GetBackupFolder() const { return GetPathInFolderAndCreateDirs("backup"); }
        inline std::string GetCatalogFilename() const { return GetPathInFolder("data/catalog/catalog.pdf"); }
        inline std::string GetTmpPath() const { return GetPathInFolder("tmp/"); }

        std::string GetPdfTablesWithOcrTypeRawImgPath() const;
        std::string GetPdfTablesWithOcrTypeRawImgPrevPath() const;
        std::string GetPdfTablesWithOcrTypeCutByPatternImgsPath() const;
        std::string GetPdfTablesWithOcrTypeCutByPatternImgsPrevPath() const;
        std::string GetPdfTablesWithOcrTypeRawExcelPath() const;

        void MakeBackup();

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

        XlsxPageViewData& GetXlsxPageViewData();

        const VariantExcelTables& GetVariantExcelTables() const { return m_VariantExcelTables; }

    public:
        friend SerializerGetPropertiesAll;

        static inline const Ref<Project> s_ProjectNotOpen = Ref<Project>();
        static inline const std::string s_ProjectFileName = "project.lmproj";

    private:
        Project(std::string_view _Folder = std::string());

    protected:
        ProjectVariant m_Type = ProjectVariant::kPdfTablesWithOcr;

        std::string m_Folder;

        Catalog m_Catalog;

        Catalog m_LastBuildCatalog;
        GenRawExcel m_GenRawExcel;
        GenImgsByCutPattern m_GenImgsByCutPattern;

        std::vector<uint32_t> m_GeneratedCatalogExcludePages;

        std::optional<XlsxPageViewData> m_XlsxPageViewData;

        VariantExcelTables m_VariantExcelTables { this };
    };

}    // namespace LM
