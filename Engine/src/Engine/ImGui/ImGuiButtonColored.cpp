#include "ImGuiButtonColored.hpp"

namespace LM
{

    bool ImGuiButtonColored(const char* label, const ImVec4& color, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, color.w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x - 0.1f, color.y - 0.1f, color.z - 0.1f, color.w));

        bool result = ImGui::Button(label, size);

        ImGui::PopStyleColor(3);

        return result;
    }

}    // namespace LM
