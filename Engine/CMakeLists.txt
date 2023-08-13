cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(Engine VERSION 1.0)

set(SOURCES     
    src/Engine/Core/Application.cpp         src/Engine/Core/Application.h
    src/Engine/Core/Window.cpp              src/Engine/Core/Window.h
    src/Engine/Core/Base.h
    src/Engine/Core/Timestep.h
    src/Engine/Core/Inputs.h
    src/Engine/Core/Assert.h
    src/Engine/Core/KeyCodes.h          
    src/Engine/Core/EntryPoint.h          
    src/Engine/Core/MouseCodes.h
    
    src/Engine/Events/Event.h
    src/Engine/Events/KeyEvent.h
    src/Engine/Events/MouseEvent.h 
    src/Engine/Events/WindowEvent.h
    src/Engine/Events/EventDispatcher.h 
    
    src/Engine/Layers/Layer.cpp             src/Engine/Layers/Layer.h
    src/Engine/Layers/LayerStack.cpp        src/Engine/Layers/LayerStack.h
    src/Engine/Layers/ImGuiLayer.cpp        src/Engine/Layers/ImGuiLayer.h

    # src/Engine/Utils/Timer.h
    # src/Engine/Utils/DataLoading.h
    src/Engine/Utils/ConsoleLog.cpp         src/Engine/Utils/ConsoleLog.h
    src/Engine/Utils/FileDialogs.h
    src/Engine/Utils/json.hpp
    src/Engine/Utils/utf8.h

    src/Platform/OpenGL/Core/GLFWWindow.cpp                 src/Platform/OpenGL/Core/GLFWWindow.h
    src/Platform/OpenGL/Core/GLFWInputs.cpp
    src/Platform/Windows/Utils/WindowsFileDiologs.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})


add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} glm glfw ImGui)

if(MSVC)
    target_link_libraries(${PROJECT_NAME} opengl32)
else()
    target_link_libraries(${PROJECT_NAME} GL)
endif()

option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG dd8a678a66f1967372e5a5e3deac41ebf65ee127
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG fc8f4bb442b9540969f2f3f351c4960d91bca17a
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/mihaillatyshov/imgui.git
    GIT_TAG 01785d8581c14156043325472a4150dae4aba8b2
)
FetchContent_MakeAvailable(ImGui)

# add_subdirectory(tests)

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Engine")
    set_target_properties(glfw PROPERTIES FOLDER "Engine/deps")
    set_target_properties(update_mappings PROPERTIES FOLDER "Engine/deps")
    set_target_properties(ImGui PROPERTIES FOLDER "Engine/deps")
endif()
