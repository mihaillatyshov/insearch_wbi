#pragma once

#include <string>

#include "Engine/Core/Base.h"

namespace LM
{

    class Texture2D
    {

    public:
        enum MASK : uint32_t
        {
            NONE = 0,

            SRGB = 0,
            NO_SRGB = BIT(0),

            ALPHA = 0,
            NO_ALPHA = BIT(1),

            MAG_LINEAR = 0,
            MAG_NEAREST = BIT(2),

            S_REPEAT = 0,
            S_MIRRORED_REPEAT = BIT(3),
            S_CLAMP_TO_EDGE = BIT(4),
            S_CLAMP_TO_BORDER = BIT(5),

            T_REPEAT = 0,
            T_MIRRORED_REPEAT = BIT(6),
            T_CLAMP_TO_EDGE = BIT(7),
            T_CLAMP_TO_BORDER = BIT(8),

            R_REPEAT = 0,
            R_MIRRORED_REPEAT = BIT(9),
            R_CLAMP_TO_EDGE = BIT(10),
            R_CLAMP_TO_BORDER = BIT(11),

            REPEAT = S_REPEAT | T_REPEAT | R_REPEAT,
            MIRRORED_REPEAT = S_MIRRORED_REPEAT | T_MIRRORED_REPEAT | R_MIRRORED_REPEAT,
            CLAMP_TO_EDGE = S_CLAMP_TO_EDGE | T_CLAMP_TO_EDGE | R_CLAMP_TO_EDGE,
            CLAMP_TO_BORDER = S_CLAMP_TO_BORDER | T_CLAMP_TO_BORDER | R_CLAMP_TO_BORDER,

            MIN_LINEAR = 0,
            MIN_NEAREST = BIT(12),
            MIN_LINEAR_MIPMAP_LINEAR = BIT(13),
            MIN_LINEAR_MIPMAP_NEAREST = BIT(14),
            MIN_NEAREST_MIPMAP_LINEAR = BIT(15),
            MIN_NEAREST_MIPMAP_NEAREST = BIT(16),
        };

    public:
        virtual ~Texture2D();

        static Ref<Texture2D> Create(std::string_view _FileName, MASK _Mask = NONE);

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;

        virtual void* GetTextureId() const = 0;
        virtual float GetWidth() const = 0;
        virtual float GetHeight() const = 0;
    };

}    // namespace LM
