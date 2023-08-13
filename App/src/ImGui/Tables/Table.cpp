#include "Table.h"

#include <iostream>

#include <imgui.h>

namespace LM
{

    void Table::DrawTable()
    {
        static ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static char textBuf[3][256] = { "hi1", "hi2 sasdadsdsaadsadssddsa", "hi3 dsdsad" };

        static bool setFocus = false;

        static int selectedCellId = -1;

        static float cellSize = 8.0f;
        static ImVec2 cellPadding { cellSize, cellSize };
        static ImVec2 itemSpacing { cellSize * 2.0f, cellSize * 2.0f };
        
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (selectedCellId >= 0)
        {
            if (setFocus)
            {
                ImGui::SetKeyboardFocusHere();
                setFocus = false;
            }
            ImGui::InputText("##cell", textBuf[selectedCellId % 3], 256, ImGuiInputTextFlags_None);
        }
        else
        {
            ImGui::BeginDisabled();
            static char disabledBuff[1] = "";
            ImGui::InputText("##cell", disabledBuff, 1);
            ImGui::EndDisabled();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

        if (ImGui::BeginTable("table", 3, ImGuiTableFlags_SizingFixedFit | tableFlags))
        {
            // ImGui::TableSetupColumn("ID",
            //                         ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed |
            //                             ImGuiTableColumnFlags_NoHide,
            //                         0.0f, MyItemColumnID_ID);
            // ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            // ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f,
            //                         MyItemColumnID_Action);
            // ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_PreferSortDescending, 0.0f,
            //                         MyItemColumnID_Quantity);
            // ImGui::TableSetupColumn("Description",
            //                         (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch,
            //                         0.0f, MyItemColumnID_Description);
            // ImGui::TableSetupColumn("Hidden", ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort);
            // ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

            for (int cell = 0; cell < 3 * 115; cell++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(cell);
                if (ImGui::Selectable(textBuf[cell % 3], selectedCellId == cell, ImGuiSelectableFlags_None))
                {
                    if (selectedCellId == cell)
                    {
                        selectedCellId = -1;
                    }
                    else
                    {
                        selectedCellId = cell;
                        setFocus = true;
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

}    // namespace LM
