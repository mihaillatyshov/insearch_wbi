#include "CatalogCutPattern.h"

namespace LM
{

    void CatalogCutPattern::Fix()
    {
        glm::vec2 fixedPointMin = glm::clamp(glm::min(PointMin, PointMax), 0.0f, 100.0f);
        glm::vec2 fixedPointMax = glm::clamp(glm::max(PointMin, PointMax), 0.0f, 100.0f);
        PointMin = fixedPointMin;
        PointMax = fixedPointMax;

        CenterPoint = glm::clamp(CenterPoint, 0.0f, 100.0f);
    }

}    // namespace LM
