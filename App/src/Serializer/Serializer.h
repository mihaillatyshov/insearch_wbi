#pragma once

#include <concepts>
#include <iostream>
#include <stdexcept>
#include <tuple>

#include "Engine/Core/Base.h"
#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/json.hpp"

#include "Project/Project.h"

namespace LM
{

    namespace Concept
    {

        template <typename T>
        concept Integral = std::is_integral<T>::value;

        template <typename T>
        concept FloatingPoint = std::is_floating_point<T>::value;

        template <typename T>
        concept StringLike = std::is_convertible_v<T, std::string_view>;

        template <typename T>
        concept JsonSimpleType = Integral<T> || FloatingPoint<T> || StringLike<T>;

        template <typename T>
        concept Vec2 = std::is_same_v<T, glm::vec2>;

        template <typename T>
        concept Array = std::is_array_v<T>;

    }    // namespace Concept

    template <typename Class, typename T>
    struct PropertyImpl
    {
        constexpr PropertyImpl(T Class::*_Member, const char* _Name, bool _IsRequired = true)
            : Member { _Member },
              Name { _Name },
              IsRequired { _IsRequired }
        { }

        using Type = T;

        T Class::*Member;
        const char* Name;
        bool IsRequired = true;
    };

    template <typename Class, typename T>
    constexpr auto property(T Class::*_Member, const char* _Name, bool _IsRequired = true)
    {
        return PropertyImpl<Class, T> { _Member, _Name, _IsRequired };
    }

#define PROPERTY(CLASS, MEMBER)         property(&CLASS::MEMBER, #MEMBER)
#define PROPERTY_NOT_REQ(CLASS, MEMBER) property(&CLASS::MEMBER, #MEMBER, false)

    template <typename T, T... S, typename F>
    constexpr void for_sequence(std::integer_sequence<T, S...>, F&& _Func)
    {
        (static_cast<void>(_Func(std::integral_constant<T, S> {})), ...);
    }

    class Serializer
    {
    public:
        static nlohmann::json Serialize(Ref<Project> _Project)
        {
            return SerializerBase::ToJson<Serializer_0_0_1>(*_Project);
        }

        static bool DeSerialize(Ref<Project> _Project, nlohmann::json _Json)
        {
            try
            {
                SerializerBase::FromJson<Serializer_0_0_1>(*_Project, _Json);
            }
            catch (const std::invalid_argument& exeption)
            {
                LOGE("Can't find field: ", exeption.what());
                return false;
            }
            return true;
        }

    protected:
        class SerializerBase
        {
        public:
            template <typename SerializerPropertiesBank, typename T>
                requires Concept::JsonSimpleType<T>
            static nlohmann::json ToJson(const T& _Object)
            {
                return _Object;
            }

            template <typename SerializerPropertiesBank, typename T>
                requires Concept::Vec2<T>
            static nlohmann::json ToJson(const T& _Object)
            {
                nlohmann::json result;
                result["x"] = _Object.x;
                result["y"] = _Object.y;
                return result;
            }

            template <typename SerializerPropertiesBank, typename T>
            static nlohmann::json ToJson(const std::vector<T>& _Object)
            {
                nlohmann::json result = nlohmann::json::array();
                for (const auto& item : _Object)
                {
                    result.push_back(ToJson<SerializerPropertiesBank>(item));
                }
                return result;
            }

            template <typename SerializerPropertiesBank, typename T>
            static nlohmann::json ToJson(const T& _Object)
            {
                nlohmann::json result;

                constexpr auto properties = SerializerPropertiesBank().GetProperties<T>();
                constexpr auto nbProperties = std::tuple_size<decltype(properties)>::value;

                for_sequence(std::make_index_sequence<nbProperties> {}, [&](auto i) {
                    constexpr auto property = std::get<i>(properties);
                    result[property.Name] = ToJson<SerializerPropertiesBank>(_Object.*(property.Member));
                });

                return result;
            }

            template <typename SerializerPropertiesBank, typename T>
                requires Concept::JsonSimpleType<T>
            static void FromJson(T& _Object, const nlohmann::json& _Json)
            {
                _Object = _Json;
            }

            template <typename SerializerPropertiesBank, typename T>
                requires Concept::Vec2<T>
            static void FromJson(T& _Object, const nlohmann::json& _Json)
            {
                _Object = { _Json["x"], _Json["y"] };
            }

            template <typename SerializerPropertiesBank, typename T>
            static void FromJson(std::vector<T>& _Object, const nlohmann::json& _Json)
            {
                for (const auto& item : _Json)
                {
                    _Object.push_back(item);
                }
            }

            template <typename SerializerPropertiesBank, typename T>
            static void FromJson(T& _Object, const nlohmann::json& _Json)
            {
                constexpr auto properties = SerializerPropertiesBank().GetProperties<T>();
                constexpr auto nbProperties = std::tuple_size<decltype(properties)>::value;

                for_sequence(std::make_index_sequence<nbProperties> {}, [&](auto i) {
                    constexpr auto property = std::get<i>(properties);
                    using Type = typename decltype(property)::Type;
                    if (!_Json.contains(property.Name))
                    {
                        if (property.IsRequired)
                        {
                            throw std::invalid_argument(property.Name);
                        }

                        _Object.*(property.Member) = Type();
                        return;
                    }

                    FromJson<SerializerPropertiesBank>(_Object.*(property.Member), _Json[property.Name]);
                });
            }
        };

        class Serializer_0_0_1
        {
        public:
            template <typename Class>
            static constexpr auto GetProperties()
            {
                static_assert("Properties not specified!");
                return std::make_tuple<int>(1);
            }

            template <>
            static constexpr auto GetProperties<GenRawExcel>()
            {
                return std::make_tuple(                                        //
                    property(&GenRawExcel::UseCutPattern, "UseCutPattern"),    //
                    property(&GenRawExcel::IsGenerated, "IsGenerated"),        //
                    property(&GenRawExcel::NeedRebuild, "NeedRebuild"),        //
                    property(&GenRawExcel::Version, "Version"));
            }

            template <>
            static constexpr auto GetProperties<GenImgsByCutPattern>()
            {
                return std::make_tuple(                                            //
                    property(&GenImgsByCutPattern::IsGenerated, "IsGenerated"),    //
                    property(&GenImgsByCutPattern::NeedRebuild, "NeedRebuild"),    //
                    property(&GenImgsByCutPattern::Version, "Version"));
            }

            template <>
            static constexpr auto GetProperties<Project>()
            {
                return std::make_tuple(                                                                           //
                    property(&Project::m_Folder, "Folder"),                                                       //
                    property(&Project::m_Catalog, "Catalog"),                                                     //
                    property(&Project::m_GeneratedCatalogExcludePages, "GeneratedCatalogExcludePages", false),    //
                    property(&Project::m_GenImgsByCutPattern, "GenImgsByCutPattern"),                             //
                    property(&Project::m_GenRawExcel, "GenRawExcel"));
            }

            template <>
            static constexpr auto GetProperties<Catalog>()
            {
                return std::make_tuple(                                              //
                    property(&Catalog::BaseFileName, "BaseFileName"),                //
                    property(&Catalog::ImgQuality, "ImgQuality"),                    //
                    property(&Catalog::SplitPages, "SplitPages"),                    //
                    property(&Catalog::NeedRebuild, "NeedRebuild"),                  //
                    property(&Catalog::IsGenerated, "IsGenerated"),                  //
                    property(&Catalog::BotRightCutPattern, "BotRightCutPattern"),    //
                    property(&Catalog::TopLeftCutPattern, "TopLeftCutPattern"));
            }

            template <>
            static constexpr auto GetProperties<CatalogCutPattern>()
            {
                return std::make_tuple(                                    //
                    property(&CatalogCutPattern::PageId, "PageId"),        //
                    property(&CatalogCutPattern::PointMin, "PointMin"),    //
                    property(&CatalogCutPattern::PointMax, "PointMax"),    //
                    property(&CatalogCutPattern::CenterPoint, "CenterPoint"));
            }
        };
    };

}    // namespace LM
