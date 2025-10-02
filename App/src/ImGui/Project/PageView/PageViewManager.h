#pragma once

#include "IPageView.h"
#include "Project/Project.h"

#include <unordered_map>
#include <vector>

namespace LM
{

    class PageViewManager
    {
    protected:
        static inline const std::string kHashPdfOcr = "PdfOcr";
        static inline const std::string kHashPdf = "Pdf";
        static inline const std::string kHashExcelFolder = "ExcelFolder";

    public:
        PageViewManager();

        static bool Save();
        static bool Clear();
        static bool OnAppClose(Ref<Project> _Project);

        static Ref<PageViewManager> GetPdfOcr();
        static Ref<PageViewManager> GetPdf();
        static Ref<PageViewManager> GetExcelFolder();
        static Ref<PageViewManager> GetCurrent();

        void DrawMenuItem();
        void DrawViewTopMenu();
        void DrawViews(Ref<Project> _Project);

        int SetPage(int _PageId);

    public:
        static inline const float kBntSizeCoef = 2.0f;

    protected:
        Ref<Project> m_Project;
        int m_PageId = 0;

        std::vector<Ref<IPageView>> m_Views;

        static inline std::string s_CurrentManagerHash = kHashPdfOcr;
        static inline std::unordered_map<std::string, Ref<PageViewManager>> s_Managers;
    };

}    // namespace LM
