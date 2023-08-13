#include "PythonCommand.h"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "Engine/Utils/ConsoleLog.h"

namespace LM
{

#ifdef _WIN32
    #define pclose _pclose
    #define popen  _popen
#endif

    PythonCommand::PythonCommand(std::string_view _Script) : m_Script(_Script) { }

    void PythonCommand::AddArg(std::string_view _Arg) { m_Args.emplace_back(_Arg); }

    void PythonCommand::AddArg(bool _Arg) { m_Args.emplace_back(_Arg ? "1" : "0"); }

    void PythonCommand::AddArg(int _Arg) { m_Args.emplace_back(std::to_string(_Arg)); }

    std::string PythonCommand::GetFullCommand() { return GetCommand() + " " + m_Script + " " + GetArgs(); }

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
            result += arg + " ";
        }
        return result;
    }

    void PythonCommand::Execute(std::function<void(const char*)> _LinePrintCallback)
    {
        std::array<char, 1024> buffer;

        std::string command = GetFullCommand();
        LOGI("Start python command: ", command);

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
