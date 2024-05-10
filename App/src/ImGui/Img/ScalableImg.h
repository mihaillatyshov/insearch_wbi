#pragma once

#include <string>

namespace LM
{

    class ScalableImg
    {
    public:
        void Draw();

        void SetNew(std::string_view _FileName);
        void Clear();
    protected:
        std::string m_ImgFileName;
    };

}
