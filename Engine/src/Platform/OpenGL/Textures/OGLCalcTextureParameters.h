#pragma once

#include "Engine/Textures/Texture2D.h"

namespace LM
{

    class OGLCalcTextureParameters
    {
    public:
        static int InputType(Texture2D::MASK _Mask);
        static int OutputType(Texture2D::MASK _Mask);
        static int MagFilter(Texture2D::MASK _Mask);
        static int MinFilter(Texture2D::MASK _Mask);
        static int WrapS(Texture2D::MASK _Mask);
        static int WrapT(Texture2D::MASK _Mask);
        static int WrapR(Texture2D::MASK _Mask);

        static bool HasMipmap(Texture2D::MASK _Mask);
    };

}    // namespace LM
