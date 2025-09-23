#include "XlsxPageView.hpp"

#include "Engine/Textures/Texture2D.h"
#include "Engine/Utils/Utf8Extras.hpp"
#include "ImGui/Overlays/Overlay.h"
#include "Managers/TextureManager.h"
#include "Utils/FileFormat.h"

#include "Engine/Utils/Log.hpp"
#include "Engine/Utils/json.hpp"
#include "Engine/Utils/utf8.h"
#include "Utils/MakeScreenshot.hpp"
#include "glm/common.hpp"
#include "glm/fwd.hpp"

#include <array>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <xlnt/xlnt.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
template <typename T>
struct adl_serializer<std::optional<T>>
{
    static void to_json(json& j, const std::optional<T>& opt)
    {
        if (opt == std::nullopt)
        {
            j = nullptr;
        }
        else
        {
            j = *opt;    // this will call adl_serializer<T>::to_json which will
                         // find the free function to_json in T's namespace!
        }
    }

    static void from_json(const json& j, std::optional<T>& opt)
    {
        if (j.is_null())
        {
            opt = std::nullopt;
        }
        else
        {
            opt = j.template get<T>();    // same as above, but with
                                          // adl_serializer<T>::from_json
        }
    }
};
NLOHMANN_JSON_NAMESPACE_END

bool CustomPassFilter(const ImGuiTextFilter& _TextFilter, std::string_view text)
{
    if (_TextFilter.Filters.Size == 0)
    {
        return true;
    }

    for (const ImGuiTextFilter::ImGuiTextRange& f : _TextFilter.Filters)
    {
        if (f.b == f.e)
        {
            continue;
        }

        std::string filterStr(f.b, f.e);
        std::string textStr(text);
        utf8upr(reinterpret_cast<utf8_int8_t*>(filterStr.data()));
        utf8upr(reinterpret_cast<utf8_int8_t*>(textStr.data()));
        // std::string filterStr = StrToLowerRu(std::string_view(f.b, f.e));
        // std::string textStr = StrToLowerRu(text);
        if (textStr.find(filterStr) != std::string::npos)
        {
            return true;
        }
    }

    // // Implicit * grep
    // if (CountGrep == 0)
    // {
    //     return true;
    // }

    return false;
}

namespace LM
{

    constexpr std::string_view kDefaultConstructionsTreeFile = "assets/constructions/constructions_tree.json";
    constexpr std::string_view kDefaultConstructionsFieldsFile = "assets/constructions/ctd_fields.json";
    constexpr std::string_view kDefaultFieldsDescriptionFile = "assets/constructions/gen_fields.json";
    constexpr std::string_view kDefaultRepresentationFieldsDescriptionFile = "assets/constructions/repr_fields.json";
    constexpr std::string_view kExtraInfoFile = "extra_info.json";

    const std::vector<std::string> kProductBaseFields = { "fulldescription", "lcs", "moq", "codem" };
    const std::vector<std::string> kImgFileTypeList = { "pic", "drw" };

    inline bool DeleteButton()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, 0xFF0000AA);
        bool result = ImGui::Button("X");
        ImGui::PopStyleColor();

        return result;
    }

    inline ImVec4 GetFrameBgColorNone(float _Alpha = 0.125f, float _Blue = 0.0f)
    {
        ImVec4 frameBgColorNone = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        frameBgColorNone.w = _Alpha;
        return { frameBgColorNone.x, frameBgColorNone.y, _Blue, _Alpha };
    }
    inline ImVec4 GetFrameBgColorOk(float _Alpha = 0.125f, float _Blue = 0.0f)
    {
        return ImVec4(0.0f, 1.0f, 0.0f, _Alpha);
    }
    inline ImVec4 GetFrameBgColorWarn(float _Alpha = 0.125f, float _Blue = 0.0f)
    {
        return ImVec4(1.0f, 1.0f, 0.0f, _Alpha);
    }
    inline ImVec4 GetFrameBgColorError(float _Alpha = 0.125f, float _Blue = 0.0f)
    {
        return ImVec4(1.0f, 0.0f, 0.0f, _Alpha);
    }

    XlsxPageView::XlsxPageView()
    {
        LoadConstructionsTree();
        LoadConstructionsFields();
        LoadFieldsDescription();
        LoadRepresentationFieldsDescription();
    }

    XlsxPageView::~XlsxPageView()
    {
        try
        {
            LOG_CORE_INFO("Start saving");
            SaveXLSX();
            LOG_CORE_INFO("End saving");
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("WARN: {}", e.what());
        }
    }

    bool XlsxPageView::OnPageWillBeChanged(int _CurrentPageId, int _NewPageId)
    {
        SaveXLSX();
        return true;
    }

    std::string XlsxPageView::GetFileName() const { return FileFormat::FormatXlsx(m_PageId); }

    void XlsxPageView::DrawWindowContent()
    {
        m_IsMainWindowFocused = ImGui::IsWindowFocused();
        if (m_PageId == -1)
        {
            return;
        }

        if (m_LoadedPageId != m_PageId)
        {
            LoadXLSX();
        }

        if (!m_IsExtraInfoJsonLoaded)
        {
            LoadExtraInfoJson();
            m_IsExtraInfoJsonLoaded = true;
        }

        if (m_TableData.empty() || m_LoadedPageId == -1 || m_LoadedPageId != m_PageId)
        {
            ImGui::Text("No data loaded or file not found.");
            return;
        }

        ImGui::Text("Имя файла: %s", m_LoadedPageFilename.string().c_str());

        if (m_IsAnyHeaderActive)
        {
            m_IsAnyCellActive = true;
        }

        HandleImGuiEvents();
        // DrawTableActions();

        size_t colsCount = 0;
        if (!m_TableData.empty())
        {
            colsCount = m_TableData[0].size();
        }

        DrawGlobalAddListWindow();

        DrawSimpleAddListWindow();
        DrawSimpleCalcListWindow();
        DrawImgsPerListWindow();

        DrawJoinModal();

        ImGui::Separator();
        if (m_SelectedCell.has_value())
        {
            ImGui::Text("Selected cell Col: %llu; Row: %llu", m_SelectedCell->x, m_SelectedCell->y);
        }
        else
        {
            ImGui::Text("No cell selected");
        }
        ImGui::Separator();

        static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
                                            ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;

        const float framePaddingX = 8.0f;
        const float framePaddingY = 4.0f;

        // TODO: fix with: add col width for header and for content for future select in setwidth
        std::vector<float> headerWidths(colsCount + 1, 0.0f);
        std::vector<float> columnWidths(colsCount + 1, 0.0f);
        headerWidths[0] =
            std::max(ImGui::CalcTextSize(std::to_string(m_TableData.size()).c_str()).x + framePaddingX * 2.0f,
                     ImGui::CalcTextSize("  . . .  ").x);

        if (m_TableData.size() > 0)
        {
            for (size_t colId = 0; colId < colsCount; ++colId)
            {
                const auto& cellText = m_TableData[0][colId].Value;
                ImVec2 cellTextSize = ImGui::CalcTextSize(std::format("  {}  ", cellText).c_str());
                headerWidths[colId + 1] = std::max(headerWidths[colId + 1], cellTextSize.x);
                // + ImGui::GetFontSize() * 2.0f
                if (std::optional<const std::reference_wrapper<std::string>> extraValue =
                        GetExtraListValue(m_TableData[0][colId].Value);
                    extraValue.has_value())
                {
                    std::string::difference_type textOffset =
                        std::min(extraValue->get().end() - extraValue->get().begin(), 64ll);
                    ImVec2 extraTextSize =
                        ImGui::CalcTextSize(extraValue->get().c_str(), extraValue->get().c_str() + textOffset);
                    columnWidths[colId + 1] = std::max(columnWidths[colId + 1], extraTextSize.x + framePaddingX * 2.0f);
                }
            }
        }

        for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
        {
            for (size_t colId = 0; colId < colsCount; ++colId)
            {
                const auto& cellText = m_TableData[rowId][colId].Value;
                float cellTextSize = ImGui::CalcTextSize(cellText.c_str()).x + ImGui::CalcTextSize(" ").x;
                columnWidths[colId + 1] = std::max(columnWidths[colId + 1], cellTextSize + framePaddingX * 2.0f);
                // + ImGui::GetFontSize() * 2.0f
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.0f, 0.0f });

        if (ImGui::BeginTable("XLSX Table", static_cast<int>(colsCount + 1), tableFlags))
        {
            ImGui::TableSetupScrollFreeze(1, 1);

            ImGui::PushStyleVarY(ImGuiStyleVar_CellPadding, framePaddingY);
            m_IsAnyHeaderActive = false;
            DrawTableHeaderReturn headerData = DrawTableHeader(colsCount);
            ImGui::PopStyleVar();

            m_IsAnyCellActive = false;
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
            {
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(rowId));
                ImGui::TableSetColumnIndex(static_cast<int>(0));

                bool isRowHovered = false;
                std::string rowIdStr = std::format("{}##RowId", rowId);

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { framePaddingX, framePaddingY });
                ImGui::Button(rowIdStr.c_str(), ImVec2(std::max(headerWidths[0], columnWidths[0]), 0.0f));
                ImGui::PopStyleVar();

                if (ImGui::IsItemHovered())
                {
                    isRowHovered = true;
                    // ImGui::SetTooltip("Right-click to open popup");
                }

                if (ImGui::IsItemFocused())
                {
                    UnSelectAll();
                    m_SelectedRow = rowId;
                }

                if (ImGui::BeginPopupContextItem(rowIdStr.c_str()))
                {
                    ImGui::Text("This a popup for Row:%zu", rowId);
                    if (ImGui::Button("Close"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                for (size_t colId = 0; colId < colsCount; ++colId)
                {
                    ImGui::PushID(static_cast<int>(colId));
                    ImGui::TableSetColumnIndex(static_cast<int>(colId + 1));

                    auto& t = m_TableData[rowId][colId].Value;
                    bool isColHovered = headerData.HoveredCol.has_value() && headerData.HoveredCol.value() == colId;

                    PushCellFrameBgColor(isRowHovered, isColHovered, rowId, colId);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { framePaddingX, framePaddingY });

                    float width = std::max(headerWidths[colId + 1], columnWidths[colId + 1]);

                    if (std::optional<const std::reference_wrapper<std::string>> extraValue =
                            GetExtraListValue(m_TableData[0][colId].Value);
                        extraValue.has_value())
                    {
                        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                        ImVec4 clipRect(cursorPos.x + framePaddingX, cursorPos.y + framePaddingY,
                                        cursorPos.x + width - framePaddingX,
                                        cursorPos.y + ImGui::GetFontSize() + framePaddingY);
                        const ImVec2 textPos = ImVec2(cursorPos.x + framePaddingX, cursorPos.y + framePaddingY);
                        draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textPos, 0xFF666666,
                                           extraValue->get().c_str(), NULL, 0.0f, &clipRect);
                    }

                    ImGui::SetNextItemWidth(width);
                    ImGui::InputText("##Input", &t, ImGuiInputTextFlags_NoHorizontalScroll);
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemActive())
                    {
                        m_IsAnyCellActive = true;
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        PushHistory();
                    }

                    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_F2))
                    {
                        ImGui::ActivateItemByID(ImGui::GetItemID());
                    }

                    if (ImGui::IsItemActive() || ImGui::IsItemFocused())
                    {
                        if (!m_SelectedCell.has_value() || (m_SelectedCell->x != colId) || (m_SelectedCell->y != rowId))
                        {
                            UnSelectAll();
                            m_SelectedCell = { colId, rowId };
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                    {
                        if (ImGui::GetIO().KeyShift && m_SelectedCell.has_value())
                        {
                            m_ExtraSelectedCell = { colId, rowId };
                        }
                        else
                        {
                            UnSelectAll();
                            m_SelectedCell = { colId, rowId };
                            ImGui::ClearActiveID();
                            ImGui::SetFocusID(ImGui::GetItemID(), ImGui::GetCurrentWindow());
                            ImGui::SetWindowFocus();
                        }
                        // ImGui::SetTooltip("Right-click to open popup");
                    }
                    if (ImGui::BeginPopupContextItem())
                    {
                        ImGui::Text("This a popup for Col: %zu Row:%zu", colId, rowId);
                        if (ImGui::Button("Close"))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();

        if (m_DeleteCol.has_value())
        {
            DeleteCol(*m_DeleteCol);
            m_DeleteCol = std::nullopt;
        }
        if (m_DeleteRow.has_value())
        {
            DeleteRow(*m_DeleteRow);
            m_DeleteRow = std::nullopt;
        }
    }

    void XlsxPageView::DrawTableActions()
    {
        if (ImGui::Button("Копировать без заголовка"))
        {
            std::string copyText;
            for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
            {
                for (size_t colId = 0; colId < m_TableData[rowId].size(); ++colId)
                {
                    copyText += m_TableData[rowId][colId].Value + "\t";
                }
                copyText += "\n";
            }
            ImGui::SetClipboardText(copyText.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Копировать с заголовком"))
        {
            std::string copyText;
            for (size_t colId = 0; colId < m_TableData[0].size(); ++colId)
            {
                copyText += m_TableData[0][colId].Value + "\t";
            }
            copyText += "\n";
            for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
            {
                for (size_t colId = 0; colId < m_TableData[rowId].size(); ++colId)
                {
                    copyText += m_TableData[rowId][colId].Value + "\t";
                }
                copyText += "\n";
            }
            ImGui::SetClipboardText(copyText.c_str());
        }

        // TODO: Move to separate function
        if (ImGui::Button("Вставить из буфера (заменить все)"))
        {
            ReplaceFromClipboard(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Вставить из буфера (пустая строка заголовка)"))
        {
            ReplaceFromClipboard(true);
        }

        // TODO: Move to separate function
        if (ImGui::Button("Test Fix"))
        {
            for (auto& row : m_TableData)
            {
                if (!row.empty())
                {
                    auto& cell = row[0].Value;
                    cell.erase(
                        std::remove_if(cell.begin(), cell.end(), [](unsigned char ch) { return std::isspace(ch); }),
                        cell.end());
                }
            }

            for (auto& row : m_TableData)
            {
                for (auto& cell : row)
                {
                    std::string& cellValue = cell.Value;

                    std::string::size_type pos;
                    while ((pos = cellValue.find(" ~")) != std::string::npos)
                    {
                        cellValue.replace(pos, 2, "~");
                    }
                    while ((pos = cellValue.find("~ ")) != std::string::npos)
                    {
                        cellValue.replace(pos, 2, "~");
                    }
                    cellValue.erase(
                        std::remove_if(cellValue.begin(), cellValue.end(), [](unsigned char ch) { return ch >= 0x80; }),
                        cellValue.end());
                    cellValue.erase(cellValue.begin(),
                                    std::find_if(cellValue.begin(), cellValue.end(),
                                                 [](unsigned char ch) { return !std::isspace(ch); }));
                    cellValue.erase(std::find_if(cellValue.rbegin(), cellValue.rend(),
                                                 [](unsigned char ch) { return !std::isspace(ch); })
                                        .base(),
                                    cellValue.end());
                    cellValue.erase(std::unique(cellValue.begin(), cellValue.end(),
                                                [](char a, char b) { return std::isspace(a) && std::isspace(b); }),
                                    cellValue.end());
                }
            }

            PushHistory();
        }

        ImGui::SameLine();

        if (ImGui::Button("Разделить столбцы по пробелу"))
        {
            SplitAndExpandTable();
        }

        ImGui::Separator();

        if (ImGui::Button("Изменить конструкцию"))
        {
            ImGui::OpenPopup("Изменить конструкцию");
        }
        if (ImGui::BeginPopup("Изменить конструкцию"))
        {
            static ImGuiTextFilter constrFilter;
            constrFilter.Draw("Фильтрация конструкции");

            ImGui::BeginChild("ConstrList", ImVec2(0.0f, ImGui::GetFontSize() * 24.0f),
                              ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_NavFlattened);
            for (size_t i = 0; i < m_Constructions.size(); ++i)
            {
                const auto& constr = m_Constructions[i];
                if (CustomPassFilter(constrFilter, constr.Label.c_str()) ||
                    CustomPassFilter(constrFilter, constr.Key.c_str()))
                {
                    ImGui::PushID(static_cast<int>(i));
                    if (ImGui::Selectable(std::format("{}\n{}", constr.Label, constr.Key).c_str(), false))
                    {
                        ChangeHeadersByConstruction(constr.Key);

                        std::string fieldName = "constr";
                        DeleteFromSimpleList(m_SimpleAddList, fieldName);
                        m_SimpleAddList.try_emplace(fieldName);
                        m_SimpleAddList[fieldName].push_back({ { { m_LoadedPageId } }, constr.Key });
                        SaveExtraInfoJson();

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void XlsxPageView::DrawGlobalAddListWindow()
    {
        std::string windowName = "Глобальный список заполнения";
        if (ImGui::Begin(windowName.c_str()))
        {
            std::string toDeleteName;

            for (auto& [globalAddListFieldName, globalAddListFieldValue] : m_GlobalAddList)
            {
                ImGui::PushID(globalAddListFieldName.c_str());
                ImGui::Text("%s", globalAddListFieldName.c_str());

                if (IsExtraInfoAutoFocusField(windowName, globalAddListFieldName))
                {
                    ImGui::SetKeyboardFocusHere();
                }
                ImGui::InputText(std::format("##{}", globalAddListFieldName).c_str(), &globalAddListFieldValue);
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    PushHistory();
                    SaveExtraInfoJson();
                }

                ImGui::SameLine();
                if (DeleteButton())
                {
                    toDeleteName = globalAddListFieldName;
                }

                ImGui::Spacing();
                ImGui::PopID();
            }

            bool isFirstTimeOpened = false;
            if (ImGui::Button("Добавить поле"))
            {
                ImGui::OpenPopup("Добавление поля##GlobalAddList");
                isFirstTimeOpened = true;
            }

            if (ImGui::BeginPopup("Добавление поля##GlobalAddList"))
            {
                static ImGuiTextFilter fieldsFilter;
                if (isFirstTimeOpened)
                {
                    ImGui::SetKeyboardFocusHere();
                }
                fieldsFilter.Draw("Фильтрация полей##GlobalAddList");

                ImGui::BeginChild("GlobalAddListField", ImVec2(0.0f, ImGui::GetFontSize() * 24.0f),
                                  ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_NavFlattened);
                for (const auto& [fieldName, fieldDescr] : m_FieldsDescription)
                {
                    if (m_GlobalAddList.contains(fieldName))
                    {
                        continue;
                    }

                    if (fieldsFilter.PassFilter(fieldDescr.Description.c_str()) ||
                        fieldsFilter.PassFilter(fieldName.c_str()))
                    {
                        ImGui::PushID(fieldName.c_str());
                        if (ImGui::Selectable(std::format("{}\n{}", fieldDescr.Description, fieldName).c_str(), false))
                        {
                            m_GlobalAddList[fieldName] = "";
                            m_ExtraInfoAutoFocusField = ExtraInfoAutoFocusField {
                                .WindowName = windowName,
                                .Field = fieldName,
                            };
                            fieldsFilter.Clear();
                            ImGui::CloseCurrentPopup();
                            SaveExtraInfoJson();
                        }
                        ImGui::PopID();
                        ImGui::Spacing();
                    }
                }
                ImGui::EndChild();

                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (!toDeleteName.empty())
            {
                m_GlobalAddList.erase(toDeleteName);
            }
        }
        ImGui::End();
    }

    template <DerivedFromSimpleListItemBase T>
    void XlsxPageView::DeleteFromSimpleList(std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                            std::string_view _DeleteName)
    {
        if (!_DeleteName.empty())
        {
            auto& items = _SimpleList[_DeleteName.data()];

            if (auto it = std::ranges::find_if(items,
                                               [this](T& item) {
                                                   return std::ranges::find(item.SharedPages, m_LoadedPageId) !=
                                                          item.SharedPages.end();
                                               });
                it != items.end())
            {
                std::erase(it->SharedPages, m_LoadedPageId);
            }

            std::erase_if(items, [](const T& item) { return item.SharedPages.empty(); });

            if (items.empty())
            {
                _SimpleList.erase(_DeleteName.data());
            }
        }
    }

    template <DerivedFromSimpleListItemBase T>
    void XlsxPageView::DrawSimpleListTemplateWindow(std::string_view _WindowName,
                                                    std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                                    std::function<void(std::string_view, T&)> _ItemInputHandle)
    {
        std::string toDeleteName;

        for (auto& [simpleListFieldName, simpleListPages] : _SimpleList)
        {
            ImGui::PushID(simpleListFieldName.c_str());
            for (T& simpleListItem : simpleListPages)
            {
                const std::vector<int>& sharedPages = simpleListItem.SharedPages;
                if (std::ranges::find(sharedPages, m_LoadedPageId) == sharedPages.end())
                {
                    continue;
                }

                ImGui::Text("%s", simpleListFieldName.c_str());
                ImGui::SameLine();
                std::string sharedPagesStr = StrJoin(sharedPages, ", ");
                ImGui::TextDisabled("%s", sharedPagesStr.c_str());

                if (IsExtraInfoAutoFocusField(_WindowName, simpleListFieldName))
                {
                    ImGui::SetKeyboardFocusHere();
                }
                _ItemInputHandle(simpleListFieldName, simpleListItem);
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    PushHistory();
                    SaveExtraInfoJson();
                }

                ImGui::SameLine();
                if (DeleteButton())
                {
                    toDeleteName = simpleListFieldName;
                }

                ImGui::Spacing();

                break;
            }
            ImGui::PopID();
        }

        std::string popupName = std::format("Добавление поля##{}", _WindowName);

        bool isFirstTimeOpened = false;
        if (ImGui::Button("Добавить поле"))
        {
            ImGui::OpenPopup(popupName.c_str());
            isFirstTimeOpened = true;
        }

        if (ImGui::BeginPopup(popupName.c_str()))
        {
            static ImGuiTextFilter fieldsFilter;
            if (isFirstTimeOpened)
            {
                ImGui::SetKeyboardFocusHere();
            }
            fieldsFilter.Draw("Фильтрация полей");

            ImGui::BeginChild("SimpleListField", ImVec2(0.0f, ImGui::GetFontSize() * 24.0f),
                              ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_NavFlattened);
            for (const auto& [fieldName, fieldDescr] : m_FieldsDescription)
            {
                if (IsItemInSimpleListForCurrentPage(_SimpleList, fieldName))
                {
                    continue;
                }

                if (fieldsFilter.PassFilter(fieldDescr.Description.c_str()) ||
                    fieldsFilter.PassFilter(fieldName.c_str()))
                {
                    ImGui::PushID(fieldName.c_str());
                    if (ImGui::Selectable(std::format("{}\n{}", fieldDescr.Description, fieldName).c_str(), false))
                    {
                        _SimpleList.try_emplace(fieldName);
                        _SimpleList[fieldName].push_back({ { { m_LoadedPageId } }, "" });
                        m_ExtraInfoAutoFocusField = ExtraInfoAutoFocusField {
                            .WindowName = _WindowName.data(),
                            .Field = fieldName,
                        };
                        fieldsFilter.Clear();
                        ImGui::CloseCurrentPopup();
                        SaveExtraInfoJson();
                    }

                    if (_SimpleList.contains(fieldName))
                    {
                        for (T& simpleListItem : _SimpleList[fieldName])
                        {
                            std::vector<int>& sharedPages = simpleListItem.SharedPages;
                            std::string sharedPagesStr = StrJoin(sharedPages, ", ");

                            ImGui::Text("\t");
                            ImGui::SameLine();
                            ImGui::BeginGroup();
                            ImGui::SetNextItemAllowOverlap();
                            if (ImGui::Selectable(
                                    std::format("{}\n\t##{}", simpleListItem.Value, static_cast<void*>(&simpleListItem))
                                        .c_str(),
                                    false))
                            {
                                sharedPages.push_back(m_LoadedPageId);
                                std::ranges::sort(sharedPages);
                                fieldsFilter.Clear();
                                ImGui::CloseCurrentPopup();
                                SaveExtraInfoJson();
                            }
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeightWithSpacing());
                            ImGui::TextDisabled("%s", sharedPagesStr.c_str());
                            ImGui::EndGroup();
                        }
                    }

                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        DeleteFromSimpleList(_SimpleList, toDeleteName);
    }

    void XlsxPageView::DrawSimpleAddListWindow()
    {
        std::string windowTitle = "Постраничный список заполнения";
        std::function<void(std::string_view, SimpleAddListItem&)> handle = [this](std::string_view _SimpleListFieldName,
                                                                                  SimpleAddListItem& _SimpleListItem) {
            if (m_FieldsRepresentation.contains(_SimpleListFieldName.data()))
            {
                const auto& items = m_FieldsRepresentation[_SimpleListFieldName.data()];

                std::string displayName = _SimpleListItem.Value;
                for (const auto& item : items)
                {
                    if (item.Key == _SimpleListItem.Value)
                    {
                        displayName = std::format("{} ({})", item.Key, item.Ru);
                        break;
                    }
                }

                if (ImGui::BeginCombo("##Combo", displayName.c_str()))
                {
                    for (const auto& item : items)
                    {
                        const bool is_selected = (_SimpleListItem.Value == item.Key);
                        if (ImGui::Selectable(std::format("{} ({})", item.Key, item.Ru).c_str(), is_selected))
                        {
                            _SimpleListItem.Value = item.Key;
                        }

                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                ImGui::InputText(std::format("##{}", _SimpleListFieldName).c_str(), &_SimpleListItem.Value);
            }
        };

        if (ImGui::Begin(windowTitle.c_str()))
        {
            DrawSimpleListTemplateWindow(windowTitle, m_SimpleAddList, handle);
        }
        ImGui::End();
    }

    void XlsxPageView::DrawSimpleCalcListWindow()
    {
        std::string windowTitle = "Постраничный список рассчета";
        std::function<void(std::string_view, SimpleAddListItem&)> handle = [](std::string_view _SimpleListFieldName,
                                                                              SimpleAddListItem& _SimpleListItem) {
            float height =
                ImGui::GetTextLineHeight() * static_cast<float>(std::ranges::count(_SimpleListItem.Value, '\n') + 1) +
                ImGui::GetStyle().FramePadding.y * 2;
            ImGui::InputTextMultiline(std::format("##{}", _SimpleListFieldName).c_str(), &_SimpleListItem.Value,
                                      { 0.0f, height });
            // ImGui::InputText(std::format("##{}", _SimpleListFieldName).c_str(), &_SimpleListItem.Value);
        };

        if (ImGui::Begin(windowTitle.c_str()))
        {
            ImGui::Text(
                "Python code. Example: result.append(df['bsg'][i].replace(' ', '_') + ':N' + str(df['dcon'][i]))");
            ImGui::Separator();

            DrawSimpleListTemplateWindow(windowTitle, m_SimpleCalcList, handle);
        }
        ImGui::End();
    }

    void XlsxPageView::DrawImgsPerListWindow()
    {
        if (!m_Project)
        {
            return;
        }

        static float elementHeight = ImGui::GetFontSize() * 10;
        float elementWidth = elementHeight * 3;

        // TODO: Implement Buttons
        if (ImGui::Begin("Картинки для страницы"))
        {
            ImGui::Button("Обновить все");
            for (std::string_view filetype : kImgFileTypeList)
            {
                ImGui::PushID(filetype.data());

                ImGui::SeparatorText(filetype == "pic" ? "Фото" : "Чертеж");

                Ref<Texture2D> texture = nullptr;
                const std::string imgFilename = GetRawImgFilename(filetype);
                if (TextureManager::Contains(imgFilename))
                {
                    texture = TextureManager::Get(imgFilename);
                }
                else if (std::filesystem::exists(imgFilename))
                {
                    texture = TextureManager::AddOrReplace(imgFilename);
                }

                if (texture)
                {
                    elementWidth = ImGui::GetContentRegionAvail().x;
                    float imgSizeCoef =
                        glm::min(elementWidth / texture->GetWidth(), elementHeight / texture->GetHeight());
                    ImVec2 imgSize { texture->GetWidth() * imgSizeCoef, texture->GetHeight() * imgSizeCoef };
                    ImGui::Image(reinterpret_cast<ImTextureID>(texture->GetTextureId()), imgSize);
                }
                else
                {
                    ImGui::Button("##empty", { elementHeight, elementHeight });
                }

                if (ImGui::Button("Сделать скиншот"))
                {
                    const std::string imgFilename = GetRawImgFilename(filetype);
                    MakeScreenshot(imgFilename);
                    TextureManager::RemoveFile(imgFilename);
                }
                ImGui::SameLine();
                if (ImGui::Button("Вставить из буфера"))
                {
                    const std::string imgFilename = GetRawImgFilename(filetype);
                    MakeScreenshotFromClipboard(imgFilename);
                    TextureManager::RemoveFile(imgFilename);
                }

                ImGui::PopID();
            }
        }
        ImGui::End();
    }

    void XlsxPageView::DrawJoinModal()
    {
        static bool isSkipEmpty = true;
        static std::string joinStr;

        if (m_IsJoinModalOpen)
        {
            if (ImGui::Begin("JoinModal", &m_IsJoinModalOpen))
            {
                ImGui::InputText("Разделитель", &joinStr);
                ImGui::Text("Текст разделителя: '%s'", joinStr.c_str());
                ImGui::Checkbox("Соединять пустые ячейки", &isSkipEmpty);

                ImGui::Separator();

                if (ImGui::Button("Применить"))
                {
                    SelectionRegion selectionRegion = GetSelectionRegion(false);
                    if (selectionRegion.RowsCount > 0 && selectionRegion.ColsCount > 0)
                    {
                        for (auto& row : std::ranges::subrange(m_TableData.begin() + selectionRegion.StartRow,
                                                               m_TableData.begin() + selectionRegion.StartRow +
                                                                   selectionRegion.RowsCount))
                        {
                            auto& writeCell = *(row.begin() + selectionRegion.StartCol);
                            for (auto& cell : std::ranges::subrange(row.begin() + selectionRegion.StartCol + 1,
                                                                    row.begin() + selectionRegion.StartCol +
                                                                        selectionRegion.ColsCount))
                            {
                                if (cell.Value.empty() && isSkipEmpty)
                                {
                                    continue;
                                }
                                writeCell.Value = StrJoin({ writeCell.Value, cell.Value }, joinStr);
                                cell.Value = "";
                            }
                        }
                    }

                    PushHistory();
                    m_IsJoinModalOpen = false;
                }
            }
            ImGui::End();
        }
    }

    void XlsxPageView::HandleImGuiEvents()
    {
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || m_TableData.empty())
        {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();

        if (m_SelectedRow.has_value() && (*m_SelectedRow >= m_TableData.size()))
        {
            m_SelectedRow = std::nullopt;
        }
        if (m_SelectedCol.has_value() && ((m_TableData.size() == 0) || (*m_SelectedCol >= m_TableData[0].size())))
        {
            m_SelectedCol = std::nullopt;
        }
        for (auto& selectedCellRef :
             std::array<std::reference_wrapper<std::optional<glm::u64vec2>>, 2> { m_SelectedCell, m_ExtraSelectedCell })
        {
            auto& selectedCell = selectedCellRef.get();
            if (selectedCell.has_value() &&
                (((m_TableData.size() == 0) || (selectedCell->x >= m_TableData[0].size())) ||
                 (selectedCell->y >= m_TableData.size())))
            {
                selectedCell = std::nullopt;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !m_IsAnyCellActive)
        {
            LOG_CORE_INFO("Escape key pressed");
            ImGui::ClearActiveID();
            ImGui::SetNavID(0, ImGuiNavLayer_Main, 0, {});
            ImGui::SetWindowFocus(GetWindowName());
            UnSelectAll();
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyCtrl && !m_IsAnyCellActive)
        {
            Undo();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Y) && io.KeyCtrl && !m_IsAnyCellActive)
        {
            Redo();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_J) && io.KeyCtrl)
        {
            m_IsJoinModalOpen = true;
        }

        // TODO: Add range for Key_X and Key_C

        if (ImGui::IsKeyPressed(ImGuiKey_X) && io.KeyCtrl)
        {
            SelectionRegion selectedRegion = GetSelectionRegion(io.KeyShift);
            CopySelectedToClipboard(selectedRegion);
            ClearSelected(selectedRegion);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_C) && io.KeyCtrl)
        {
            SelectionRegion selectedRegion = GetSelectionRegion(io.KeyShift);
            CopySelectedToClipboard(selectedRegion);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_V) && io.KeyCtrl)
        {
            // TODO: Insert with column or row selectd (with shift and without)
            InsertFromClipboard();
        }

        // TODO: Move to separate function
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) && io.KeyCtrl)
        {
            size_t selectedRow = io.KeyAlt ? 0 : m_TableData.size();
            size_t selectedCol = io.KeyAlt ? 0 : m_TableData[0].size();
            if (m_SelectedCell.has_value())
            {
                selectedCol = m_SelectedCell->x + 1;
                selectedRow = m_SelectedCell->y + 1;
            }
            else if (m_SelectedRow.has_value())
            {
                selectedRow = *m_SelectedRow + 1;
            }
            else if (m_SelectedCol.has_value())
            {
                selectedCol = *m_SelectedCol + 1;
            }

            bool isAddToCol = io.KeyShift;
            size_t selectedItem = isAddToCol ? selectedCol : selectedRow;
            if ((selectedItem > 0) && io.KeyAlt)
            {
                --selectedItem;
            }

            if (isAddToCol)
            {
                InsertCol(selectedItem);
            }
            else
            {
                InsertRow(selectedItem);
            }
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Minus) && io.KeyCtrl && !io.KeyShift && !io.KeyAlt)
        {
            if (m_SelectedRow.has_value())
            {
                DeleteRow(*m_SelectedRow);
            }
            if (m_SelectedCol.has_value())
            {
                DeleteCol(*m_SelectedCol);
            }
            if ((m_SelectedRow.has_value() || m_SelectedCol.has_value()) && (m_TableData.size() == 0))
            {
                m_TableData.resize(1, std::vector<TableCell>(1, { .Value = "" }));
            }
            return;
        }

        // TODO:
        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {

            SelectionRegion selectedRegion = GetSelectionRegion(io.KeyShift);
            ClearSelected(selectedRegion);
            return;
        }
    }

    void XlsxPageView::PushCellFrameBgColor(bool _IsRowHovered, bool _IsColHovered, size_t _RowId, size_t _ColId)
    {
        float frameBgAlpha = 0.125f;
        float frameBgBlue = 0.0f;

        bool isHovered = _IsRowHovered || _IsColHovered;

        bool isSelected = IsInSelectionRegion(GetSelectionRegion(false), _RowId, _ColId);

        // Check if the cell is selected
        if (isSelected && isHovered)
        {
            frameBgAlpha = 0.87f;
            frameBgBlue = 0.65f;
        }
        else if (isHovered)
        {
            frameBgAlpha = 0.6f;
            frameBgBlue = 0.5f;
        }
        else if (isSelected)
        {
            frameBgAlpha = 0.75f;
            frameBgBlue = 0.75f;
        }

        switch (m_TableData[_RowId][_ColId].Check)
        {
            case CheckStatus::kOk:
                ImGui::PushStyleColor(ImGuiCol_FrameBg, GetFrameBgColorOk(frameBgAlpha, frameBgBlue));
                break;
            case CheckStatus::kWarning:
                ImGui::PushStyleColor(ImGuiCol_FrameBg, GetFrameBgColorWarn(frameBgAlpha, frameBgBlue));
                break;
            case CheckStatus::kError:
                ImGui::PushStyleColor(ImGuiCol_FrameBg, GetFrameBgColorError(frameBgAlpha, frameBgBlue));
                break;
            default: ImGui::PushStyleColor(ImGuiCol_FrameBg, GetFrameBgColorNone(frameBgAlpha, frameBgBlue)); break;
        }
    }

    XlsxPageView::DrawTableHeaderReturn XlsxPageView::DrawTableHeader(size_t _ColsCount)
    {
        DrawTableHeaderReturn result;

        for (size_t colId = 0; colId < _ColsCount + 1; ++colId)
        {
            ImGui::TableSetupColumn(std::format("Header_{}", colId).c_str());
        }

        ImGui::TableNextRow();
        ImGui::PushID(static_cast<int>(0));
        ImGui::TableSetColumnIndex(static_cast<int>(0));
        ImGui::TableHeader("  . . .  ##Header");
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !m_IsAnyCellActive && m_IsMainWindowFocused)
        {
            ImGui::SetFocusID(ImGui::GetItemID(), ImGui::GetCurrentWindow());
        }
        if (ImGui::BeginPopupContextItem("Header_0"))
        {
            DrawTableHeaderRowContextMenu();

            ImGui::Separator();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        for (size_t colId = 0; colId < _ColsCount; ++colId)
        {
            ImGui::PushID(static_cast<int>(colId));
            ImGui::TableSetColumnIndex(static_cast<int>(colId + 1));

            auto& t = m_TableData[0][colId].Value;
            // std::string headerText = std::format("{}##_Header_{}", t, colId + 1);
            ImU32 headerColor = 0xFF000000;
            if (m_GlobalAddList.contains(t) || IsItemInSimpleListForCurrentPage(m_SimpleAddList, t) ||
                IsItemInSimpleListForCurrentPage(m_SimpleCalcList, t))
            {
                headerColor = 0xFF008000;
            }
            else if (std::ranges::find(kProductBaseFields, t) != kProductBaseFields.end())
            {
                headerColor = 0xFF555555;
            }
            else if (m_FieldsDescription.contains(t))
            {
                if (m_FieldsDescription[t].IsDescr)
                {
                    headerColor = 0xFF008CFF;
                }
                else if (!m_FieldsDescription[t].AllowNulls)
                {
                    headerColor = 0xFFCC3299;
                }
            }
            ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, headerColor);
            ImGui::TableHeader(std::format("  {}  ", t).c_str());
            ImGui::PopStyleColor();

            bool autoFocusNameChange = false;
            std::string popupStrId = std::format("Header_{}", colId);
            if (ImGui::IsItemHovered())
            {
                result.HoveredCol = colId;

                if (m_FieldsDescription.contains(t))
                {
                    ImGui::SetTooltip("%s", m_FieldsDescription[t].Description.c_str());
                }
            }

            if (ImGui::IsItemActivated() && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImGui::OpenPopup(popupStrId.c_str());
            }

            if ((ImGui::IsItemHovered() || ImGui::IsItemFocused()) && ImGui::IsKeyPressed(ImGuiKey_F2))
            {
                autoFocusNameChange = true;
                ImGui::OpenPopup(popupStrId.c_str());
            }

            if (ImGui::IsItemFocused())
            {
                UnSelectAll();
                m_SelectedCol = colId;
            }

            if (ImGui::BeginPopupContextItem(popupStrId.c_str()))
            {
                if (autoFocusNameChange)
                {
                    ImGui::SetKeyboardFocusHere();
                }
                ImGui::InputText("Имя заголовка##HeaderInput", &t);
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    PushHistory();
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::IsItemActive())
                {
                    m_IsAnyHeaderActive = true;
                }

                ImGui::SeparatorText("Fix Data");

                // TODO: Move to separate function
                if (ImGui::Button("Fix as float"))
                {
                    std::unordered_map<char, char> replacements = {
                        { ',', '.' },
                        { 'O', '0' },
                        { 'o', '0' },
                        { 'D', '0' },
                        { 'Q', '0' },
                        { 'i', '1' },
                        { 'I', '1' },
                        { 'l', '1' },
                        { 'L', '1' },
                        { '|', '1' },
                        { 'Z', '2' },
                        { 'A', '4' },
                        { 'H', '4' },
                        { 'S', '5' },
                        { 'G', '6' },
                        { 'b', '6' },
                        { 'B', '8' },
                        { 'g', '9' },
                        { 'q', '9' },
                    };

                    for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
                    {
                        auto& cellText = m_TableData[rowId][colId].Value;
                        for (auto& ch : cellText)
                        {
                            if (replacements.find(ch) != replacements.end())
                            {
                                ch = replacements[ch];
                            }
                        }

                        size_t pos = cellText.find("..");
                        while (pos != std::string::npos)
                        {
                            cellText.replace(pos, 2, ".");
                            pos = cellText.find("..", pos + 1);
                        }
                    }

                    PushHistory();
                }

                ImGui::SeparatorText("Checks");

                // TODO: Move to separate function
                if (ImGui::Button("Clear Checks"))
                {
                    for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
                    {
                        m_TableData[rowId][colId].Check = CheckStatus::kNone;
                    }

                    PushHistory();
                }

                // TODO: Move to separate function
                if (ImGui::Button("Check for float"))
                {
                    for (size_t rowId = 1; rowId < m_TableData.size(); ++rowId)
                    {
                        auto& cellText = m_TableData[rowId][colId].Value;
                        float _;
                        auto [ptr, ec] = std::from_chars(cellText.data(), cellText.data() + cellText.size(), _);
                        bool result = ec == std::errc() && ptr == cellText.data() + cellText.size();
                        if (result)
                        {
                            m_TableData[rowId][colId].Check = CheckStatus::kOk;
                        }
                        else
                        {
                            m_TableData[rowId][colId].Check = CheckStatus::kError;
                        }
                    }

                    PushHistory();
                }

                ImGui::SeparatorText("Actions");
                if (ImGui::Button("Delete Col"))
                {
                    m_DeleteCol = colId;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::Separator();
                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::PopID();

        return result;
    }

    void XlsxPageView::DrawTableHeaderRowContextMenu()
    {
        if (ImGui::Button("Удалить заголовок"))
        {
            m_DeleteRow = 0;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        ImGui::Text("Вторая строка станет заголовком");

        DrawTableActions();
    }

    template <DerivedFromSimpleListItemBase T>
    bool XlsxPageView::IsItemInSimpleListForCurrentPage(std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                                        std::string_view _FieldName)
    {
        return _SimpleList.contains(_FieldName.data()) &&
               std::ranges::any_of(_SimpleList[_FieldName.data()], [this](const T& item) {
                   return std::ranges::find(item.SharedPages, m_LoadedPageId) != item.SharedPages.end();
               });
    }

    template <DerivedFromSimpleListItemBase T>
    std::optional<const std::reference_wrapper<T>>
    XlsxPageView::GetItemInSimpleListForCurrentPage(std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                                    std::string_view _FieldName)
    {
        if (_SimpleList.contains(_FieldName.data()))
        {
            std::vector<T>& items = _SimpleList[_FieldName.data()];
            if (auto it = std::ranges::find_if(items,
                                               [this](T& item) {
                                                   return std::ranges::find(item.SharedPages, m_LoadedPageId) !=
                                                          item.SharedPages.end();
                                               });
                it != items.end())
            {
                return *it;
            }
        }

        return std::nullopt;
    }

    std::optional<const std::reference_wrapper<std::string>> XlsxPageView::GetExtraListValue(std::string_view _Header)
    {
        if (std::optional<std::reference_wrapper<SimpleAddListItem>> cell =
                GetItemInSimpleListForCurrentPage(m_SimpleCalcList, _Header);
            cell.has_value())
        {
            return cell->get().Value;
        }

        if (std::optional<std::reference_wrapper<SimpleAddListItem>> cell =
                GetItemInSimpleListForCurrentPage(m_SimpleAddList, _Header);
            cell.has_value())
        {
            return cell->get().Value;
        }

        if (m_GlobalAddList.contains(_Header.data()))
        {
            return m_GlobalAddList[_Header.data()];
        }

        return std::nullopt;
    }

    std::string XlsxPageView::GetRawImgFilename(std::string_view _Filetype)
    {
        std::filesystem::path filename = m_LoadedPageFilename;
        filename.replace_extension();
        filename = std::format("{}_{}.png", filename.string(), _Filetype);

        return (std::filesystem::path(m_Project->GetExcelTablesTypeRawImgsPath()) / filename).string();
    }

    void XlsxPageView::LoadXLSX()
    {
        m_LoadedPageId = -1;
        UnSelectAll();
        m_TableData.clear();
        ClearHistory();

        auto pathIterator = std::filesystem::directory_iterator(m_BasePath);
        for (int i = 0; i < m_PageId; ++i)
        {
            ++pathIterator;
        }
        if (pathIterator == std::filesystem::end(pathIterator))
        {
            LOG_CORE_ERROR("No file found for page ID: {}", m_PageId);
            return;
        }

        std::filesystem::path path = std::filesystem::path(m_BasePath) / pathIterator->path().filename();

        LOG_CORE_INFO("Loading file: {}", path.string());

        if (!std::filesystem::exists(path))
        {
            return;
        }

        xlnt::workbook wb;

        try
        {
            wb.load(path);
        }
        catch (const std::exception& e)
        {
            LOG_CORE_ERROR("Failed to load workbook: {}", e.what());
            return;
        }

        xlnt::worksheet ws = wb.active_sheet();

        for (auto row : ws.rows(false))
        {
            std::vector<TableCell> rowData;
            for (auto cell : row)
            {
                rowData.push_back({ .Value = cell.has_value() ? cell.to_string() : "" });
            }
            m_TableData.push_back(rowData);
        }

        size_t maxCols = 0;
        for (const auto& row : m_TableData)
        {
            if (row.size() > maxCols)
            {
                maxCols = row.size();
            }
        }

        for (auto& row : m_TableData)
        {
            while (row.size() < maxCols)
            {
                row.push_back({ .Value = "" });
            }
        }

        m_LoadedPageId = m_PageId;
        m_LoadedPageFilename = pathIterator->path().filename();

        PushHistory();
    }

    void XlsxPageView::SaveXLSX()
    {
        if ((m_PageId < 0) || (m_LoadedPageId < 0) || m_BasePath.empty())
        {
            return;
        }

        auto pathIterator = std::filesystem::directory_iterator(m_BasePath);
        for (int i = 0; i < m_PageId; ++i)
        {
            ++pathIterator;
        }
        if (pathIterator == std::filesystem::end(pathIterator))
        {
            LOG_CORE_ERROR("No file found for page ID: {}", m_PageId);
            return;
        }

        std::filesystem::path path = std::filesystem::path(m_BasePath) / pathIterator->path().filename().string();

        LOG_CORE_INFO("Saving file: {}", path.string());

        // if (!std::filesystem::exists(path))
        // {
        //     return;
        // }

        xlnt::workbook wb;
        LOG_CORE_INFO("Created WB");

        xlnt::worksheet ws = wb.active_sheet();
        LOG_CORE_INFO("Created WS");

        for (size_t rowId = 0; rowId < m_TableData.size(); ++rowId)
        {
            for (size_t colId = 0; colId < m_TableData[rowId].size(); ++colId)
            {
                ws.cell(static_cast<xlnt::column_t>(colId + 1), static_cast<xlnt::row_t>(rowId + 1))
                    .value(m_TableData[rowId][colId].Value);
            }
        }

        wb.save(path);

        SaveExtraInfoJson();
    }

    void XlsxPageView::LoadExtraInfoJson()
    {
        std::string filename = kExtraInfoFile.data();
        std::filesystem::path inFilePath =
            (std::filesystem::path(m_Project->GetExcelTablesTypePath()) / std::filesystem::path(filename));
        m_ExtraInfoJsonPath = inFilePath.string();
        if (!std::filesystem::exists(inFilePath))
        {
            // Overlay::Get()->Start(Format("Файл не найден: \n{}", inFilePath.string()));
            return;
        }

        std::ifstream infile(inFilePath);
        if (!infile.is_open())
        {
            Overlay::Get()->Start(Format("Не удалось открыть стандартный файл ExtraInfo: \n{}", inFilePath.string()));
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            if (json.contains("global_add_list"))
            {
                for (const auto& item : json["global_add_list"])
                {
                    m_GlobalAddList[item["name"]] = item["value"];
                }
            }

            if (json.contains("simple_add_list"))
            {
                for (const auto& item : json["simple_add_list"])
                {
                    m_SimpleAddList.try_emplace(item["name"]);
                    for (const auto& values : item["values"])
                    {
                        std::vector<int> index;
                        values["index"].get_to(index);
                        m_SimpleAddList[item["name"]].push_back({ { index }, values["value"] });
                    }
                }
            }

            if (json.contains("per_page_calc_list"))
            {
                for (const auto& item : json["per_page_calc_list"])
                {
                    m_SimpleCalcList.try_emplace(item["name"]);
                    for (const auto& values : item["values"])
                    {
                        std::vector<int> index;
                        values["index"].get_to(index);
                        m_SimpleCalcList[item["name"]].push_back({ { index }, values["exec"] });
                    }
                }
            }
        }
        catch (...)
        {
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", inFilePath.string()));
        }
    }

    void XlsxPageView::SaveExtraInfoJson()
    {

        std::ofstream fout(m_ExtraInfoJsonPath);
        if (!fout.is_open())
        {
            Overlay::Get()->Start(Format("Не удалось сохранить ExtraInfo: \n{}", m_ExtraInfoJsonPath));
            return;
        }

        nlohmann::json result;

        // result["global_add_list"] = nlohmann::json::array();
        for (const auto& [name, value] : m_GlobalAddList)
        {
            result["global_add_list"].push_back(nlohmann::json {
                {  "name",  name },
                { "value", value }
            });
        }

        for (const auto& [name, items] : m_SimpleAddList)
        {
            if (items.size() == 0)
            {
                continue;
            }

            nlohmann::json jsonItems = {
                {   "name",                    name },
                { "values", nlohmann::json::array() }
            };
            for (const SimpleAddListItem& item : items)
            {
                jsonItems["values"].push_back(nlohmann::json {
                    { "index", item.SharedPages },
                    { "value",       item.Value }
                });
            }

            result["simple_add_list"].push_back(jsonItems);
        }

        for (const auto& [name, items] : m_SimpleCalcList)
        {
            if (items.size() == 0)
            {
                continue;
            }

            nlohmann::json jsonItems = {
                {   "name",                    name },
                { "values", nlohmann::json::array() }
            };
            for (const SimpleAddListItem& item : items)
            {
                jsonItems["values"].push_back(nlohmann::json {
                    { "index", item.SharedPages },
                    {  "exec",       item.Value }
                });
            }

            result["per_page_calc_list"].push_back(jsonItems);
        }

        fout << std::setw(4) << result;
    }

    void XlsxPageView::Undo()
    {
        if (m_HistoryPointer <= 1)
        {
            return;
        }

        --m_HistoryPointer;
        RestoreFromHistory(m_HistoryState[m_HistoryPointer - 1]);
    }

    void XlsxPageView::Redo()
    {
        if (m_HistoryPointer >= m_HistoryState.size())
        {
            return;
        }

        ++m_HistoryPointer;
        RestoreFromHistory(m_HistoryState[m_HistoryPointer - 1]);
    }

    void XlsxPageView::RestoreFromHistory(const HistoryState& _HistoryState)
    {
        m_TableData = _HistoryState.DataTable;
        // TODO: fix restore of m_SelectedCell and m_ExtraSelectedCell
        m_SelectedCell = _HistoryState.SelectedCell;
        m_ExtraSelectedCell = std::nullopt;
        m_SelectedCol = _HistoryState.SelectedRow;
        m_SelectedRow = _HistoryState.SelectedRow;
    }

    void XlsxPageView::PushHistory()
    {
        m_HistoryState.erase(m_HistoryState.begin() + m_HistoryPointer, m_HistoryState.end());
        m_HistoryState.push_back({
            .DataTable = m_TableData,
            .SelectedCell = m_SelectedCell,
            .SelectedCol = m_SelectedCol,
            .SelectedRow = m_SelectedRow,
        });
        ++m_HistoryPointer;
    }

    void XlsxPageView::ClearHistory()
    {
        m_HistoryPointer = 0;
        m_HistoryState.clear();
    }

    void XlsxPageView::DeleteCol(size_t _ColId)
    {
        for (auto& row : m_TableData)
        {
            if (_ColId >= row.size())
            {
                continue;
            }
            row.erase(row.begin() + _ColId);
        }
        if ((m_TableData.size() > 0) && (m_TableData[0].size() == 0))
        {
            m_TableData.clear();
        }

        PushHistory();
    }

    void XlsxPageView::DeleteRow(size_t _RowId)
    {
        if (_RowId >= m_TableData.size())
        {
            return;
        }
        m_TableData.erase(m_TableData.begin() + _RowId);

        PushHistory();
    }

    void XlsxPageView::InsertCol(size_t _ColId)
    {
        bool isChanged = false;
        for (auto& row : m_TableData)
        {
            if (_ColId > row.size())
            {
                continue;
            }
            isChanged = true;
            row.insert(row.begin() + _ColId, { .Value = "" });
        }

        if (isChanged)
        {
            PushHistory();
        }
    }

    void XlsxPageView::InsertRow(size_t _RowId)
    {
        if (_RowId > m_TableData.size())
        {
            return;
        }
        size_t colsCount = m_TableData.size() > 0 ? m_TableData[0].size() : 1;
        m_TableData.insert(m_TableData.begin() + _RowId, std::vector<TableCell>(colsCount, { .Value = "" }));

        PushHistory();
    }

    void XlsxPageView::ReplaceFromClipboard(bool _IsNeedEmptyHeaderRow)
    {
        if (const char* clipboard = ImGui::GetClipboardText())
        {
            m_TableData.clear();

            std::istringstream iss(clipboard);
            std::string line;
            while (std::getline(iss, line))
            {
                std::vector<TableCell> row;
                std::istringstream lineStream(line);
                std::string cell;
                while (std::getline(lineStream, cell, '\t'))
                {
                    row.push_back({ .Value = cell });
                }
                m_TableData.push_back(std::move(row));
            }

            if (_IsNeedEmptyHeaderRow)
            {
                m_TableData.insert(m_TableData.begin(), std::vector<TableCell>());
            }
            FixDimensions();
            PushHistory();
        }
    }

    XlsxPageView::SelectionRegion XlsxPageView::GetSelectionRegion(bool _IncludeHeader)
    {
        if ((!m_SelectedCell.has_value() && !m_SelectedRow.has_value() && !m_SelectedCol.has_value()) ||
            (m_SelectedCell.has_value() && m_IsAnyCellActive) || (m_TableData.empty() || m_TableData[0].empty()))
        {
            return { .StartRow = 0, .StartCol = 0, .RowsCount = 0, .ColsCount = 0 };
        }

        // TODO: Add checks if needed

        if (m_SelectedCell.has_value())
        {
            glm::u64vec2 minCell = *m_SelectedCell;
            glm::u64vec2 maxCell = *m_SelectedCell;
            if (m_ExtraSelectedCell.has_value())
            {
                minCell = glm::min(*m_SelectedCell, *m_ExtraSelectedCell);
                maxCell = glm::max(*m_SelectedCell, *m_ExtraSelectedCell);
            }
            return {
                .StartRow = minCell.y,
                .StartCol = minCell.x,
                .RowsCount = maxCell.y - minCell.y + 1,
                .ColsCount = maxCell.x - minCell.x + 1,
            };
        }
        if (m_SelectedRow.has_value())
        {
            return { .StartRow = *m_SelectedRow, .StartCol = 0, .RowsCount = 1, .ColsCount = m_TableData[0].size() };
        }
        if (m_SelectedCol.has_value())
        {
            size_t offset = _IncludeHeader ? 0 : 1;
            return {
                .StartRow = offset, .StartCol = *m_SelectedCol, .RowsCount = m_TableData.size() - offset, .ColsCount = 1
            };
        }

        return { .StartRow = 0, .StartCol = 0, .RowsCount = 0, .ColsCount = 0 };
    }

    bool XlsxPageView::IsInSelectionRegion(const SelectionRegion& _SelectionRegion, size_t _RowId, size_t _ColId)
    {
        if (_SelectionRegion.RowsCount == 0 || _SelectionRegion.ColsCount == 0)
        {
            return false;
        }

        glm::u64vec2 minCell = { _SelectionRegion.StartRow, _SelectionRegion.StartCol };
        glm::u64vec2 maxCell = { _SelectionRegion.StartRow + _SelectionRegion.RowsCount - 1,
                                 _SelectionRegion.StartCol + _SelectionRegion.ColsCount - 1 };

        return _RowId >= minCell.x && _RowId <= maxCell.x && _ColId >= minCell.y && _ColId <= maxCell.y;
    }

    void XlsxPageView::CopySelectedToClipboard(const SelectionRegion& _SelectionRegion)
    {
        std::string result;
        for (const auto& row :
             std::ranges::subrange(m_TableData.begin() + _SelectionRegion.StartRow,
                                   m_TableData.begin() + _SelectionRegion.StartRow + _SelectionRegion.RowsCount))
        {
            for (const auto& cell :
                 std::ranges::subrange(row.begin() + _SelectionRegion.StartCol,
                                       row.begin() + _SelectionRegion.StartCol + _SelectionRegion.ColsCount))
            {
                result += cell.Value + "\t";
            }
            result += "\n";
        }
        ImGui::SetClipboardText(result.c_str());
    }

    void XlsxPageView::InsertFromClipboard()
    {
        const char* clipboard = ImGui::GetClipboardText();
        if (!clipboard)
        {
            return;
        }

        if (m_SelectedCell.has_value() && (!m_IsAnyCellActive || StrContainsNewlineOrTab(clipboard)))
        {
            ImGui::ClearActiveID();
            std::istringstream iss(clipboard);
            std::string line;
            for (size_t insertRow = m_SelectedCell->y; std::getline(iss, line); ++insertRow)
            {
                if (m_TableData.size() == insertRow)
                {
                    m_TableData.push_back(std::vector<TableCell>(m_TableData[0].size(), TableCell {}));
                }
                std::vector<TableCell>& row = m_TableData[insertRow];

                std::istringstream lineStream(line);
                std::string cell;
                for (size_t insertCol = m_SelectedCell->x; std::getline(lineStream, cell, '\t'); ++insertCol)
                {
                    if (row.size() == insertCol)
                    {
                        row.push_back({});
                    }
                    row[insertCol].Value = cell;
                }
            }
            FixDimensions();
            PushHistory();
        }
    }

    void XlsxPageView::ClearSelected(const SelectionRegion& _SelectionRegion)
    {
        for (auto& row :
             std::ranges::subrange(m_TableData.begin() + _SelectionRegion.StartRow,
                                   m_TableData.begin() + _SelectionRegion.StartRow + _SelectionRegion.RowsCount))
        {
            for (auto& cell :
                 std::ranges::subrange(row.begin() + _SelectionRegion.StartCol,
                                       row.begin() + _SelectionRegion.StartCol + _SelectionRegion.ColsCount))
            {
                cell = {};
            }
        }

        PushHistory();
    }

    void XlsxPageView::UnSelectAll(bool _UnSelectExtraCell)
    {
        m_SelectedCell = std::nullopt;
        if (_UnSelectExtraCell)
        {
            m_ExtraSelectedCell = std::nullopt;
        }
        m_SelectedCol = std::nullopt;
        m_SelectedRow = std::nullopt;
    }

    void XlsxPageView::SplitAndExpandTable()
    {
        if (m_TableData.empty())
        {
            return;
        }

        size_t rows = m_TableData.size();
        size_t cols = m_TableData[0].size();

        LOG_CORE_WARN("Splitting table with columns: {}", cols);

        std::vector<std::vector<std::vector<std::string>>> colsSplitData(cols);
        for (auto& col : colsSplitData)
        {
            col.resize(m_TableData.size());
        }
        std::vector<size_t> colsAfterSplitCount(cols, 1);

        for (size_t col = 0; col < cols; ++col)
        {
            for (size_t row = 0; row < m_TableData.size(); ++row)
            {
                std::istringstream iss(m_TableData[row][col].Value);
                std::string word;

                while (iss >> word)
                {
                    colsSplitData[col][row].push_back(word);
                }
                if (colsSplitData[col][row].size() == 0)
                {
                    colsSplitData[col][row].push_back("");
                }

                colsAfterSplitCount[col] = std::max(colsAfterSplitCount[col], colsSplitData[col][row].size());
            }
        }

        size_t totalSplitCount = std::accumulate(colsAfterSplitCount.begin(), colsAfterSplitCount.end(), 0);

        m_TableData.clear();
        m_TableData.resize(rows, std::vector<TableCell>(totalSplitCount, { .Value = "" }));

        for (size_t oldCol = 0; oldCol < cols; ++oldCol)
        {
            size_t newCol = 0;
            for (size_t i = 0; i < oldCol; ++i)
            {
                newCol += colsAfterSplitCount[i];
            }

            for (size_t row = 0; row < m_TableData.size(); ++row)
            {
                for (size_t col = 0; col < colsAfterSplitCount[oldCol]; ++col)
                {
                    m_TableData[row][newCol + col] = {
                        .Value = col < colsSplitData[oldCol][row].size() ? colsSplitData[oldCol][row][col] : "",
                    };
                }
            }
        }

        PushHistory();
    }

    void XlsxPageView::FixDimensions()
    {
        size_t maxCols = 0;
        for (const auto& row : m_TableData)
        {
            maxCols = std::max(maxCols, row.size());
        }
        for (auto& row : m_TableData)
        {
            row.resize(maxCols, { .Value = "" });
        }
    }

    void XlsxPageView::ChangeHeadersByConstruction(std::string_view _ConstrKey)
    {
        if (!m_ConstructionsFields.contains(_ConstrKey.data()))
        {
            Overlay::Get()->Start(Format("Не найдены поля для конструкции: \n{}", _ConstrKey));
        }
        if (m_TableData.size() == 0)
        {
            m_TableData.push_back(std::vector<TableCell>());
        }

        std::vector<TableCell>& headerRow = m_TableData[0];
        headerRow.clear();
        for (const std::string& field : kProductBaseFields)
        {
            headerRow.push_back({ .Value = field });
        }
        for (std::string& field : m_ConstructionsFields[_ConstrKey.data()])
        {
            headerRow.push_back({ .Value = field });
        }

        FixDimensions();
        PushHistory();
    }

    void XlsxPageView::LoadConstructionsTree()
    {
        std::ifstream infile(kDefaultConstructionsTreeFile.data());
        if (!infile.is_open())
        {
            Overlay::Get()->Start(
                Format("Не удалось открыть стандартный файл дерева конструкций: \n{}", kDefaultConstructionsTreeFile));
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            for (const auto& jsonEntity : json["nodes"])
            {
                for (const auto& jsonGroup : jsonEntity["nodes"])
                {
                    for (const auto& jsonConstr : jsonGroup["nodes"])
                    {
                        ConstructionTreeConstr constr {
                            { .Img = jsonConstr["img_link"], .Label = jsonConstr["text"], .Key = jsonConstr["key"] },
                        };
                        constr.Drw = jsonConstr["drw_link"];
                        m_Constructions.emplace_back(std::move(constr));
                    }
                }
            }
        }
        catch (...)
        {
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kDefaultConstructionsTreeFile));
        }
    }

    void XlsxPageView::LoadConstructionsFields()
    {
        std::ifstream infile(kDefaultConstructionsFieldsFile.data());
        if (!infile.is_open())
        {
            Overlay::Get()->Start(
                Format("Не удалось открыть стандартный файл конструкций: \n{}", kDefaultConstructionsFieldsFile));
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            for (const auto& element : json.items())
            {
                std::vector<std::string> v;
                element.value().get_to(v);
                m_ConstructionsFields[element.key()] = v;
            }
        }
        catch (...)
        {
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kDefaultConstructionsFieldsFile));
        }
    }

    void XlsxPageView::LoadFieldsDescription()
    {
        std::ifstream infile(kDefaultFieldsDescriptionFile.data());
        if (!infile.is_open())
        {
            Overlay::Get()->Start(Format("Не удалось открыть стандартный файл описания полей конструкций: \n{}",
                                         kDefaultFieldsDescriptionFile));
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            for (const auto& element : json)
            {
                m_FieldsDescription[element["fieldname"].get<std::string>()] = FieldDescription {
                    .Type = element["typ"],
                    .UnitRu = element.at("unitru").get<std::optional<std::string>>(),
                    .RuDescription = element["rudescription"],
                    .AllowNulls = element["allownulls"],
                    .IfBooleanTrue = element.at("ifbooleantrue").get<std::optional<std::string>>(),
                    .IfBooleanFalse = element.at("ifbooleanfalse").get<std::optional<std::string>>(),
                    .QType = element["qtyp"],
                    .IsDescr = element["is_descr"],
                    .Description = element["descr"],
                };
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kDefaultFieldsDescriptionFile));
        }
    }

    void XlsxPageView::LoadRepresentationFieldsDescription()
    {
        std::ifstream infile(kDefaultRepresentationFieldsDescriptionFile.data());
        if (!infile.is_open())
        {
            Overlay::Get()->Start(Format("Не удалось открыть стандартный файл представления полей: \n{}",
                                         kDefaultRepresentationFieldsDescriptionFile));
        }

        try
        {
            nlohmann::json json;
            infile >> json;

            for (const auto& element : json.items())
            {
                std::vector<FieldRepresentationItem> v;
                for (const auto& item : element.value())
                {
                    std::string key;
                    if (item["key"].is_number())
                    {
                        uint32_t val = item["key"];
                        key = std::to_string(val);
                    }
                    else
                    {
                        key = item["key"];
                    }

                    v.push_back({
                        .Key = key,
                        .RuShort = item["rushort"],
                        .Ru = item["ru"],
                    });
                }
                m_FieldsRepresentation[element.key()] = v;
            }
        }
        catch (std::exception& e)
        {
            LOG_CORE_WARN("{}", e.what());
            Overlay::Get()->Start(
                Format("Ошибка во время чтения формата json: \n{}", kDefaultRepresentationFieldsDescriptionFile));
        }
    }

    bool XlsxPageView::IsExtraInfoAutoFocusField(std::string_view _WindowName, std::string_view _FieldName)
    {
        bool result = m_ExtraInfoAutoFocusField.has_value() && (m_ExtraInfoAutoFocusField->WindowName == _WindowName) &&
                      (m_ExtraInfoAutoFocusField->Field == _FieldName);
        if (result)
        {
            m_ExtraInfoAutoFocusField = std::nullopt;
        }
        return result;
    }

}    // namespace LM
