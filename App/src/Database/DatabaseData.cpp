#include "DatabaseData.hpp"

#include "ImGui/Overlays/Overlay.h"

#include "Engine/Utils/Log.hpp"
#include "Engine/Utils/json.hpp"

#include <format>
#include <fstream>
#include <ranges>
#include <xlnt/xlnt.hpp>

namespace LM
{

    namespace
    {

        template <typename T, typename Loader>
        const T& GetOrLoad(std::optional<T>& _Data, Loader&& _Loader)
        {
            if (!_Data.has_value())
            {
                _Data = _Loader();
            }
            return *_Data;
        }

        template <typename Map>
        const typename Map::mapped_type* FindInMap(const Map& _Map, std::string_view _Key)
        {
            auto it = _Map.find(std::string(_Key));
            return it != _Map.end() ? &it->second : nullptr;
        }

        std::string GetJsonString(const nlohmann::json& _Json, std::string_view _Key)
        {
            auto it = _Json.find(_Key);
            return (it != _Json.end() && it->is_string()) ? it->get<std::string>() : std::string {};
        }

        std::optional<std::string> GetJsonOptionalString(const nlohmann::json& _Json, std::string_view _Key)
        {
            const nlohmann::json& value = _Json.at(_Key);
            return value.is_null() ? std::nullopt : std::optional<std::string>(value.get<std::string>());
        }

        std::optional<nlohmann::json> ReadJsonFile(const std::filesystem::path& _Path, std::string_view _FileDescription)
        {
            std::ifstream infile(_Path);
            if (!infile.is_open())
            {
                Overlay::Get()->Start(Format("Не удалось открыть стандартный файл {}: \n{}", _FileDescription,
                                             _Path.string()));
                return std::nullopt;
            }

            try
            {
                nlohmann::json json;
                infile >> json;
                return json;
            }
            catch (std::exception& e)
            {
                LOG_CORE_WARN("{}", e.what());
                Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", _Path.string()));
                return std::nullopt;
            }
        }

    }    // namespace

    const std::vector<ConstructionTreeConstr>& DatabaseData::GetConstructionsList()
    {
        return GetOrLoad(m_ConstructionsList, &DatabaseData::LoadConstructionsList);
    }

    const std::unordered_map<std::string, std::vector<std::string>>& DatabaseData::GetConstructionsFields()
    {
        return GetOrLoad(m_ConstructionsFields, &DatabaseData::LoadConstructionsFields);
    }

    const std::unordered_map<std::string, FieldDescription>& DatabaseData::GetFieldsDescription()
    {
        return GetOrLoad(m_FieldsDescription, &DatabaseData::LoadFieldsDescription);
    }

    const std::unordered_map<std::string, std::vector<FieldRepresentationItem>>& DatabaseData::GetFieldsRepresentation()
    {
        return GetOrLoad(m_FieldsRepresentation, &DatabaseData::LoadFieldsRepresentation);
    }

    const std::vector<std::string>* DatabaseData::FindConstructionFields(std::string_view _Constr)
    {
        return FindInMap(GetConstructionsFields(), _Constr);
    }

    const FieldDescription* DatabaseData::FindFieldDescription(std::string_view _FieldName)
    {
        return FindInMap(GetFieldsDescription(), _FieldName);
    }

    const std::vector<FieldRepresentationItem>* DatabaseData::FindFieldRepresentation(std::string_view _FieldName)
    {
        return FindInMap(GetFieldsRepresentation(), _FieldName);
    }

    const ConstrExample* DatabaseData::FindConstrExample(std::string_view _Constr)
    {
        auto it = m_ConstrExamples.find(std::string(_Constr));
        if (it == m_ConstrExamples.end())
        {
            it = m_ConstrExamples.emplace(std::string(_Constr), LoadConstrExample(_Constr)).first;
        }
        return it->second.has_value() ? &*it->second : nullptr;
    }

    void DatabaseData::Reload()
    {
        LOG_CORE_INFO("Reload database data");

        m_ConstructionsList.reset();
        m_ConstructionsFields.reset();
        m_FieldsDescription.reset();
        m_FieldsRepresentation.reset();
        m_ConstrExamples.clear();
    }

    std::vector<ConstructionTreeConstr> DatabaseData::LoadConstructionsList()
    {
        std::vector<ConstructionTreeConstr> result;

        std::optional<nlohmann::json> json = ReadJsonFile(kConstructionsListFile, "списка конструкций");
        if (!json)
        {
            return result;
        }

        try
        {
            for (const auto& jsonConstr : *json)
            {
                ConstructionTreeConstr constr {
                    { .Img = GetJsonString(jsonConstr, "img_link"),
                      .Label = GetJsonString(jsonConstr, "text"),
                      .Key = jsonConstr.at("key") },
                };
                constr.Drw = GetJsonString(jsonConstr, "drw_link");
                result.emplace_back(std::move(constr));
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kConstructionsListFile.string()));
        }

        return result;
    }

    std::unordered_map<std::string, std::vector<std::string>> DatabaseData::LoadConstructionsFields()
    {
        std::unordered_map<std::string, std::vector<std::string>> result;

        std::optional<nlohmann::json> json = ReadJsonFile(kConstructionsFieldsFile, "конструкций");
        if (!json)
        {
            return result;
        }

        try
        {
            for (const auto& element : json->items())
            {
                element.value().get_to(result[element.key()]);
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(
                Format("Ошибка во время чтения формата json: \n{}", kConstructionsFieldsFile.string()));
        }

        return result;
    }

    std::unordered_map<std::string, FieldDescription> DatabaseData::LoadFieldsDescription()
    {
        std::unordered_map<std::string, FieldDescription> result;

        std::optional<nlohmann::json> json = ReadJsonFile(kFieldsDescriptionFile, "описания полей конструкций");
        if (!json)
        {
            return result;
        }

        try
        {
            for (const auto& element : *json)
            {
                result[element["fieldname"].get<std::string>()] = FieldDescription {
                    .Type = element["typ"],
                    .UnitRu = GetJsonOptionalString(element, "unitru"),
                    .RuDescription = element["rudescription"],
                    .AllowNulls = element["allownulls"],
                    .IfBooleanTrue = GetJsonOptionalString(element, "ifbooleantrue"),
                    .IfBooleanFalse = GetJsonOptionalString(element, "ifbooleanfalse"),
                    .QType = element["qtyp"],
                    .IsDescr = element["is_descr"],
                    .Description = element["descr"],
                };
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kFieldsDescriptionFile.string()));
        }

        return result;
    }

    std::unordered_map<std::string, std::vector<FieldRepresentationItem>> DatabaseData::LoadFieldsRepresentation()
    {
        std::unordered_map<std::string, std::vector<FieldRepresentationItem>> result;

        std::optional<nlohmann::json> json = ReadJsonFile(kFieldsRepresentationFile, "представления полей");
        if (!json)
        {
            return result;
        }

        try
        {
            for (const auto& element : json->items())
            {
                std::vector<FieldRepresentationItem> v;
                for (const auto& item : element.value())
                {
                    std::string key;
                    if (item["key"].is_number())
                    {
                        uint32_t val = item["key"];
                        key = std::to_string(val);
                    }
                    else
                    {
                        key = item["key"];
                    }

                    v.push_back({
                        .Key = key,
                        .RuShort = item["rushort"],
                        .Ru = item["ru"],
                    });
                }
                result[element.key()] = std::move(v);
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(
                Format("Ошибка во время чтения формата json: \n{}", kFieldsRepresentationFile.string()));
        }

        return result;
    }

    std::optional<ConstrExample> DatabaseData::LoadConstrExample(std::string_view _Constr)
    {
        std::filesystem::path path = kConstrExamplesFolder / std::format("{}.xlsx", _Constr);

        LOG_CORE_INFO("Loading file: {}", path.string());

        if (!std::filesystem::exists(path))
        {
            return std::nullopt;
        }

        xlnt::workbook wb;

        try
        {
            wb.load(path);
        }
        catch (const std::exception& e)
        {
            LOG_CORE_ERROR("Failed to load workbook: {}", e.what());
            return std::nullopt;
        }

        xlnt::worksheet ws = wb.active_sheet();

        ConstrExample result;
        for (const auto& col : ws.columns(true))
        {
            if (col.length() < 2 || !col[0].has_value())
            {
                continue;
            }

            std::vector<std::string> values;
            auto begin = col.begin();
            ++begin;
            for (const auto& cell : std::ranges::subrange(begin, col.end()))
            {
                values.push_back(cell.has_value() ? cell.to_string() : "");
            }
            result[col[0].to_string()] = values;
        }

        return result;
    }

}    // namespace LM
