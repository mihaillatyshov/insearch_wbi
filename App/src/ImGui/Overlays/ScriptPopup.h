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

    enum class ScriptPopupRuningState
    {
        kRunning,
        kWaitingForClose,
        kFinished,
    };

    struct ScriptPopupProps
    {
        std::string WindowName = "";
        std::function<void(void)> PopupDesc = nullptr;
        std::function<void(int)> EndCallback = nullptr;
        bool IsStartOnPrevFail = true;
    };

    class ScriptPopup
    {
    public:
        static Ref<ScriptPopup> Get()
        {
            static Ref<ScriptPopup> instance = Ref<ScriptPopup>(new ScriptPopup);
            return instance;
        }

        void AddToQueue(const PythonCommand& _Command, const ScriptPopupProps& _Props);

        void Draw();

    protected:
        ScriptPopup() = default;

        void DrawScriptBuffer();

        void TryStartNewScript();

    protected:
        std::atomic<ScriptPopupRuningState> m_ScriptRuningState = ScriptPopupRuningState::kFinished;
        std::atomic_int32_t m_ScritpReturnCode = 0;
        std::mutex m_ScriptBufferMtx;
        std::string m_ScriptBuffer;

        bool m_IsNeedOpenPopup = false;
        ScriptPopupProps m_LastScriptProps;

        std::queue<std::pair<PythonCommand, ScriptPopupProps>> m_ScriptsQueue;
    };

}    // namespace LM
