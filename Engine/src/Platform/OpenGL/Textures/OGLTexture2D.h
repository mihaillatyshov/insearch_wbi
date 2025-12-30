#pragma once

#include "Engine/Textures/Texture2D.h"

namespace LM
{

    class OGLTexture2D : public Texture2D
    {
    public:
        OGLTexture2D(std::string_view _FileName, Texture2D::MASK _Mask);
        virtual ~OGLTexture2D();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void* GetTextureId() const override
        {
            return reinterpret_cast<void*>(static_cast<uintptr_t>(m_TextureID));
        }

        virtual float GetWidth() const override { return static_cast<float>(m_Width); }
        virtual float GetHeight() const override { return static_cast<float>(m_Height); }

    protected:
        void LoadOnFileReadError();

    protected:
        std::string m_FileName;
        MASK m_Mask;
        uint32_t m_Width;
        uint32_t m_Height;

        uint32_t m_TextureID = 0;
    };

}    // namespace LM
