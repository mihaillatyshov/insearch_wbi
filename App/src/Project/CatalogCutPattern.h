#pragma once

#include "glm/glm.hpp"

namespace LM
{

    struct CatalogCutPattern
    {
        int PageId = -1;
        glm::vec2 PointMin = glm::vec2(0.0f, 0.0f);
        glm::vec2 PointMax = glm::vec2(0.0f, 0.0f);
        glm::vec2 CenterPoint = glm::vec2(0.0f, 0.0f);

        bool IsExists() const { return PageId != -1; }
        void Fix();
    };
    
}
