cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(Engine VERSION 1.0)

set(SOURCES     
    src/Engine/Core/Application.h                               src/Engine/Core/Application.cpp         
    src/Engine/Core/Window.h                                    src/Engine/Core/Window.cpp              
    src/Engine/Core/Base.h
    src/Engine/Core/Timestep.h
    src/Engine/Core/Inputs.h
    src/Engine/Core/Assert.h
    src/Engine/Core/KeyCodes.h          
    src/Engine/Core/EntryPoint.h          
    src/Engine/Core/MouseCodes.h
    
    src/Engine/ImGui/imgui_impl_glfw.h                          src/Engine/ImGui/imgui_impl_glfw.cpp
    src/Engine/ImGui/imgui_impl_opengl3.h                       src/Engine/ImGui/imgui_impl_opengl3.cpp
    src/Engine/ImGui/imgui_impl_opengl3_loader.h            

    src/Engine/Textures/Texture2D.h                             src/Engine/Textures/Texture2D.cpp
    src/Engine/Textures/TextureLoader.h                         src/Engine/Textures/TextureLoader.cpp
    
    src/Engine/Events/Event.h
    src/Engine/Events/KeyEvent.h
    src/Engine/Events/MouseEvent.h 
    src/Engine/Events/WindowEvent.h
    src/Engine/Events/EventDispatcher.h 
    
    src/Engine/Layers/Layer.h                                   src/Engine/Layers/Layer.cpp             
    src/Engine/Layers/LayerStack.h                              src/Engine/Layers/LayerStack.cpp        
    src/Engine/Layers/ImGuiLayer.h                              src/Engine/Layers/ImGuiLayer.cpp        

    # src/Engine/Utils/Timer.h          
    # src/Engine/Utils/DataLoading.h            
    src/Engine/Utils/ConsoleLog.h                               src/Engine/Utils/ConsoleLog.cpp         
    src/Engine/Utils/FileDialogs.h
    src/Engine/Utils/json.hpp
    src/Engine/Utils/utf8.h

    src/Platform/OpenGL/Core/GLFWWindow.h                       src/Platform/OpenGL/Core/GLFWWindow.cpp
    src/Platform/OpenGL/Core/GLFWInputs.cpp
    src/Platform/OpenGL/Textures/OGLTexture2D.h                 src/Platform/OpenGL/Textures/OGLTexture2D.cpp
    src/Platform/OpenGL/Textures/OGLCalcTextureParameters.h     src/Platform/OpenGL/Textures/OGLCalcTextureParameters.cpp
    src/Platform/Windows/Utils/WindowsFileDiologs.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_compile_definitions(GLEW_STATIC _LIB RES_FOLDER="${CMAKE_SOURCE_DIR}/")

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} glm glfw glew stb ImGui nfd)

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
    GIT_TAG b35641f4a3c62aa86a0b3c983d163bc0fe36026d
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/mihaillatyshov/glew.git
    GIT_TAG bd68b6e4b996d165451049637c1d66018f207b53
)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG fc8f4bb442b9540969f2f3f351c4960d91bca17a
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/mihaillatyshov/imgui.git
    GIT_TAG e1d4b0481c1147a750502e5425f712e7516a7203
)
FetchContent_MakeAvailable(ImGui)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/mihaillatyshov/stb.git
    GIT_TAG c21c3bd687736635033267bd836168103cce59dd
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    nativefiledialog
    GIT_REPOSITORY https://github.com/mihaillatyshov/nativefiledialog-extended.git
    GIT_TAG 75cbdf819785d9f94855987724e30a6ba0a87e29
)
FetchContent_MakeAvailable(nativefiledialog)

# add_subdirectory(tests)

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Engine")
    set_target_properties(glfw PROPERTIES FOLDER "Engine/deps")
    set_target_properties(uninstall PROPERTIES FOLDER "Engine/deps")
    set_target_properties(update_mappings PROPERTIES FOLDER "Engine/deps")
    set_target_properties(glew PROPERTIES FOLDER "Engine/deps")
    set_target_properties(ImGui PROPERTIES FOLDER "Engine/deps")
    set_target_properties(nfd PROPERTIES FOLDER "Engine/deps")
endif()
