#include <Engine/Core/EntryPoint.h>

#include "Engine/Utils/utf8.h"

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
        std::cout << "Create Application start" << std::endl;

        ApplicationSpecification spec;
        spec.Name = "InSearch Editor";
        spec.CommandLineArgs = args;
        spec.WorkingDirectory = U8_RES(RES_FOLDER);


        return new App(spec);
    }

}    // namespace LM
