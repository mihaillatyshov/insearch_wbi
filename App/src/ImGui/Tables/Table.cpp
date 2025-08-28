#include "Table.h"

#include <format>
#include <iostream>
#include <string>

#include <imgui.h>
#include <xlnt/xlnt.hpp>

#include "Engine/Utils/Log.hpp"
#include "Engine/Utils/utf8.h"

namespace LM
{

    const size_t kCellsCount = 3;

    void Table::DrawTable()
    {
        /*ImVec2 cursorBegin = ImGui::GetCursorPos();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "tttt");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "tttt");
        ImGui::SetCursorPos(cursorBegin);

        ImVec2 ts = ImGui::CalcTextSize("tttt tttt");
        if (ImGui::InvisibleButton("drag area", ImVec2(ts.x, ImGui::GetFrameHeight())))
        {
            LOG_CORE_ERROR("Clicked invisible button");
        }*/

        if (ImGui::Button("Test Excel"))
        {
            xlnt::workbook wb;
            xlnt::worksheet ws = wb.active_sheet();
            ws.cell("A1").value(5);
            ws.cell("B2").value("string data");

            // if (xlnt::cell::type::number == ws.cell({ 1, 1 }).data_type())
            // {
            //     LOG_CORE_WARN("xlnt::cell::type::number == ws.cell({ 1, 1 }).data_type()");
            // }

            // if (xlnt::cell::type::shared_string == ws.cell({ 2, 2 }).data_type())
            // {
            //     LOG_CORE_WARN("xlnt::cell::type::shared_string == ws.cell({ 2, 2 }).data_type()");
            // }
            // if (xlnt::cell::type::inline_string == ws.cell({ 2, 2 }).data_type())
            // {
            //     LOG_CORE_WARN("xlnt::cell::type::inline_string == ws.cell({ 2, 2 }).data_type()");
            // }
            ws.freeze_panes({ 2, 2 });
            // wb.save("example.xlsx");
        }

        ImGui::Separator();

        static ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static char textBuf[kCellsCount][256] = { "hi1", "hi2 sasdadsdsaadsadssddsa", "hi3 dsdsad" };

        static bool setFocus = false;

        static int selectedCellId = -1;

        static float cellSize = 8.0f;
        static ImVec2 cellPadding { cellSize, cellSize };
        static ImVec2 itemSpacing { cellSize * 2.0f, cellSize * 2.0f };

        // ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 4.0f });

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

            ImGui::TableSetupScrollFreeze(1, 1);

            for (int row = 0; row < 155; row++)
            {
                ImGui::TableNextColumn();

                ImGui::PushID(0);
                ImGui::Text("%d", row);
                ImGui::PopID();

                for (int cell = 0; cell < kCellsCount; cell++)
                {
                    ImGui::TableNextColumn();
                    int cellId = row * kCellsCount + cell + 1;
                    ImGui::PushID(std::to_string(cellId).c_str());
                    ImGui::BeginGroup();
                    ImVec2 ts = ImGui::CalcTextSize(textBuf[cell]);
                    ImGui::SetNextItemWidth(ts.x + 20.0f);
                    if (ImGui::InputText("##cell", textBuf[cell], 256, ImGuiInputTextFlags_NoHorizontalScroll))
                    {
                        LOG_CORE_ERROR("I C");
                    }
                    if (ImGui::IsItemActivated())
                    {
                        LOG_CORE_ERROR("I A");
                    }
                    if (ImGui::IsItemDeactivated())
                    {
                        LOG_CORE_ERROR("I D");
                    }
                    if (ImGui::IsItemFocused())
                    {
                        LOG_CORE_ERROR("I F");
                    }

                    ImGui::EndGroup();
                    ImGui::PopID();

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Right-click to open popup");
                    }

                    // use last item id as popup id
                    if (ImGui::BeginPopupContextItem(std::to_string(cellId).c_str()))
                    {
                        ImGui::Text("This a popup for!");
                        if (ImGui::Button("Close"))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::PopStyleVar(3);
        // ImGui::PopStyleColor();
    }

}    // namespace LM
