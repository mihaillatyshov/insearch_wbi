#include "MakeScreenshot.hpp"
#include <fstream>

#define UNICODE
#define _UNICODE

#include <string>
#include <windows.h>
#include <windowsx.h>

#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace LM
{

    namespace
    {
        HINSTANCE g_hInst;
        POINT g_ptStart = { 0, 0 };
        POINT g_ptEnd = { 0, 0 };
        bool g_selecting = false;
        std::wstring g_savePath = L"screenshot.png";
        bool g_IsSaveResultOk = false;
    }    // namespace

    std::wstring StringToWString(const std::string& s)
    {
        if (s.empty())
        {
            return std::wstring();
        }
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
        std::wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &result[0], size_needed);
        return result;
    }

    void SaveBitmap(HBITMAP hBmp, const std::wstring& path)
    {
        Gdiplus::Bitmap bmp(hBmp, nullptr);
        CLSID clsid;
        CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);    // PNG
        bmp.Save(path.c_str(), &clsid, nullptr);
    }

    void CaptureRect(const RECT& rc, const std::wstring& filename)
    {
        HDC hdcScreen = GetDC(nullptr);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, w, h);
        SelectObject(hdcMem, hbm);

        BitBlt(hdcMem, 0, 0, w, h, hdcScreen, rc.left, rc.top, SRCCOPY);

        SaveBitmap(hbm, filename);

        DeleteObject(hbm);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_LBUTTONDOWN: {
                g_selecting = true;
                g_ptStart.x = GET_X_LPARAM(lParam);
                g_ptStart.y = GET_Y_LPARAM(lParam);
                g_ptEnd = g_ptStart;
                SetCapture(hwnd);
                return 0;
            }
            case WM_MOUSEMOVE: {
                if (g_selecting)
                {
                    g_ptEnd.x = GET_X_LPARAM(lParam);
                    g_ptEnd.y = GET_Y_LPARAM(lParam);
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
                return 0;
            }
            case WM_LBUTTONUP: {
                if (g_selecting)
                {
                    g_selecting = false;
                    ReleaseCapture();

                    RECT rc;
                    rc.left = min(g_ptStart.x, g_ptEnd.x);
                    rc.top = min(g_ptStart.y, g_ptEnd.y);
                    rc.right = max(g_ptStart.x, g_ptEnd.x);
                    rc.bottom = max(g_ptStart.y, g_ptEnd.y);

                    DestroyWindow(hwnd);
                    PostQuitMessage(0);

                    CaptureRect(rc, g_savePath);
                    g_IsSaveResultOk = true;
                }
                return 0;
            }
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                RECT rcClient;
                GetClientRect(hwnd, &rcClient);

                HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(hdc, &rcClient, hBrush);
                DeleteObject(hBrush);

                if (g_selecting)
                {
                    RECT rcSel;
                    rcSel.left = min(g_ptStart.x, g_ptEnd.x);
                    rcSel.top = min(g_ptStart.y, g_ptEnd.y);
                    rcSel.right = max(g_ptStart.x, g_ptEnd.x);
                    rcSel.bottom = max(g_ptStart.y, g_ptEnd.y);

                    InvertRect(hdc, &rcSel);
                }

                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_KEYDOWN: {
                if (wParam == VK_ESCAPE)
                {
                    g_selecting = false;
                    DestroyWindow(hwnd);
                    PostQuitMessage(0);
                }
                return 0;
            }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void RunSnippingOverlay(const std::wstring& savePath)
    {
        g_savePath = savePath;

        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = g_hInst;
        wc.lpszClassName = L"SnipOverlay";
        wc.hCursor = LoadCursor(nullptr, IDC_CROSS);

        RegisterClass(&wc);

        int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, wc.lpszClassName, L"", WS_POPUP, x, y, w, h, nullptr,
                                   nullptr, g_hInst, nullptr);

        SetLayeredWindowAttributes(hwnd, 0, 64, LWA_ALPHA);
        ShowWindow(hwnd, SW_SHOW);

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool MakeScreenshot(const std::string& outputPath)
    {
        g_IsSaveResultOk = false;
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

        RunSnippingOverlay(StringToWString(outputPath));

        Gdiplus::GdiplusShutdown(gdiplusToken);

        return g_IsSaveResultOk;
    }

    bool MakeScreenshotFromClipboard(const std::string& _OutputPath)
    {
        g_IsSaveResultOk = false;
        if (!OpenClipboard(nullptr))
        {
            return false;
        }

        HBITMAP hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
        if (!hBitmap)
        {
            CloseClipboard();
            return false;
        }

        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Gdiplus::Ok)
        {
            CloseClipboard();
            return false;
        }

        {
            Gdiplus::Bitmap bmp(hBitmap, nullptr);

            CLSID clsid;
            {
                UINT num = 0, size = 0;
                Gdiplus::GetImageEncodersSize(&num, &size);
                if (size == 0)
                {
                    Gdiplus::GdiplusShutdown(gdiplusToken);
                    CloseClipboard();
                    return false;
                }
                auto* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
                GetImageEncoders(num, size, pImageCodecInfo);
                for (UINT j = 0; j < num; ++j)
                {
                    if (wcscmp(pImageCodecInfo[j].MimeType, L"image/png") == 0)
                    {
                        clsid = pImageCodecInfo[j].Clsid;
                        break;
                    }
                }
                free(pImageCodecInfo);
            }

            if (bmp.Save(StringToWString(_OutputPath).c_str(), &clsid, nullptr) != Gdiplus::Ok)
            {
                Gdiplus::GdiplusShutdown(gdiplusToken);
                CloseClipboard();
                return false;
            }
        }

        Gdiplus::GdiplusShutdown(gdiplusToken);
        CloseClipboard();

        return true;
    }

}    // namespace LM
