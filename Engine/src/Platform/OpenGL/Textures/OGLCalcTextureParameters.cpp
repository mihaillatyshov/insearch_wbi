#include "OGLCalcTextureParameters.h"

#include <GL/glew.h>

namespace LM
{

    int OGLCalcTextureParameters::InputType(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::NO_SRGB)
        {
            return GL_RGBA;
        }

        return GL_SRGB_ALPHA;
    }

    int OGLCalcTextureParameters::OutputType(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::NO_ALPHA)
        {
            return GL_RGB;
        }

        return GL_RGBA;
    }

    int OGLCalcTextureParameters::MagFilter(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::MAG_NEAREST)
        {
            return GL_NEAREST;
        }

        return GL_LINEAR;
    }

    int OGLCalcTextureParameters::MinFilter(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::MIN_NEAREST)
        {
            return GL_NEAREST;
        }
        if (_Mask & Texture2D::MASK::MIN_LINEAR_MIPMAP_LINEAR)
        {
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        if (_Mask & Texture2D::MASK::MIN_LINEAR_MIPMAP_NEAREST)
        {
            return GL_LINEAR_MIPMAP_NEAREST;
        }
        if (_Mask & Texture2D::MASK::MIN_NEAREST_MIPMAP_LINEAR)
        {
            return GL_NEAREST_MIPMAP_LINEAR;
        }
        if (_Mask & Texture2D::MASK::MIN_NEAREST_MIPMAP_NEAREST)
        {
            return GL_NEAREST_MIPMAP_NEAREST;
        }

        return GL_LINEAR;
    }

    int OGLCalcTextureParameters::WrapS(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::S_MIRRORED_REPEAT)
        {
            return GL_MIRRORED_REPEAT;
        }
        if (_Mask & Texture2D::MASK::S_CLAMP_TO_EDGE)
        {
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        if (_Mask & Texture2D::MASK::S_CLAMP_TO_BORDER)
        {
            return GL_LINEAR_MIPMAP_NEAREST;
        }

        return GL_REPEAT;
    }

    int OGLCalcTextureParameters::WrapT(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::T_MIRRORED_REPEAT)
        {
            return GL_MIRRORED_REPEAT;
        }
        if (_Mask & Texture2D::MASK::T_CLAMP_TO_EDGE)
        {
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        if (_Mask & Texture2D::MASK::T_CLAMP_TO_BORDER)
        {
            return GL_LINEAR_MIPMAP_NEAREST;
        }

        return GL_REPEAT;
    }

    int OGLCalcTextureParameters::WrapR(Texture2D::MASK _Mask)
    {
        if (_Mask & Texture2D::MASK::R_MIRRORED_REPEAT)
        {
            return GL_MIRRORED_REPEAT;
        }
        if (_Mask & Texture2D::MASK::R_CLAMP_TO_EDGE)
        {
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        if (_Mask & Texture2D::MASK::R_CLAMP_TO_BORDER)
        {
            return GL_LINEAR_MIPMAP_NEAREST;
        }

        return GL_REPEAT;
    }

    bool OGLCalcTextureParameters::HasMipmap(Texture2D::MASK _Mask)
    {
        return _Mask & Texture2D::MASK::MIN_LINEAR_MIPMAP_LINEAR ||
               _Mask & Texture2D::MASK::MIN_LINEAR_MIPMAP_NEAREST ||
               _Mask & Texture2D::MASK::MIN_NEAREST_MIPMAP_LINEAR ||
               _Mask & Texture2D::MASK::MIN_NEAREST_MIPMAP_NEAREST;
    }

}    // namespace LM
