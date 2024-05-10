#pragma once

#include "IPageView.h"

namespace LM
{

    class IImgPageView : public IPageView
    {
    protected:
        std::string GetFileName() const override;

        void DrawWindowContent() override;
        float GetCircleRadius() const;

    protected:
        glm::vec2 m_ImgStart { 0.0f, 0.0f };
        glm::vec2 m_ImgEnd { 0.0f, 0.0f };
        void* m_ImgDrawList = nullptr;
    };

}    // namespace LM
