#include "Table.h"

#include <format>
#include <iostream>

#include <imgui.h>

#include "Engine/Utils/ConsoleLog.h"

namespace LM
{

    const size_t kCellsCount = 3;

    void Table::DrawTable()
    {
        ImVec2 cursorBegin = ImGui::GetCursorPos();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "tttt");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "tttt");
        ImGui::SetCursorPos(cursorBegin);

        ImVec2 ts = ImGui::CalcTextSize("tttt tttt");
        if (ImGui::InvisibleButton("drag area", ImVec2(ts.x, ImGui::GetFrameHeight())))
        {
            LOGE("Clicked invisible button");
        }

        ImGui::Separator();

        static ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static char textBuf[kCellsCount][256] = { "hi1", "hi2 sasdadsdsaadsadssddsa", "hi3 dsdsad" };
        static char disabledBuff[1] = "";

        static bool setFocus = false;

        static int selectedCellId = -1;

        static float cellSize = 8.0f;
        static ImVec2 cellPadding { cellSize, cellSize };
        static ImVec2 itemSpacing { cellSize * 2.0f, cellSize * 2.0f };

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (selectedCellId > 0)
        {
            if (setFocus)
            {
                ImGui::SetKeyboardFocusHere();
                setFocus = false;
            }
            ImGui::InputText("##cell", textBuf[(selectedCellId - 1) % kCellsCount], 256, ImGuiInputTextFlags_None);
        }
        else
        {
            ImGui::BeginDisabled();
            ImGui::InputText("##cell", disabledBuff, 1);
            ImGui::EndDisabled();
        }

        // ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 4.0f });

        // ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 0.0f });
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

        if (ImGui::BeginTable("table", kCellsCount + 1, ImGuiTableFlags_SizingFixedFit | tableFlags))
        {
            ImGui::TableSetupColumn("##debug",
                                    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed |
                                        ImGuiTableColumnFlags_NoHide,
                                    0.0f, -1);

            for (int cell = 0; cell < kCellsCount; cell++)
            {
                ImGui::TableSetupColumn(std::format("{}", cell).c_str(),
                                        ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed |
                                            ImGuiTableColumnFlags_NoHide,
                                        0.0f, cell);
            }

            // ImGui::TableSetupScrollFreeze(0, 1);

            for (int row = 0; row < 115; row++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(0);

                ImGui::BeginDisabled();
                ImGui::SetNextItemWidth(1.0f);
                ImGui::InputText("##cell_test", disabledBuff, 1);
                ImGui::SameLine();
                ImGui::EndDisabled();
                ImGui::Text("%d", row);
                ImGui::PopID();

                for (int cell = 0; cell < kCellsCount; cell++)
                {
                    ImGui::TableNextColumn();
                    int cellId = row * kCellsCount + cell + 1;
                    ImGui::PushID(std::to_string(cellId).c_str());
                    ImGui::BeginGroup();
                    ImVec2 ts = ImGui::CalcTextSize(textBuf[cell]);
                    if (selectedCellId == cellId)
                    {
                        ImGui::SetNextItemWidth(ts.x + 20.0f);
                        // ImGui::GetContentRegionAvail()
                        ImGui::InputText("##cell", textBuf[cell], 256, ImGuiInputTextFlags_NoHorizontalScroll);
                    }
                    else
                    {
                        if (ImGui::Selectable(textBuf[cell % kCellsCount], false, ImGuiSelectableFlags_None,
                                              { ts.x + 20.0f, 0.0f }))
                        {
                            if (selectedCellId == cellId)
                            {
                                selectedCellId = -1;
                            }
                            else
                            {
                                selectedCellId = cellId;
                                setFocus = true;
                            }
                        }
                    }

                    ImGui::SameLine();
                    ImGui::Text("Test text");
                    ImGui::EndGroup();
                    ImGui::PopID();
                    if (ImGui::BeginPopupContextItem(
                            std::to_string(cellId).c_str()))    // <-- use last item id as popup id
                    {
                        ImGui::Text("This a popup for!");
                        if (ImGui::Button("Close"))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Right-click to open popup");
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::PopStyleVar(3);
        // ImGui::PopStyleColor();
    }

}    // namespace LM
