#pragma once

#include <cstdint>
#include <string>
#include <functional>

#include "Engine/Core/Base.h"
#include "Engine/Events/Event.h"

namespace LM
{
    struct WindowProps
    {
        std::string Title = "Engine";
        uint32_t Width = 1280u;
        uint32_t Height = 720u;
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void OnUpdate() = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

        virtual void* GetNativeWindow() const = 0;

        static Ref<Window> Create(const WindowProps& _Props = WindowProps());
    };

}    // namespace LM
