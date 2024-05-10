#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Engine/Utils/ConsoleLog.h"

namespace LM
{

    TextureLoader::TextureLoader(std::string_view _FileName)
    {
        m_Data = stbi_load(_FileName.data(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);
        if (!m_Data)
        {
            LOGW("Can't load texture: ", _FileName);
        }
    }

    TextureLoader::~TextureLoader() { stbi_image_free(m_Data); }

}    // namespace LM
