#include "Project/Project.h"

#include <chrono>
#include <filesystem>
#include <format>

namespace LM
{

    Project::Project(std::string_view _FileName) : m_FileName(_FileName) { }

    Project::Project()
    {
        using namespace std::string_literals;

        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        // std::tm now_tm = *std::localtime(&now_c);
        // now_tm.
        m_AssetsPath =
            std::string(RES_FOLDER) + "assets/projects/"s + std::format("{:%Y_%m_%d__%H_%M_%OS}", time) + "/";
        std::filesystem::create_directories(m_AssetsPath);
    }

    std::string Project::GetRawImgPath() const
    {
        std::string imgPath = m_AssetsPath + "raw_img/";
        std::filesystem::create_directories(imgPath);
        return imgPath;
    }

}    // namespace LM
