#include <Engine/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace LM
{

    class App : public Application
    {
    public:
        App(const ApplicationSpecification& spec) : Application(spec) { PushLayer(new EditorLayer()); }
    };

    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.Name = "InSearch Editor";
        spec.CommandLineArgs = args;

        return new App(spec);
    }

}    // namespace LM
