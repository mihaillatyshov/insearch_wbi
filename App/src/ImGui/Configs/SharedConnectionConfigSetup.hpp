#pragma once

#include <chrono>
#include <filesystem>

namespace LM
{

    struct SharedConnectionConfig
    {
        std::string Name;

        std::string SshHost;
        int SshPort = 22;
        std::string SshUser;
        std::string SshPassword;

        std::string DbHost;
        int DbPort = 5432;
        std::string DbUser;
        std::string DbPassword;

        std::string ServerImgsPath;
    };

    class SharedConnectionConfigSetup
    {
    private:
        static inline const std::filesystem::path kSharedConnectionConfigSetupPath =
            "./assets/configs/shared_connection_config.json";

    public:
        SharedConnectionConfigSetup();
        ~SharedConnectionConfigSetup();

        void OnImGuiRender();

        void SaveConfig();

    private:
        void LoadConfig();

    private:
        std::vector<SharedConnectionConfig> m_Configs;
        std::string m_CurrentConfigName;
        std::chrono::steady_clock::time_point m_LastChangeTime;
        bool m_HasChanges = false;
        static constexpr auto kSaveDebounceDelay = std::chrono::milliseconds(1000);
    };

}    // namespace LM
