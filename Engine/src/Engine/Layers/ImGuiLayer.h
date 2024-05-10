#pragma once

#include "Engine/Layers/Layer.h"

#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/WindowEvent.h"

#include <unordered_map>

struct ImFont;

namespace LM
{

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& e) override;

        void Begin();
        void End();

        void BlockEvents(bool block) { m_BlockEvents = block; }

        void SetDarkThemeColors();

        uint32_t GetActiveWidgetID() const;

        void ChangeFontSize(bool _NeedUpdateFontTexture);

        void SetFontSizeByMonitorScale(float _MonitorScale) { m_FontSize = static_cast<int>(14.0f * _MonitorScale); }

    private:
        bool m_BlockEvents = true;

        bool m_ChangeSize = false;
        int m_FontSize = 13;

        std::unordered_map<int, ImFont*> m_Fonts;
    };

}    // namespace LM
