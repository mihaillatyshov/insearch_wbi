#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Utils/json.hpp"

namespace LM
{

    class Project;

    class Serializer
    {
    public:
        static nlohmann::json Serialize(Ref<Project> _Project);
        static bool DeSerialize(Ref<Project> _Project, nlohmann::json _Json);

    protected:
    };

}    // namespace LM
