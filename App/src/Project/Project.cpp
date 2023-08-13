#include "Project/Project.h"

#include <format>
#include <chrono>
#include <filesystem>

namespace LM
{

    Project::Project(std::string_view _FileName) { }

    Project::Project()
    {
        using namespace std::string_literals;

        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        //std::tm now_tm = *std::localtime(&now_c);
        //now_tm.
        m_AssetsPath =
            std::string(RES_FOLDER) + "assets/projects/"s + std::format("{:%Y_%m_%d__%H_%M_%OS}", time) + "/";
        std::filesystem::create_directories(m_AssetsPath);
    }

}    // namespace LM
