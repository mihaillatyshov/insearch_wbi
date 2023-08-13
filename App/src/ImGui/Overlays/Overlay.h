#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace LM
{
    using namespace std::chrono_literals;

    class Overlay
    {
    protected:
        using ChronoTimePoint = std::chrono::time_point<std::chrono::system_clock>;
    public:
        Overlay();

        void Start(const std::string& _Text, std::chrono::seconds _Duration = 3s);
        void Start(std::function<void(void)> _ImGuiFunction, std::chrono::seconds _Duration = 3s);

        void Draw();

    protected:
        std::function<void(void)> m_ImGuiFunction;
        ChronoTimePoint m_EndTimePoint;
    };

}    // namespace LM
