#pragma once

#include <optional>
#include <string>
#include <vector>

namespace LM
{

    struct ConstructionTreeNodeBase
    {
        std::string Img;
        std::string Label;
        std::string Key;
    };

    struct ConstructionTreeConstr : public ConstructionTreeNodeBase
    {
        std::string Drw;
    };

    struct ConstructionTreeGroup : public ConstructionTreeNodeBase
    {
        std::vector<ConstructionTreeConstr> Constructions;
    };

    struct ConstructionTreeEntity : public ConstructionTreeNodeBase
    {
        std::vector<ConstructionTreeGroup> Groups;
    };

    class SelectConstructionFromTree
    {
    public:
        static void LoadTreeFromDefaultFile();

        std::string_view operator()();

    protected:
        void DrawTree(size_t _Index, size_t _NodesCount, std::string_view _Label, std::string_view _Img,
                      float _WindowVisibleX2, std::optional<size_t>& _SelectedIndex);

    protected:
        static inline std::vector<ConstructionTreeEntity> s_TreeEntities;
        static inline int elementWidth = 256;

    protected:
        std::optional<size_t> m_SelectedEntityIndex;
        std::optional<size_t> m_SelectedGroupIndex;
        std::optional<size_t> m_SelectedConstrIndex;
    };

}    // namespace LM
