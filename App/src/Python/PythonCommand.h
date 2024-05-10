#pragma once

#include <functional>
#include <string>
#include <vector>

namespace LM
{

    class PythonCommand
    {
    public:
        PythonCommand(std::string_view _Script);

        void AddArg(std::string_view _Arg);
        void AddArg(bool _Arg);
        void AddArg(float _Arg);
        void AddArg(int _Arg);

        void Execute(std::function<void(const char*)> _LinePrintCallback = nullptr);

    protected:
        std::string GetFullCommand();
        std::string GetCommand();
        std::string GetArgs();

    protected:
        std::string m_Script;
        std::vector<std::string> m_Args;
    };

}    // namespace LM
