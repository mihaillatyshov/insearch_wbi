#include "Window.h"

#if defined(OPENGL)
    #include "Platform/OpenGL/Core/GLFWWindow.h"
#endif

namespace LM
{

    Ref<Window> Window::Create(const WindowProps& _Props)
    {
#ifdef OPENGL
        return CreateRef<GLFWWindow>(_Props);
#endif
    }

}    // namespace LM
