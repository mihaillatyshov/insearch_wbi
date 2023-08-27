#pragma once

#include <string>

#include "glm/glm.hpp"

#include "Engine/Core/Base.h"

#include "Catalog.h"

namespace LM
{

    class Serializer;
    class NewSerializer;

    class Project
    {
    public:
        static Ref<Project> New();
        static Ref<Project> Open();
        static bool Save(Ref<Project> _Project);

        inline const std::string& GetFileName() const { return m_FileName; }
        inline void SetFileName(std::string_view _FileName) { m_FileName = _FileName; }

        inline const std::string& GetAssetsPath() const { return m_AssetsPath; }
        inline std::string GetCatalogFilename() const { return m_AssetsPath + "catalog.pdf"; }

        // =========== Catalog ===============================
        inline std::string GetCatalogBaseFilename() const { return m_Catalog.BaseFileName; }
        inline void SetCatalogBaseFilename(std::string_view _FileName)
        {
            m_Catalog.BaseFileName = _FileName;
            m_Catalog.NeedRebuild = true;
        }

        inline bool GetCatalogNeedRebuild() const { return m_Catalog.NeedRebuild; };

        inline bool GetCatalogSplitPages() const { return m_Catalog.SplitPages; }
        inline void SetCatalogSplitPages(bool _SplitPages)
        {
            m_Catalog.SplitPages = _SplitPages;
            m_Catalog.NeedRebuild = true;
        }

        inline int GetCatalogImgQuality() const { return m_Catalog.ImgQuality; }
        inline void SetCatalogImgQuality(int _ImgQuality)
        {
            m_Catalog.ImgQuality = glm::clamp(_ImgQuality, 1, 8);
            m_Catalog.NeedRebuild = true;
        }

        //
        std::string GetRawImgPath() const;

    public:
        friend Serializer;
        friend NewSerializer;

        static inline const Ref<Project> s_ProjectNotOpen = Ref<Project>();

    private:
        Project(std::string_view _FileName);

    protected:
        std::string m_FileName;
        std::string m_AssetsPath;
        Catalog m_Catalog;
    };

}    // namespace LM
