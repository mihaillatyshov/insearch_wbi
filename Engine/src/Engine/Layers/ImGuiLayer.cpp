#include "ImGuiLayer.h"

#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/Inputs.h"
#include "Engine/Events/EventDispatcher.h"
#include "Engine/Utils/json.hpp"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define USE_CUSTOM_FONT true

namespace LM
{

    const std::string regFont = "assets/fonts/roboto/Roboto-Regular.ttf";
    const std::string settingsFile = "assets/settings/imgui.json";

    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") { }

    void ImGuiLayer::OnAttach()
    {

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

#if USE_CUSTOM_FONT
        ImFontConfig config;
        for (int i = 0; i < 9; ++i)
        {
            std::cout << std::string(RES_FOLDER + regFont) << std::endl;
            io.Fonts->AddFontFromFileTTF(std::string(RES_FOLDER + regFont).c_str(), 12.0f * (1.0f + 0.5 * i), &config, io.Fonts->GetGlyphRangesCyrillic());
        }

        std::ifstream infile(std::string(RES_FOLDER + settingsFile).c_str());
        m_SizeId = 4;
        if (infile.is_open())
        {
            nlohmann::json data = nlohmann::json::parse(infile);
            std::cout << data << std::endl;
            if (data.contains("size") && data["size"].is_number_unsigned())
            {
                m_SizeId = data["size"];
            }
        }
        m_SizeId = glm::clamp(m_SizeId, 0, io.Fonts->Fonts.size() - 1);
        io.FontDefault = io.Fonts->Fonts[m_SizeId];
#endif

        //ImFontGlyphRangesBuilder

        /*
        auto grb = ImFontAtlas::GetGlyphRangesCyrillic();
        ImFontAtlas::
        grb.AddText(u8"ąčęėį");
        for (int n = 0; n < 0xFFFF; n++)
        {
            if (grb.GetBit(n))
            {
                printf("Contains: U+%04X\n", n);
            }
        }
        */

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular
        // ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        SetDarkThemeColors();

        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);

        dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& event) {
            bool control = Input::IsKeyPressed(LM::Key::LeftControl) || Input::IsKeyPressed(LM::Key::RightControl);
            bool shift = Input::IsKeyPressed(LM::Key::LeftShift) || Input::IsKeyPressed(LM::Key::RightShift);
            {
                if (control && shift)
                {
                    if (event.GetKeyCode() == LM::Key::Minus)
                    {
                        --m_SizeId;
                        m_ChangeSize = true;
                    }
                    if (event.GetKeyCode() == LM::Key::Equal)
                    {
                        ++m_SizeId;
                        m_ChangeSize = true;
                    }
                }
                return false;
            }
        });

        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }

    void ImGuiLayer::Begin()
    {
        ChangeSize();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {

        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::SetDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4 { 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4 { 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4 { 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4 { 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4 { 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4 { 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4 { 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4 { 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4 { 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4 { 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4 { 0.15f, 0.1505f, 0.151f, 1.0f };
    }

    void ImGuiLayer::ChangeSize()
    {
#if USE_CUSTOM_FONT
        if (m_ChangeSize)
        {
            ImGuiIO& io = ImGui::GetIO();
            m_SizeId = glm::clamp(m_SizeId, 0, io.Fonts->Fonts.size() - 1);
            io.FontDefault = io.Fonts->Fonts[m_SizeId];
            m_ChangeSize = false;
        }
#endif
    }

    uint32_t ImGuiLayer::GetActiveWidgetID() const { return GImGui->ActiveId; }

}    // namespace LM
