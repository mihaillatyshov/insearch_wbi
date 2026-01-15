#include "PageViewManager.h"

#include "CutByPatternImgPageView.h"
#include "RawImgPageView.h"
#include "Utils/FileSystemUtils.h"
#include "XlsxPageView.hpp"

#include <imgui.h>

#include <filesystem>

namespace LM
{

    PageViewManager::PageViewManager() { }

    bool PageViewManager::Save()
    {
        for (auto& [managerHash, manager] : s_Managers)
        {
            for (auto& view : manager->m_Views)
            {
                view->Save();
            }
        }

        return true;
    }

    bool PageViewManager::Clear()
    {
        s_Managers.clear();
        return true;
    }

    bool PageViewManager::OnAppClose(Ref<Project> _Project)
    {
        for (auto& [managerHash, manager] : s_Managers)
        {
            // TODO: May need add function OnAppClose to better handle close event
            manager->m_Views.clear();
        }

        return true;
    }

    Ref<PageViewManager> PageViewManager::GetPdfOcr()
    {
        s_CurrentManagerHash = kHashPdfOcr;
        if (!s_Managers.contains(kHashPdfOcr))
        {
            s_Managers[kHashPdfOcr] = CreateRef<PageViewManager>();
            s_Managers[kHashPdfOcr]->m_Views.emplace_back(CreateRef<RawImgPageView>());
            s_Managers[kHashPdfOcr]->m_Views.emplace_back(CreateRef<CutByPatternImgPageView>());
            s_Managers[kHashPdfOcr]->m_Views.emplace_back(CreateRef<XlsxPageView>());
        }
        return s_Managers[kHashPdfOcr];
    }

    Ref<PageViewManager> PageViewManager::GetPdf()
    {
        s_CurrentManagerHash = kHashPdf;
        if (!s_Managers.contains(kHashPdf))
        {
            s_Managers[kHashPdf] = CreateRef<PageViewManager>();
        }
        return s_Managers[kHashPdf];
    }

    Ref<PageViewManager> PageViewManager::GetExcelFolder()
    {
        s_CurrentManagerHash = kHashExcelFolder;
        if (!s_Managers.contains(kHashExcelFolder))
        {
            s_Managers[kHashExcelFolder] = CreateRef<PageViewManager>();
            s_Managers[kHashExcelFolder]->m_Views.emplace_back(CreateRef<XlsxPageView>());
        }
        return s_Managers[kHashExcelFolder];
    }

    Ref<PageViewManager> PageViewManager::GetCurrent() { return s_Managers[s_CurrentManagerHash]; }

    void PageViewManager::DrawMenuItem() { }

    void PageViewManager::DrawViewTopMenu()
    {
        // TODO: Get pages count from pdf info ???
        int filesCount = 0;
        if (s_CurrentManagerHash == kHashPdfOcr)
        {
            filesCount = static_cast<int>(
                FileSystemUtils::FilesCountInDirectory(m_Project->GetPdfTablesWithOcrTypeRawImgPath()));
        }
        else if (s_CurrentManagerHash == kHashPdf)
        {
            // TODO: other way to get pdf pages count ???
            filesCount = static_cast<int>(
                FileSystemUtils::FilesCountInDirectory(m_Project->GetPdfTablesWithOcrTypeRawImgPath()));
        }
        else if (s_CurrentManagerHash == kHashExcelFolder)
        {
            filesCount = static_cast<int>(
                FileSystemUtils::FilesCountInDirectory(m_Project->GetVariantExcelTablesHelpers().GetXlsxStartupPath()));
        }

        ImVec2 buttonSize = { ImGui::GetFontSize() * kBntSizeCoef, ImGui::GetFontSize() * kBntSizeCoef };

        int newPageId = m_PageId;
        if (ImGui::Button("<<", buttonSize))
        {
            newPageId = 0;
        }
        ImGui::SameLine();

        ImGui::PushButtonRepeat(true);
        if (ImGui::Button("<", buttonSize))
        {
            --newPageId;
        }
        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10.0f);
        ImGui::DragInt("##PageIdInput", &newPageId, 0.01f, 0, filesCount - 1);
        ImGui::SameLine();

        if (ImGui::Button(">", buttonSize))
        {
            ++newPageId;
        }
        ImGui::PopButtonRepeat();

        ImGui::SameLine();
        if (ImGui::Button(">>", buttonSize))
        {
            newPageId = filesCount - 1;
        }

        newPageId = glm::clamp(newPageId, 0, filesCount - 1);

        if (newPageId != m_PageId)
        {
            bool isCanChangePage = true;
            for (auto& view : m_Views)
            {
                // TODO: Add handle for cases, where page can't be changed (overlay or window with error in front)
                isCanChangePage = isCanChangePage && view->OnPageWillBeChanged(m_PageId, newPageId);
            }

            if (isCanChangePage)
            {
                m_PageId = newPageId;
            }
        }
    }

    void PageViewManager::DrawViews(Ref<Project> _Project)
    {
        if (_Project == Project::s_ProjectNotOpen)
        {
            return;
        }

        m_Project = _Project;

        for (size_t i = 0; i < m_Views.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            m_Views[i]->SetContext(m_Project, m_PageId);
            m_Views[i]->Draw();
            m_Views[i]->DrawOtherWindows();
            ImGui::PopID();
        }

        m_Project = Project::s_ProjectNotOpen;
    }

    int PageViewManager::SetPage(int _PageId)
    {
        int filesCount =
            static_cast<int>(FileSystemUtils::FilesCountInDirectory(m_Project->GetPdfTablesWithOcrTypeRawImgPath()));

        m_PageId = glm::clamp(_PageId, 0, filesCount - 1);

        return m_PageId;
    }

}    // namespace LM
