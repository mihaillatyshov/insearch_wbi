#include "Engine/Utils/FileDialogs.h"

#include "Engine/Core/Application.h"
#include "Engine/Utils/utf8.h"

#include <Windows.h>
#include <commdlg.h>

#include <codecvt>
#include <iostream>
#include <locale>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace LM
{

    constexpr size_t kFileSize = 1024;
    constexpr size_t kCurrentDirSize = 2048;

    static WCHAR currentDir[kCurrentDirSize] = { 0 };
    static bool IsFirstCustomCurrentDirectory() { return wcslen(currentDir) == 0; }

    static WCHAR* GetCustomCurrentDirectory()
    {
        if (wcslen(currentDir) == 0)
        {
            GetCurrentDirectoryW(kCurrentDirSize, currentDir);
        }
        return currentDir;
    }

    static const wchar_t* CreateFilterString(const FileDialogs::Filter& filter)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wideDescription = converter.from_bytes(filter.Description);
        std::wstring wideExtention = converter.from_bytes(filter.Extention);

        wchar_t* result = new wchar_t[wideDescription.size() + wideExtention.size() + 2] { '\0' };

        size_t cursor = 0;
        for (; cursor < wideDescription.size(); cursor++)
        {
            result[cursor] = wideDescription[cursor];
        }
        cursor++;

        for (size_t i = 0; i < wideExtention.size(); i++, cursor++)
        {
            result[cursor] = wideExtention[i];
        }
        return result;
    }

    std::string FileDialogs::OpenFile(const Filter& filter)
    {
        OPENFILENAMEW ofn;
        WCHAR szFile[kFileSize] = { 0 };
        WCHAR currentDir[kCurrentDirSize] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (IsFirstCustomCurrentDirectory())
        {
            ofn.lpstrInitialDir = GetCustomCurrentDirectory();
        }
        ofn.lpstrFilter = CreateFilterString(filter);
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        std::string result;
        if (GetOpenFileNameW(&ofn) == TRUE)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            result = converter.to_bytes(ofn.lpstrFile);
        }

        delete[] ofn.lpstrFilter;
        return result;
    }

    std::string FileDialogs::SaveFile(const Filter& filter)
    {
        OPENFILENAMEW ofn;
        WCHAR szFile[kFileSize] = { 0 };
        WCHAR currentDir[kCurrentDirSize] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (IsFirstCustomCurrentDirectory())
        {
            ofn.lpstrInitialDir = GetCustomCurrentDirectory();
        }
        ofn.lpstrFilter = CreateFilterString(filter);
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        // Sets the default extension by extracting it from the filter
        ofn.lpstrDefExt = wcschr(ofn.lpstrFilter, '\0') + 1;

        std::string result;
        if (GetSaveFileNameW(&ofn) == TRUE)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            result = converter.to_bytes(ofn.lpstrFile);
        }

        delete[] ofn.lpstrFilter;
        return result;
    }

}    // namespace LM
