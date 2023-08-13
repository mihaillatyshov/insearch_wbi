#pragma once

#include "Engine/Layers/Layer.h"

#include "Engine/Events/WindowEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

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

        void ChangeSize();
    private:
        bool m_BlockEvents = true;

        bool m_ChangeSize = true;
        int m_SizeId = 0;
    };

}    // namespace LM
