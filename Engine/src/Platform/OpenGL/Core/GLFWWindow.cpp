#include "GLFWWindow.h"

#include "Engine/Core/Application.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/WindowEvent.h"
#include "Engine/Utils/Log.hpp"
#include "GLFW/glfw3.h"
#include <cmath>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <Windowsx.h>
// #include <dwmapi.h>
// #pragma comment(lib, "dwmapi.lib")

namespace LM
{

    WNDPROC original_proc;
    float g_MonitorScale = 1.0f;

    bool IsWindowMaximizedAlt(HWND hWnd)
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(hWnd, &wp))
        {
            return wp.showCmd == SW_MAXIMIZE;
        }
        return false;
    }

    glm::ivec4 calcOffset(HWND hWnd)
    {
        if (IsZoomed(hWnd) != 0)
        {
            LONG offset = std::lround(8.0f * g_MonitorScale);
            return { offset, offset, offset, offset };
        }
        else
        {
            LONG offset = std::lround(1.0f * g_MonitorScale);
            return { 1, offset, offset, offset };
        }
    }

    void DrawBorder(HWND hWnd)
    {
        HDC hdc = GetWindowDC(hWnd);
        RECT rect;
        GetWindowRect(hWnd, &rect);
        OffsetRect(&rect, -rect.left, -rect.top);

        HPEN hPen = CreatePen(PS_SOLID, std::lround(2.0f * g_MonitorScale), RGB(64, 64, 64));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(hPen);

        ReleaseDC(hWnd, hdc);
    }

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // if (uMsg != 124 && uMsg != 125)
        // {
        //     LOG_CORE_WARN("WindowProc {}", uMsg);
        // }

        switch (uMsg)
        {
            case WM_NCCALCSIZE: {
                if (wParam == TRUE && lParam != NULL)
                {
                    NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                    glm::ivec4 offset = calcOffset(hWnd);
                    pParams->rgrc[0].top += offset[0];
                    pParams->rgrc[0].right -= offset[1];
                    pParams->rgrc[0].bottom -= offset[2];
                    pParams->rgrc[0].left += offset[3];
                }
                return 0;
            }
            case WM_NCACTIVATE: return 1;
            // case WM_ACTIVATE: {
            //     LRESULT res = DefWindowProc(hWnd, uMsg, wParam, lParam);
            //     // LONG res = CallWindowProc(original_proc, hWnd, uMsg, wParam, lParam);
            //     DrawBorder(hWnd);
            //     return res;
            // }
            case WM_NCPAINT: {
                LOG_CORE_WARN("NC PT");
                DrawBorder(hWnd);
                return 0;
            }
            case WM_PAINT: {

                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, nullptr);

                DrawBorder(hWnd);
                return 0;
            }
            case WM_NCHITTEST: {
                const int borderWidth = 8;

                POINT mousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

                RECT windowRect;
                GetWindowRect(hWnd, &windowRect);

                if (mousePos.y >= windowRect.bottom - borderWidth)
                {
                    if (mousePos.x <= windowRect.left + borderWidth)
                    {
                        return HTBOTTOMLEFT;
                    }
                    else if (mousePos.x >= windowRect.right - borderWidth)
                    {
                        return HTBOTTOMRIGHT;
                    }
                    else
                    {
                        return HTBOTTOM;
                    }
                }
                else if (mousePos.y <= windowRect.top + borderWidth)
                {
                    if (mousePos.x <= windowRect.left + borderWidth)
                    {
                        return HTTOPLEFT;
                    }
                    else if (mousePos.x >= windowRect.right - borderWidth)
                    {
                        return HTTOPRIGHT;
                    }
                    else
                    {
                        return HTTOP;
                    }
                }
                else if (mousePos.x <= windowRect.left + borderWidth)
                {
                    return HTLEFT;
                }
                else if (mousePos.x >= windowRect.right - borderWidth)
                {
                    return HTRIGHT;
                }

                ScreenToClient(hWnd, &mousePos);
                if (mousePos.y < Application::Get().GetMainMenuFrameHeight() &&
                    !Application::Get().GetIsMainMenuAnyItemHovered())
                {
                    return HTCAPTION;
                }

                return HTCLIENT;
            }
        }

        // return DefWindowProc(hWnd, uMsg, wParam, lParam);
        return CallWindowProc(original_proc, hWnd, uMsg, wParam, lParam);
    }
    void disableTitlebar(GLFWwindow* window)
    {
        HWND hWnd = glfwGetWin32Window(window);

        LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
        lStyle |= WS_THICKFRAME;
        // lStyle &= ~WS_CAPTION;
        // lStyle &= ~WS_BORDER;
        // lStyle &= ~WS_DLGFRAME;
        SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);
        // MARGINS margins = {0, 0, 0, 0};
        // DwmExtendFrameIntoClientArea(hWnd, &margins);

        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        int width = windowRect.right - windowRect.left;
        int height = windowRect.bottom - windowRect.top;

        original_proc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
        original_proc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
        SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
    }

    bool glfw_get_window_monitor(GLFWmonitor** monitor, GLFWwindow* window)
    {
        bool success = false;

        int window_rectangle[4] = { 0 };
        glfwGetWindowPos(window, &window_rectangle[0], &window_rectangle[1]);
        glfwGetWindowSize(window, &window_rectangle[2], &window_rectangle[3]);

        int monitors_size = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

        GLFWmonitor* closest_monitor = NULL;
        int max_overlap_area = 0;

        for (int i = 0; i < monitors_size; ++i)
        {
            int monitor_position[2] = { 0 };
            glfwGetMonitorPos(monitors[i], &monitor_position[0], &monitor_position[1]);

            const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);
            if (monitor_video_mode == nullptr)
            {
                continue;
            }

            int monitor_rectangle[4] = {
                monitor_position[0],
                monitor_position[1],
                monitor_video_mode->width,
                monitor_video_mode->height,
            };

            if (!(((window_rectangle[0] + window_rectangle[2]) < monitor_rectangle[0]) ||
                  (window_rectangle[0] > (monitor_rectangle[0] + monitor_rectangle[2])) ||
                  ((window_rectangle[1] + window_rectangle[3]) < monitor_rectangle[1]) ||
                  (window_rectangle[1] > (monitor_rectangle[1] + monitor_rectangle[3]))))
            {
                int intersection_rectangle[4] = { 0 };

                // x, width
                if (window_rectangle[0] < monitor_rectangle[0])
                {
                    intersection_rectangle[0] = monitor_rectangle[0];

                    if ((window_rectangle[0] + window_rectangle[2]) < (monitor_rectangle[0] + monitor_rectangle[2]))
                    {
                        intersection_rectangle[2] =
                            (window_rectangle[0] + window_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = monitor_rectangle[2];
                    }
                }
                else
                {
                    intersection_rectangle[0] = window_rectangle[0];

                    if ((monitor_rectangle[0] + monitor_rectangle[2]) < (window_rectangle[0] + window_rectangle[2]))
                    {
                        intersection_rectangle[2] =
                            (monitor_rectangle[0] + monitor_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = window_rectangle[2];
                    }
                }

                // y, height
                if (window_rectangle[1] < monitor_rectangle[1])
                {
                    intersection_rectangle[1] = monitor_rectangle[1];

                    if ((window_rectangle[1] + window_rectangle[3]) < (monitor_rectangle[1] + monitor_rectangle[3]))
                    {
                        intersection_rectangle[3] =
                            (window_rectangle[1] + window_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = monitor_rectangle[3];
                    }
                }
                else
                {
                    intersection_rectangle[1] = window_rectangle[1];

                    if ((monitor_rectangle[1] + monitor_rectangle[3]) < (window_rectangle[1] + window_rectangle[3]))
                    {
                        intersection_rectangle[3] =
                            (monitor_rectangle[1] + monitor_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = window_rectangle[3];
                    }
                }

                // int overlap_area = intersection_rectangle[3] * intersection_rectangle[4];
                int overlap_area = intersection_rectangle[2] * intersection_rectangle[3];

                if (overlap_area > max_overlap_area)
                {
                    closest_monitor = monitors[i];
                    max_overlap_area = overlap_area;
                }
            }
        }

        if (closest_monitor)
        {
            *monitor = closest_monitor;
            success = true;
        }

        // true: monitor contains the monitor the window is most on
        // false: monitor is unmodified
        return success;
    }

    GLFWWindow::GLFWWindow(const WindowProps& _Props) : m_Data(_Props) { Init(); }

    GLFWWindow::~GLFWWindow()
    {
        glfwDestroyWindow(m_Window);
        // glfwTerminate();

        // delete m_Window;
    }

    bool GLFWWindow::IsWindowMaximized() const
    {
        int maximized = glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
        return maximized == GLFW_TRUE;
    }

    void GLFWWindow::Maximize() { glfwMaximizeWindow(m_Window); }

    void GLFWWindow::Minimize() { glfwIconifyWindow(m_Window); }

    void GLFWWindow::Restore() { glfwRestoreWindow(m_Window); }

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
            LOG_CORE_ERROR("Failed to initialize GLFW!");
            return false;
        }
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);

        if (!m_Window)
        {
            LOG_CORE_ERROR("Failed to create window!");
            return false;
        }

        GLFWmonitor* nowMonitor = NULL;
        if (!glfw_get_window_monitor(&nowMonitor, m_Window))
        {
            nowMonitor = glfwGetPrimaryMonitor();
        }
        glfwGetMonitorContentScale(nowMonitor, NULL, &m_Data.MonitorScale);
        g_MonitorScale = m_Data.MonitorScale;
        disableTitlebar(m_Window);
        glfwMaximizeWindow(m_Window);

        LOG_CORE_INFO("Current monitor: {} scale: {}", static_cast<void*>(nowMonitor), m_Data.MonitorScale);

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

        glfwSwapInterval(1);

        LOG_CORE_INFO("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        LOG_CORE_INFO("OpenGL vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));

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
        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xpos, int ypos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            GLFWmonitor* nowMonitor = NULL;
            if (!glfw_get_window_monitor(&nowMonitor, window))
            {
                nowMonitor = glfwGetPrimaryMonitor();
            }
            float newScale = 1.0f;
            glfwGetMonitorContentScale(nowMonitor, NULL, &newScale);

            if (newScale != data.MonitorScale)
            {
                data.MonitorScale = newScale;
                g_MonitorScale = newScale;

                WindowMonitorScaleChangedEvent event(newScale);
                data.EventCallback(event);
            }
        });

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event(width, height);
            data.EventCallback(event);

            GLFWmonitor* nowMonitor = NULL;
            if (!glfw_get_window_monitor(&nowMonitor, window))
            {
                nowMonitor = glfwGetPrimaryMonitor();
            }
            float newScale = 1.0f;
            glfwGetMonitorContentScale(nowMonitor, NULL, &newScale);

            if (newScale != data.MonitorScale)
            {
                data.MonitorScale = newScale;
                g_MonitorScale = newScale;

                WindowMonitorScaleChangedEvent event(newScale);
                data.EventCallback(event);
            }
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
