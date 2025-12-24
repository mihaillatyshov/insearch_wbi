#include "PythonCommand.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "Engine/Utils/Log.hpp"

namespace LM
{

#ifdef _WIN32
    #define pclose _pclose
    #define popen  _popen
#endif

    PythonCommand::PythonCommand(std::string_view _Script) : m_Script(_Script) { }

    void PythonCommand::AddArg(std::string_view _Arg, std::string_view _ArgName)
    {
        m_Args.emplace_back(ArgType { .Arg = _Arg.data(), .ArgName = _ArgName.data() });
    }

    void PythonCommand::AddPathArg(const std::filesystem::path& _Arg, std::string_view _ArgName)
    {
        m_Args.emplace_back(ArgType { .Arg = _Arg.string().c_str(), .ArgName = _ArgName.data() });
    }

    void PythonCommand::AddArg(bool _Arg, std::string_view _ArgName)
    {
        m_Args.emplace_back(ArgType { .Arg = _Arg ? "1" : "0", .ArgName = _ArgName.data() });
    }

    void PythonCommand::AddArg(float _Arg, std::string_view _ArgName)
    {
        m_Args.emplace_back(ArgType { .Arg = std::to_string(_Arg), .ArgName = _ArgName.data() });
    }

    void PythonCommand::AddArg(int _Arg, std::string_view _ArgName)
    {
        m_Args.emplace_back(ArgType { .Arg = std::to_string(_Arg), .ArgName = _ArgName.data() });
    }

    std::string PythonCommand::GetFullCommand() { return GetCommand() + ' ' + m_Script + ' ' + GetArgs(); }

    std::string PythonCommand::GetCommand()
    {
#ifdef _WIN32
        return "python";
#else
        return "python3";
#endif
    }

    std::string PythonCommand::GetArgs()
    {
        std::string result;
        for (const auto& arg : m_Args)
        {
            result += arg.ArgName + ' ' + '"' + arg.Arg + '"' + ' ';
        }
        return result;
    }

    void PythonCommand::Execute(std::function<void(const char*)> _LinePrintCallback)
    {
        std::array<char, 1024> buffer;

        std::string command = GetFullCommand();
        LOG_CORE_INFO("Start python command: {}", command);

        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            std::cout << "[Python]: " << buffer.data();
            if (_LinePrintCallback)
            {
                _LinePrintCallback(buffer.data());
            }
        }
    }

}    // namespace LM
