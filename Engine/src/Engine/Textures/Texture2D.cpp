#include "Texture2D.h"

#include "Platform/OpenGL/Textures/OGLTexture2D.h"

#include "Platform/OpenGL/Textures/OGLTexture2D.h"

namespace LM
{

    Ref<Texture2D> Texture2D::Create(std::string_view _FileName, Texture2D::MASK _Mask)
    {
        return CreateRef<OGLTexture2D>(_FileName, _Mask);
    }

    Texture2D::~Texture2D()
    { }

}    // namespace LM
