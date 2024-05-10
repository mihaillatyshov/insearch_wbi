#include "PageViewManager.h"

#include "CutByPatternImgPageView.h"
#include "RawImgPageView.h"

#include <imgui.h>

#include <filesystem>

namespace LM
{

    namespace Utils
    {

        size_t FilesCountInDirectory(std::filesystem::path path)
        {
            return std::distance(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator {});
        }

    }    // namespace Utils

    PageViewManager::PageViewManager()
    {
        m_Views.emplace_back(Scope<RawImgPageView>(new RawImgPageView));
        m_Views.emplace_back(Scope<CutByPatternImgPageView>(new CutByPatternImgPageView));
    }

    Ref<PageViewManager> PageViewManager::Get()
    {
        static Ref<PageViewManager> manager = Ref<PageViewManager>(new PageViewManager);
        return manager;
    }

    void PageViewManager::DrawMenuItem() { }

    void PageViewManager::DrawViewTopMenu()
    {
        // TODO: Get pages count from pdf info ???
        int filesCount = static_cast<int>(Utils::FilesCountInDirectory(m_Project->GetRawImgPath()));
        ImVec2 buttonSize = { ImGui::GetFontSize() * kBntSizeCoef, ImGui::GetFontSize() * kBntSizeCoef };

        if (ImGui::Button("<<", buttonSize))
        {
            m_PageId = 0;
        }
        ImGui::SameLine();

        ImGui::PushButtonRepeat(true);
        if (ImGui::Button("<", buttonSize))
        {
            --m_PageId;
        }
        ImGui::SameLine();
        if (ImGui::Button(">", buttonSize))
        {
            ++m_PageId;
        }
        ImGui::PopButtonRepeat();

        ImGui::SameLine();
        if (ImGui::Button(">>", buttonSize))
        {
            m_PageId = filesCount - 1;
        }

        m_PageId = glm::clamp(m_PageId, 0, filesCount - 1);
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
            ImGui::PushID(i);
            m_Views[i]->Draw(m_Project, m_PageId);
            ImGui::PopID();
        }

        m_Project = Project::s_ProjectNotOpen;
    }

    int PageViewManager::SetPage(int _PageId)
    {
        int filesCount = static_cast<int>(Utils::FilesCountInDirectory(m_Project->GetRawImgPath()));

        m_PageId = glm::clamp(_PageId, 0, filesCount - 1);

        return m_PageId;
    }

}    // namespace LM
