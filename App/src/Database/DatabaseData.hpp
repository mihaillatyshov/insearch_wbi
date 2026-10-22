#pragma once

#include "ImGui/Constructions/SelectConstructionFromTree.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace LM
{

    struct FieldDescription
    {
        std::string Type;
        std::optional<std::string> UnitRu;
        std::string RuDescription;
        bool AllowNulls;
        std::optional<std::string> IfBooleanTrue;
        std::optional<std::string> IfBooleanFalse;
        int QType;
        bool IsDescr;
        std::string Description;
    };

    struct FieldRepresentationItem
    {
        std::string Key;
        std::string RuShort;
        std::string Ru;
    };

    // Ключ - имя поля, значение - примеры значений поля
    using ConstrExample = std::unordered_map<std::string, std::vector<std::string>>;

    // Данные, выгруженные из БД скриптом InsearchEditorData.py.
    // Каждый файл загружается при первом обращении к нему.
    // Reload() сбрасывает загруженные данные, ссылки на них после вызова становятся невалидными,
    // поэтому вызывать его нужно из главного потока вне отрисовки, которая держит эти ссылки.
    class DatabaseData
    {
    public:
        static inline const std::filesystem::path kConstructionsFolder = "./assets/constructions";

    private:
        static inline const std::filesystem::path kConstructionsListFile = kConstructionsFolder / "constructions_list.json";
        static inline const std::filesystem::path kConstructionsFieldsFile = kConstructionsFolder / "ctd_fields.json";
        static inline const std::filesystem::path kFieldsDescriptionFile = kConstructionsFolder / "gen_fields.json";
        static inline const std::filesystem::path kFieldsRepresentationFile = kConstructionsFolder / "repr_fields.json";
        static inline const std::filesystem::path kConstrExamplesFolder = kConstructionsFolder / "examples";

    public:
        static DatabaseData& Get()
        {
            static DatabaseData instance;
            return instance;
        }

        DatabaseData(const DatabaseData&) = delete;
        DatabaseData& operator=(const DatabaseData&) = delete;

        const std::vector<ConstructionTreeConstr>& GetConstructionsList();
        const std::unordered_map<std::string, std::vector<std::string>>& GetConstructionsFields();
        const std::unordered_map<std::string, FieldDescription>& GetFieldsDescription();
        const std::unordered_map<std::string, std::vector<FieldRepresentationItem>>& GetFieldsRepresentation();

        const std::vector<std::string>* FindConstructionFields(std::string_view _Constr);
        const FieldDescription* FindFieldDescription(std::string_view _FieldName);
        const std::vector<FieldRepresentationItem>* FindFieldRepresentation(std::string_view _FieldName);
        const ConstrExample* FindConstrExample(std::string_view _Constr);

        void Reload();

    private:
        DatabaseData() = default;

        static std::vector<ConstructionTreeConstr> LoadConstructionsList();
        static std::unordered_map<std::string, std::vector<std::string>> LoadConstructionsFields();
        static std::unordered_map<std::string, FieldDescription> LoadFieldsDescription();
        static std::unordered_map<std::string, std::vector<FieldRepresentationItem>> LoadFieldsRepresentation();
        static std::optional<ConstrExample> LoadConstrExample(std::string_view _Constr);

    private:
        std::optional<std::vector<ConstructionTreeConstr>> m_ConstructionsList;
        std::optional<std::unordered_map<std::string, std::vector<std::string>>> m_ConstructionsFields;
        std::optional<std::unordered_map<std::string, FieldDescription>> m_FieldsDescription;
        std::optional<std::unordered_map<std::string, std::vector<FieldRepresentationItem>>> m_FieldsRepresentation;
        // std::nullopt - файл примера для конструкции отсутствует или не прочитан
        std::unordered_map<std::string, std::optional<ConstrExample>> m_ConstrExamples;
    };

}    // namespace LM
