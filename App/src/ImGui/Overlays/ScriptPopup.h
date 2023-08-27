#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

#include"Engine/Core/Base.h"

#include "Python/PythonCommand.h"

namespace LM
{

    struct ScriptPopupProps
    {
        std::string WindowName = "";
        std::function<void(void)> PopupDesc = nullptr;
    };

    struct ScriptOpenPopupProps : public ScriptPopupProps
    {
        PythonCommand Command;
    };

    class ScriptPopup
    {
    public:
        static Ref<ScriptPopup> Get()
        {
            static Ref<ScriptPopup> instance = Ref<ScriptPopup>(new ScriptPopup);
            return instance;
        }

        void OpenPopup(const ScriptOpenPopupProps& _Props);

        void Draw();

    protected:
        ScriptPopup() = default;

        void DrawScriptBuffer();

    protected:
        std::atomic_bool m_IsScriptRuning = false;
        std::mutex m_ScriptBufferMtx;
        std::string m_ScriptBuffer;

        bool m_NeedOpenPopup = false;

        ScriptPopupProps m_Props;
    };

}    // namespace LM
