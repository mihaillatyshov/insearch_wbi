#include "ProjectVariantExcelTables.hpp"

#include "Project.h"

namespace LM
{

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetBasePath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetXlsxStartupPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_startup/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetXlsxAddExtraInfoPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetXlsxAddExtraInfoWithProcessedImagesPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info_with_processed_images/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetXlsxForServerImportPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_for_server_import/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetXlsxAddExtraInfoYg1Path() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/xlsx_add_info_yg1-shop/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetImgsPerPagePath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_per_page/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetImgsSimpleRulePath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_simple_rule/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetImgsNoConditionPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_no_condition/");
    }

    std::filesystem::path ProjectVariantExcelTablesHelpers::GetImgsProcessedPath() const
    {
        return m_Owner->GetPathInFolderAndCreateDirs("data/excel/img_processed/");
    }

    const std::vector<std::string>& ProjectVariantExcelTables::GetPageNamesToSkipOnServerImport() const
    {
        return m_PageNamesToSkipOnServerImport;
    }

    void ProjectVariantExcelTables::TogglePageNameToSkipOnServerImport(const std::string& pageName)
    {
        auto it = std::find(m_PageNamesToSkipOnServerImport.begin(), m_PageNamesToSkipOnServerImport.end(), pageName);
        if (it != m_PageNamesToSkipOnServerImport.end())
        {
            m_PageNamesToSkipOnServerImport.erase(it);
        }
        else
        {
            m_PageNamesToSkipOnServerImport.push_back(pageName);
        }
    }

}    // namespace LM
