#pragma once

#include "IPageView.h"
#include "ImGui/Constructions/SelectConstructionFromTree.hpp"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ImVec4;

namespace LM
{

    struct SimpleListItemBase
    {
        std::vector<int> SharedPages;
    };

    template <typename T>
    concept DerivedFromSimpleListItemBase = std::is_base_of_v<SimpleListItemBase, T>;

    struct SimpleAddListItem : public SimpleListItemBase
    {
        std::string Value;
    };

    class XlsxPageView : public IPageView
    {
    protected:
        enum class CheckStatus
        {
            kNone,
            kOk,
            kWarning,
            kError,
        };

        struct TableCell
        {
            std::string Value = "";
            CheckStatus Check = CheckStatus::kNone;
        };

        struct HistoryState
        {
            std::vector<std::vector<TableCell>> DataTable;

            std::optional<glm::u64vec2> SelectedCell = std::nullopt;
            std::optional<size_t> SelectedCol = std::nullopt;
            std::optional<size_t> SelectedRow = std::nullopt;
        };

        struct DrawTableHeaderReturn
        {
            std::optional<size_t> HoveredCol = std::nullopt;
        };

        struct FieldDescription
        {
            std::string Type;
            std::optional<std::string> UnitRu;
            std::string RuDescription;
            bool AllowNulls;
            std::optional<std::string> IfBooleanTrue;
            std::optional<std::string> IfBooleanFalse;
            int QType;
            bool IsDescr;
            std::string Description;
        };

        struct ExtraInfoAutoFocusField
        {
            std::string WindowName;
            std::string Field;
        };

    public:
        XlsxPageView();
        virtual ~XlsxPageView();

        virtual bool OnPageWillBeChanged(int _CurrentPageId, int _NewPageId) override;

    protected:
        virtual std::string GetBasePath() const override { return m_Project->GetExcelTablesTypeStartupPath(); }
        virtual const char* GetWindowName() const override { return "Первый Excel"; }
        std::string GetFileName() const override;

        void DrawWindowContent() override;

    protected:
        void DrawTableActions();
        void DrawGlobalAddList();

        template <DerivedFromSimpleListItemBase T>
        void DrawSimpleListTemplate(std::string_view _WindowName,
                                    std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                    std::function<void(std::string_view, T&)> _ItemInputHandle);

        void DrawSimpleAddList();
        void DrawSimpleCalcList();

        void HandleImGuiEvents();

        void PushCellFrameBgColor(bool _IsRowHovered, bool _IsColHovered, size_t _RowId, size_t _ColId);

        DrawTableHeaderReturn DrawTableHeader(size_t _ColsCount);
        void DrawTableHeaderRowContextMenu();

        template <DerivedFromSimpleListItemBase T>
        bool IsItemInSimpleListForCurrentPage(std::unordered_map<std::string, std::vector<T>>& _SimpleList,
                                              std::string_view _FieldName);

        void LoadXLSX();
        void SaveXLSX();

        void LoadExtraInfoJson();
        void SaveExtraInfoJson();

        void Undo();
        void Redo();

        void RestoreFromHistory(const HistoryState& _HistoryState);
        void PushHistory();
        void ClearHistory();

        void DeleteCol(size_t _ColId);
        void DeleteRow(size_t _RowId);

        void InsertCol(size_t _ColId);
        void InsertRow(size_t _RowId);

        void ReplaceFromClipboard(bool _IsNeedEmptyHeaderRow);

        void CopySelectedToClipboard(bool _CopyHeader);
        void InsertFromClipboard();

        void SplitAndExpandTable();

        void FixDimensions();

        void ChangeHeadersByConstruction(std::string_view _ConstrKey);

        void LoadConstructionsTree();
        void LoadConstructionsFields();
        void LoadFieldsDescription();
        void LoadRepresentationFieldsDescription();

        bool IsExtraInfoAutoFocusField(std::string_view _WindowName, std::string_view _FieldName);

    protected:
        std::vector<std::vector<TableCell>> m_TableData;

        int m_LoadedPageId = -1;
        std::filesystem::path m_LoadedPageFilename;

        bool m_IsAnyCellActive = false;
        std::optional<glm::u64vec2> m_SelectedCell = std::nullopt;
        std::optional<size_t> m_SelectedCol = std::nullopt;
        std::optional<size_t> m_SelectedRow = std::nullopt;
        std::optional<size_t> m_DeleteCol = std::nullopt;
        std::optional<size_t> m_DeleteRow = std::nullopt;

        std::vector<ConstructionTreeConstr> m_Constructions;
        std::unordered_map<std::string, std::vector<std::string>> m_ConstructionsFields;
        std::unordered_map<std::string, FieldDescription> m_FieldsDescription;

        bool m_IsExtraInfoJsonLoaded = false;
        std::string m_ExtraInfoJsonPath;
        std::optional<ExtraInfoAutoFocusField> m_ExtraInfoAutoFocusField = std::nullopt;
        std::unordered_map<std::string, std::string> m_GlobalAddList;
        std::unordered_map<std::string, std::vector<SimpleAddListItem>> m_SimpleAddList;
        std::unordered_map<std::string, std::vector<SimpleAddListItem>> m_SimpleCalcList;

        size_t m_HistoryPointer = 0;
        std::vector<HistoryState> m_HistoryState;
    };

}    // namespace LM
