#pragma once

#include <filesystem>

namespace LM
{

    constexpr std::string_view kAddExtraInfoYg1Parser = "yg1-shop";
    constexpr std::string_view kAddExtraInfoWbiToolsParser = "wbi-tools";

    struct SerializerGetPropertiesAll;

    class Project;

    struct ProjectVariantExcelTablesHelpers
    {
        explicit ProjectVariantExcelTablesHelpers(const Project* _Owner) : m_Owner(_Owner) { }

        std::filesystem::path GetBasePath() const;
        std::filesystem::path GetXlsxStartupPath() const;
        std::filesystem::path GetXlsxAddExtraInfoPath() const;
        std::filesystem::path GetXlsxAddExtraInfoWithProcessedImagesPath() const;
        std::filesystem::path GetXlsxForServerImportPath(std::string_view _ParserName) const;
        std::filesystem::path GetImgsPerPagePath() const;
        std::filesystem::path GetImgsSimpleRulePath() const;
        std::filesystem::path GetImgsNoConditionPath() const;
        std::filesystem::path GetImgsProcessedPath() const;
        std::filesystem::path GetJsonPrevProcessedImagesFilePath() const;

    protected:
        const Project* m_Owner;
    };

    class ProjectVariantExcelTables
    {
    public:
        const std::vector<std::string>& GetPageNamesToSkipOnServerImport() const;

        void TogglePageNameToSkipOnServerImport(const std::string& _PageName);

        bool GetIsAddExtraInfoNeedRebuild() const { return IsAddExtraInfoNeedRebuild; }
        void SetIsAddExtraInfoNeedRebuild(bool _NeedRebuild);

        bool GetIsProcessImagesNeedRebuild() const { return IsProcessImagesNeedRebuild; }
        void SetIsProcessImagesNeedRebuild(bool _NeedRebuild);

        bool GetIsUploadImagesAndPrepareXlsxForWbiToolsNeedRebuild() const
        {
            return IsUploadImagesAndPrepareXlsxForWbiToolsNeedRebuild;
        }
        void SetIsUploadImagesAndPrepareXlsxForWbiToolsNeedRebuild(bool _NeedRebuild);

        bool GetIsImportDataToWbiToolsServerNeedRebuild() const { return IsImportDataToWbiToolsServerNeedRebuild; }
        void SetIsImportDataToWbiToolsServerNeedRebuild(bool _NeedRebuild);

    public:
        friend SerializerGetPropertiesAll;

    private:
        std::vector<std::string> m_PageNamesToSkipOnServerImport;

        bool IsAddExtraInfoNeedRebuild = true;
        bool IsProcessImagesNeedRebuild = true;

        bool IsUploadImagesAndPrepareXlsxForWbiToolsNeedRebuild = true;
        bool IsImportDataToWbiToolsServerNeedRebuild = true;
    };

}    // namespace LM
