#include "Serializer.h"

#include "Project/Project.h"

using namespace std::string_literals;

namespace LM
{

#define SET_IF_CONTAINS_OR_RETURN_FALSE(var, key)                                                                      \
    if (!_Json.contains(key))                                                                                          \
        return false;                                                                                                  \
    var = _Json[key]

    const std::string kAssetsPath = "assetsPath"s;
    const std::string kCatalogBaseFileName = "catalogBaseFileName"s;

    nlohmann::json Serializer::Serialize(Ref<Project> _Project)
    {
        nlohmann::json result;

        result[kAssetsPath] = _Project->m_AssetsPath;
        result[kCatalogBaseFileName] = _Project->m_Catalog.CatalogBaseFileName;

        return result;
    }

    bool Serializer::DeSerialize(Ref<Project> _Project, nlohmann::json _Json)
    {
        if (!_Json.is_object())
        {
            return false;
        }

        try
        {
            _Project->m_AssetsPath = _Json[kAssetsPath];
            SET_IF_CONTAINS_OR_RETURN_FALSE(_Project->m_Catalog.CatalogBaseFileName, kCatalogBaseFileName);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

}    // namespace LM
