#pragma once

#include <functional>
#include <string>
#include <vector>

namespace LM
{

    class PythonCommand
    {
    public:
    public:
        struct ArgType
        {
            std::string Arg;
            std::string ArgName;
        };

    public:
        PythonCommand(std::string_view _Script);

        void AddArg(std::string_view _Arg, std::string_view _ArgName = "");
        void AddArg(bool _Arg, std::string_view _ArgName = "");
        void AddArg(float _Arg, std::string_view _ArgName = "");
        void AddArg(int _Arg, std::string_view _ArgName = "");

        void Execute(std::function<void(const char*)> _LinePrintCallback = nullptr);

    protected:
        std::string GetFullCommand();
        std::string GetCommand();
        std::string GetArgs();

    protected:
        std::string m_Script;
        std::vector<ArgType> m_Args;
    };

}    // namespace LM
