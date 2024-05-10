#include "OGLTexture2D.h"

#include "Engine/Textures/TextureLoader.h"
#include "Platform/OpenGL/Textures/OGLCalcTextureParameters.h"

#include <GL/glew.h>

namespace LM
{

    OGLTexture2D::OGLTexture2D(std::string_view _FileName, Texture2D::MASK _Mask)
        : m_FileName(_FileName),
          m_Mask(_Mask),
          m_Width(0),
          m_Height(0)
    { }

    OGLTexture2D::~OGLTexture2D() { }

    void OGLTexture2D::OnAttach()
    {
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);

        TextureLoader textureData(m_FileName);
        if (!textureData.IsOk())
        {
            LoadOnFileReadError();
            return;
        }

        m_Width = textureData.GetWidth();
        m_Height = textureData.GetHeight();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OGLCalcTextureParameters::WrapS(m_Mask));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OGLCalcTextureParameters::WrapT(m_Mask));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OGLCalcTextureParameters::MinFilter(m_Mask));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OGLCalcTextureParameters::MagFilter(m_Mask));

        glTexImage2D(GL_TEXTURE_2D, 0, OGLCalcTextureParameters::InputType(m_Mask), m_Width, m_Height, 0,
                     OGLCalcTextureParameters::OutputType(m_Mask), GL_UNSIGNED_BYTE, textureData.GetData());

        if (OGLCalcTextureParameters::MagFilter(m_Mask) != OGLCalcTextureParameters::MinFilter(m_Mask))
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OGLTexture2D::OnDetach() { glDeleteTextures(1, &m_TextureID); }

    void OGLTexture2D::LoadOnFileReadError()
    {
        m_Width = 2;
        m_Height = 2;

        uint32_t data[] = { 0xffff00ff, 0xff000000, 0xff000000, 0xffff00ff };

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

}    // namespace LM
