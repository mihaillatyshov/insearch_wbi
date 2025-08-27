#include "SelectConstructionFromTree.hpp"

#include "ImGui/Overlays/Overlay.h"
#include "Managers/TextureManager.h"

#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/json.hpp"
#include "Engine/Utils/utf8.h"

#include <glm/glm.hpp>
#include <imgui.h>

#include <filesystem>
#include <fstream>

namespace LM
{

    constexpr std::string_view kDefaultConstructionsTreeFile = "assets/constructions/constructions_tree.json";

    void SelectConstructionFromTree::LoadTreeFromDefaultFile()
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

            s_TreeEntities.reserve(json["nodes"].size());
            for (const auto& jsonEntity : json["nodes"])
            {
                ConstructionTreeEntity entity {
                    { .Img = jsonEntity["img_link"], .Label = jsonEntity["text"], .Key = jsonEntity["key"] }
                };

                entity.Groups.reserve(jsonEntity["nodes"].size());
                for (const auto& jsonGroup : jsonEntity["nodes"])
                {
                    ConstructionTreeGroup group {
                        { .Img = jsonGroup["img_link"], .Label = jsonGroup["text"], .Key = jsonGroup["key"] }
                    };

                    group.Constructions.reserve(jsonGroup["nodes"].size());
                    for (const auto& jsonConstr : jsonGroup["nodes"])
                    {
                        ConstructionTreeConstr constr {
                            { .Img = jsonConstr["img_link"], .Label = jsonConstr["text"], .Key = jsonConstr["key"] },
                        };
                        constr.Drw = jsonConstr["drw_link"];

                        LOGI("Constr key: ", constr.Key);
                        group.Constructions.emplace_back(std::move(constr));
                    }

                    entity.Groups.emplace_back(std::move(group));
                }

                s_TreeEntities.emplace_back(std::move(entity));
            }
        }
        catch (...)
        {
            Overlay::Get()->Start(Format("Ошибка во время чтения формата json: \n{}", kDefaultConstructionsTreeFile));
        }
    }

    std::string_view SelectConstructionFromTree::operator()()
    {
        if (ImGui::Begin("Select Construction"))
        {
            ImGui::DragInt("Размер элемента", &elementWidth, 0.5f, 128, 2048);

            ImVec2 buttonSize { static_cast<float>(elementWidth), static_cast<float>(elementWidth) };
            ImGuiStyle& style = ImGui::GetStyle();
            float windowVisibleX2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
            if (!m_SelectedEntityIndex.has_value())
            {
                for (size_t i = 0; i < s_TreeEntities.size(); ++i)
                {
                    const auto& treeEntity = s_TreeEntities[i];
                    DrawTree(i, s_TreeEntities.size(), treeEntity.Label, treeEntity.Img, windowVisibleX2,
                             m_SelectedEntityIndex);
                }
            }
            else if (!m_SelectedGroupIndex.has_value())
            {
                const auto& treeEntityGroups = s_TreeEntities[*m_SelectedEntityIndex].Groups;
                for (size_t i = 0; i < treeEntityGroups.size(); ++i)
                {
                    const auto& treeGroup = treeEntityGroups[i];
                    DrawTree(i, treeEntityGroups.size(), treeGroup.Label, treeGroup.Img, windowVisibleX2,
                             m_SelectedGroupIndex);
                }
            }
            else if (!m_SelectedConstrIndex.has_value())
            {
                const auto& treeEntityGroups = s_TreeEntities[*m_SelectedEntityIndex].Groups;
                const auto& treeGroupConstrs = treeEntityGroups[*m_SelectedGroupIndex].Constructions;
                for (size_t i = 0; i < treeGroupConstrs.size(); ++i)
                {
                    const auto& treeConstr = treeGroupConstrs[i];
                    DrawTree(i, treeGroupConstrs.size(), treeConstr.Label, treeConstr.Img, windowVisibleX2,
                             m_SelectedConstrIndex);
                }
            }
        }
        ImGui::End();

        if (m_SelectedConstrIndex.has_value())
        {
            size_t entityIndex = *m_SelectedEntityIndex;
            size_t groupIndex = *m_SelectedGroupIndex;
            size_t constrIndex = *m_SelectedConstrIndex;

            m_SelectedEntityIndex.reset();
            m_SelectedGroupIndex.reset();
            m_SelectedConstrIndex.reset();

            const auto& treeEntityGroups = s_TreeEntities[entityIndex].Groups;
            const auto& treeGroupConstrs = treeEntityGroups[groupIndex].Constructions;
            return treeGroupConstrs[constrIndex].Key;
        }

        return std::string_view();
    }

    void SelectConstructionFromTree::DrawTree(size_t _Index, size_t _NodesCount, std::string_view _Label,
                                              std::string_view _Img, float _WindowVisibleX2,
                                              std::optional<size_t>& _SelectedIndex)
    {
        ImVec2 buttonSize { static_cast<float>(elementWidth), static_cast<float>(elementWidth) };
        ImGuiStyle& style = ImGui::GetStyle();

        const std::string imgFilename =
            (std::filesystem::path("assets/textures") / std::filesystem::path(_Img)).string();

        if (!TextureManager::Contains(imgFilename))
        {
            TextureManager::AddOrReplace(imgFilename);
        }

        Ref<Texture2D> image = TextureManager::Get(imgFilename);

        ImGui::PushID(static_cast<int>(_Index));
        ImGui::BeginGroup();
        ImVec2 buttonPos = ImGui::GetCursorScreenPos();
        float imgSizeCoef = glm::max(image->GetWidth(), image->GetHeight());
        ImVec2 imgSize { image->GetWidth() / imgSizeCoef * buttonSize.x,
                         image->GetHeight() / imgSizeCoef * buttonSize.y };
        ImVec2 imgPos =
            ImVec2(buttonPos.x + (buttonSize.x - imgSize.x) * 0.5f, buttonPos.y + (buttonSize.y - imgSize.y) * 0.5f);
        if (ImGui::Button("", buttonSize))
        {
            _SelectedIndex = _Index;
            LOGI("Selected: ", _Label);
        }
        ImVec2 textPos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(imgPos);
        ImGui::Image(reinterpret_cast<ImTextureID>(image->GetTextureId()), imgSize);
        ImGui::SetCursorScreenPos(textPos);
        float lastButtonX2 = ImGui::GetItemRectMax().x;
        float nextButtonX2 = lastButtonX2 + style.ItemSpacing.x + buttonSize.x;
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + buttonSize.x);
        ImGui::TextUnformatted(_Label.data());

        ImGui::EndGroup();
        if (_Index + 1 < _NodesCount && nextButtonX2 < _WindowVisibleX2)
        {
            ImGui::SameLine();
        }
        else
        {
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
        }
        ImGui::PopID();
    }

}    // namespace LM
