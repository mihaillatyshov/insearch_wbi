#include "GLFWWindow.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/WindowEvent.h"
#include "Engine/Utils/ConsoleLog.h"

namespace LM
{

    GLFWWindow::GLFWWindow(const WindowProps& _Props) : m_Data(_Props) { Init(); }

    GLFWWindow::~GLFWWindow()
    {
        glfwDestroyWindow(m_Window);
        // glfwTerminate();

        // delete m_Window;
    }

    void GLFWWindow::OnUpdate()
    {
        glfwMakeContextCurrent(m_Window);
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    bool GLFWWindow::Init()
    {
        if (!glfwInit())
        {
            LOGE("Failed to initialize GLFW!");
            return false;
        }

        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);

        if (!m_Window)
        {
            LOGE("Failed to create window!");
            return false;
        }

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);

        SetCallbacks();
        // glfwSetErrorCallback(glfw_error_callback);
        // glfwSetFramebufferSizeCallback(m_Window, window_resize);
        // glfwSetWindowSizeCallback(m_Window, window_resize);
        // glfwSetKeyCallback(m_Window, key_callback);
        // glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
        // glfwSetCursorPosCallback(m_Window, cursor_position_callback);
        // glfwSetScrollCallback(m_Window, scroll_callback);
        // glfwSetCharCallback(m_Window, character_input);
        // glfwSetDropCallback(m_Window, droping_paths);

        // glfwSetErrorCallback
        // glfwSetFramebufferSizeCallback
        // glfwSetDropCallback

        glfwSwapInterval(0);

        LOGI("OpenGL version: ", glGetString(GL_VERSION));
        LOGI("OpenGL vendor: ", glGetString(GL_VENDOR));

        return true;
    }

    void GLFWWindow::SetCallbacks()
    {
        // glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* _Window, int _Width, int _Height)
        //{
        //	WindowData* Data = (WindowData*)glfwGetWindowUserPointer(_Window);
        //	Data->Width = _Width;
        //	Data->Height = _Height;
        //	Data->EQueue->Add(CreateRef<WindowResizeEvent>(_Width, _Height));
        // });

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, true);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            KeyTypedEvent event(keycode);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });
    }

}    // namespace LM
