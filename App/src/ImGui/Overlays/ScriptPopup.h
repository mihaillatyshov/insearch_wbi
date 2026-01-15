#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

#include "Engine/Core/Base.h"

#include "Python/PythonCommand.h"

namespace LM
{

    struct ScriptPopupProps
    {
        std::string WindowName = "";
        std::function<void(void)> PopupDesc = nullptr;
        std::function<void(int)> EndCallback = nullptr;
    };

    class ScriptPopup
    {
    public:
        static Ref<ScriptPopup> Get()
        {
            static Ref<ScriptPopup> instance = Ref<ScriptPopup>(new ScriptPopup);
            return instance;
        }

        void OpenPopup(const PythonCommand& _Command, const ScriptPopupProps& _Props);

        void Draw();

    protected:
        ScriptPopup() = default;

        void DrawScriptBuffer();

    protected:
        std::atomic_bool m_IsScriptRuning = false;
        std::atomic_int32_t m_ScritpReturnCode = 0;
        std::mutex m_ScriptBufferMtx;
        std::string m_ScriptBuffer;

        bool m_NeedOpenPopup = false;

        std::queue<std::pair<PythonCommand, ScriptPopupProps>> m_PendingPopups;
    };

}    // namespace LM
