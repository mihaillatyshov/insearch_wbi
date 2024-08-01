#pragma once

#include "Engine/Core/Application.h"
#include "Engine/Core/Base.h"

extern LM::Application* LM::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    std::setlocale(LC_CTYPE, ".UTF8");
    std::cout << "Entry point start" << std::endl;

    auto App = LM::CreateApplication({ argc, argv });

    App->Run();

    delete App;

    return 0;
}
