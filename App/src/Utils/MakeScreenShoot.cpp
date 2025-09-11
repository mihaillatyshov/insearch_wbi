#include "MakeScreenShoot.hpp"

#include "Engine/Utils/Log.hpp"

#include <windows.h>

#include "gdiplus.h"

#pragma comment(lib, "Gdiplus.lib")

namespace LM
{

    HBITMAP CaptureScreenRegion(RECT rect)
    {
        HDC hScreen = GetDC(NULL);
        HDC hDC = CreateCompatibleDC(hScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, rect.right - rect.left, rect.bottom - rect.top);
        SelectObject(hDC, hBitmap);
        BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hScreen, rect.left, rect.top, SRCCOPY);
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);
        return hBitmap;
    }

    void SaveBitmap(HBITMAP hBitmap, const wchar_t* filename)
    {
        Gdiplus::Bitmap bitmap(hBitmap, NULL);
        CLSID clsid;
        CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);    // PNG
        bitmap.Save(filename, &clsid, NULL);
    }

    RECT g_rect;
    bool g_selecting = false;
    POINT g_start;

    LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_LBUTTONDOWN:
                g_selecting = true;
                g_start.x = GET_X_LPARAM(lParam);
                g_start.y = GET_Y_LPARAM(lParam);
                break;
            case WM_MOUSEMOVE:
                if (g_selecting)
                {
                    POINT current;
                    current.x = GET_X_LPARAM(lParam);
                    current.y = GET_Y_LPARAM(lParam);
                    g_rect.left = min(g_start.x, current.x);
                    g_rect.top = min(g_start.y, current.y);
                    g_rect.right = max(g_start.x, current.x);
                    g_rect.bottom = max(g_start.y, current.y);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            case WM_LBUTTONUP:
                g_selecting = false;
                PostQuitMessage(0);
                break;
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                if (g_selecting)
                {
                    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 255));
                    FrameRect(hdc, &g_rect, brush);
                    DeleteObject(brush);
                }
                EndPaint(hwnd, &ps);
            }
            break;
            case WM_DESTROY: PostQuitMessage(0); break;
            default: return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    RECT SelectScreenRegion()
    {
        const wchar_t CLASS_NAME[] = L"OverlayWindow";

        WNDCLASS wc = {};
        wc.lpfnWndProc = OverlayProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_CROSS);

        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, CLASS_NAME, L"Выберите область",
                                   WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL,
                                   NULL, GetModuleHandle(NULL), NULL);

        SetLayeredWindowAttributes(hwnd, 0, 128, LWA_ALPHA);
        ShowWindow(hwnd, SW_SHOW);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        DestroyWindow(hwnd);
        return g_rect;
    }

    void MakeScreenShoot()
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        // Выбор области экрана
        RECT region = SelectScreenRegion();

        // Захват выбранной области экрана
        HBITMAP hBitmap = CaptureScreenRegion(region);

        // Сохранение как PNG-файл
        SaveBitmap(hBitmap, L"screen_capture.png");

        // Освобождение ресурсов
        DeleteObject(hBitmap);
        Gdiplus::GdiplusShutdown(gdiplusToken);

        LOG_CORE_INFO("Image saved as screen_capture.png");
    }

}    // namespace LM
