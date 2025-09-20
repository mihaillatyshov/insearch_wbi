#include "GLFWWindow.h"

#include "Engine/Core/Application.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/WindowEvent.h"
#include "Engine/Utils/Log.hpp"
#include "GLFW/glfw3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <Windowsx.h>

namespace LM
{

    WNDPROC original_proc;

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

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // if (uMsg != 124 && uMsg != 125)
        // {
        //     LOG_CORE_WARN("WindowProc {}", uMsg);
        // }

        switch (uMsg)
        {
            case WM_NCCALCSIZE: {
                // LOG_CORE_WARN("NC CALC SIZE");
                if (wParam == TRUE && lParam != NULL)
                {
                    NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                    if (IsZoomed(hWnd) != 0)
                    {
                        pParams->rgrc[0].top += 12;
                        pParams->rgrc[0].right -= 12;
                        pParams->rgrc[0].bottom -= 12;
                        pParams->rgrc[0].left += 12;
                    }
                    else
                    {
                        pParams->rgrc[0].top += 1;
                        pParams->rgrc[0].right -= 2;
                        pParams->rgrc[0].bottom -= 2;
                        pParams->rgrc[0].left += 2;
                    }
                }
                return 0;
            }
            case WM_NCACTIVATE:
            case WM_NCPAINT: {
                LOG_CORE_WARN("NC PT");

                LRESULT res = 0;
                if (uMsg != WM_NCPAINT)
                {
                    res = CallWindowProc(original_proc, hWnd, uMsg, wParam, lParam);
                }

                HDC hdc = GetWindowDC(hWnd);
                RECT rect;
                GetWindowRect(hWnd, &rect);
                OffsetRect(&rect, -rect.left, -rect.top);

                HBRUSH brush = CreateSolidBrush(RGB(128, 128, 128));
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);

                ReleaseDC(hWnd, hdc);

                return res;
            }
            case WM_PAINT: {
                LOG_CORE_WARN("PT");
                {
                    HDC hdc = GetWindowDC(hWnd);
                    RECT rect;
                    GetWindowRect(hWnd, &rect);
                    OffsetRect(&rect, -rect.left, -rect.top);

                    HBRUSH brush = CreateSolidBrush(RGB(128, 128, 128));
                    FillRect(hdc, &rect, brush);
                    DeleteObject(brush);

                    ReleaseDC(hWnd, hdc);
                }

                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);

                    RECT rc;
                    GetClientRect(hWnd, &rc);

                    RECT rcTitle = rc;
                    rcTitle.bottom = static_cast<LONG>(Application::Get().GetMainMenuFrameHeight());
                    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 215));    // синий как в Win10
                    FillRect(hdc, &rcTitle, hBrush);
                    DeleteObject(hBrush);

                    // Текст заголовка
                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(255, 255, 255));
                    DrawText(hdc, "Based On Vega Engine", -1, &rcTitle, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

                    EndPaint(hWnd, &ps);
                }
                return 0;
            }
            case WM_NCHITTEST: {
                // LOG_CORE_WARN("HT");
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

                break;
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
        disableTitlebar(m_Window);
        glfwMaximizeWindow(m_Window);

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
