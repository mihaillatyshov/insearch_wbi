#pragma once

#include <functional>

#include "Engine/Core/Assert.h"
#include "Engine/Core/Base.h"
#include "Engine/Core/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvent.h"
#include "Engine/Layers/ImGuiLayer.h"
#include "Engine/Layers/LayerStack.h"

int main(int argc, char** argv);

namespace LM
{

    struct ApplicationCommandLineArgs
    {
        int Count = 0;
        char** Args = nullptr;

        const char* operator[](int index) const
        {
            CORE_ASSERT(index < Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string Name = "LM Application";
        std::string WorkingDirectory;
        ApplicationCommandLineArgs CommandLineArgs;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& specification);
        virtual ~Application();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        Window& GetWindow() { return *m_Window; }

        void Close();

        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

        static Application& Get() { return *s_Instance; }

        const ApplicationSpecification& GetSpecification() const { return m_Specification; }

    private:
        void Run();
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        ApplicationSpecification m_Specification;
        Ref<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

    private:
        static Application* s_Instance;
        friend int ::main(int argc, char** argv);
    };

    // To be defined in CLIENT
    Application* CreateApplication(ApplicationCommandLineArgs args);

}    // namespace LM
