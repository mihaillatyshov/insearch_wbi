#include "TextureManager.h"

#include "Engine/Utils/ConsoleLog.h"

#include <filesystem>

namespace LM
{

    static std::pair<std::string, std::string> SeparateFileName(const std::string_view _FileName)
    {
        std::filesystem::path filesystemFileName = std::filesystem::path(_FileName);
        //LOGW("filesystemFileName.parent_path: ", filesystemFileName.parent_path());
        //LOGW("filesystemFileName.filename: ", filesystemFileName.filename());

        return std::make_pair(filesystemFileName.parent_path().string(), filesystemFileName.filename().string());
    }

    Ref<Texture2D> TextureManager::AddOrReplace(std::string_view _FileName)
    {
        //LOGW("Add: ", _FileName);

        auto [folder, filename] = SeparateFileName(_FileName);

        Ref<Texture2D> newTexture = Texture2D::Create(_FileName);
        newTexture->OnAttach();
        m_Bank[folder][filename] = newTexture;
        return newTexture;
    }

    void TextureManager::RemoveFile(std::string_view _FileName)
    {
        auto [folder, filename] = SeparateFileName(_FileName);

        if (m_Bank.contains(folder) && m_Bank[folder].contains(filename))
        {
            m_Bank[folder][filename]->OnDetach();
            m_Bank[folder].erase(filename);
        }
    }

    void TextureManager::RemoveFolder(std::string_view _Folder)
    {
        std::filesystem::path filesystemFolder = std::filesystem::path(_Folder);
        std::string folder = filesystemFolder.string();

        if (m_Bank.contains(folder))
        {
            for (const auto& [_, texture] : m_Bank[folder])
            {
                texture->OnDetach();
            }
            m_Bank.erase(folder);
        }
    }

    Ref<Texture2D> TextureManager::Get(std::string_view _FileName)
    {
        //LOGW("Get: ", _FileName);

        auto [folder, filename] = SeparateFileName(_FileName);

        return m_Bank[folder][filename];
    }

    bool TextureManager::Contains(std::string_view _FileName)
    {
        auto [folder, filename] = SeparateFileName(_FileName);

        if (!m_Bank.contains(folder))
        {
            return false;
        }

        return m_Bank[folder].contains(filename);
    }

    void TextureManager::RemoveAll()
    {
        for (const auto& [_, folder] : m_Bank)
        {
            for (const auto& [_, texture] : folder)
            {
                texture->OnDetach();
            }
        }
        m_Bank.clear();
    }

}    // namespace LM
