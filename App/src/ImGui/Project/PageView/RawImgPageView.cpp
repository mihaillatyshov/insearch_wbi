#include "RawImgPageView.h"

#include "ImGui/Overlays/Overlay.h"
#include "PageViewManager.h"

#include "Engine/Utils/ConsoleLog.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

namespace LM
{
    constexpr float kPatternSpeed = 0.01f;
    constexpr float kPatternMin = 0.0f;
    constexpr float kPatternMax = 100.0f;

    void RawImgPageView::DrawTopMenuExtras()
    {
        ImVec2 buttonSize = { 0.0f, ImGui::GetFontSize() * PageViewManager::kBntSizeCoef };

        ImGui::SameLine(0.0f, 20.0f);
        if (ImGui::Button(U8("Действия"), buttonSize))
        {
            ImGui::OpenPopup(U8("Меню дейстивий"));
        }

        ImGui::SameLine(0.0f, 20.0f);
        if (m_Project->IsPageInGeneratedCatalogExcludePages(m_PageId))
        {
            if (ImGui::Button(U8("Включить в обработку"), buttonSize))
            {
                m_Project->RemoveGeneratedCatalogExcludePage(m_PageId);
            }
        }
        else
        {
            if (ImGui::Button(U8("Исключить из обработки"), buttonSize))
            {
                m_Project->AddGeneratedCatalogExcludePage(m_PageId);
            }
        }

        if (ImGui::BeginPopup(U8("Меню дейстивий")))
        {
            if (ImGui::Selectable(U8("Задать левый верхний паттерн")))
            {
                m_PatternState = PatternState::kCreateTopLeft;
                const CatalogCutPattern& cutPattern = m_Project->GetCatalogTopLeftPattern();
                m_TmpCatalogCutPattern = m_PageId == cutPattern.PageId ? cutPattern : CatalogCutPattern {};
            }
            if (ImGui::Selectable(U8("Задать правый нижний паттерн")))
            {
                m_PatternState = PatternState::kCreateBotRight;
                const CatalogCutPattern& cutPattern = m_Project->GetCatalogBotRightPattern();
                m_TmpCatalogCutPattern = m_PageId == cutPattern.PageId ? cutPattern : CatalogCutPattern {};
            }

            if (ImGui::Selectable(U8("Просмотр: левый верхний паттерн")))
            {
                m_PatternState = PatternState::kShowTopLeft;
                const CatalogCutPattern& cutPattern = m_Project->GetCatalogTopLeftPattern();
                if (cutPattern.PageId == -1)
                {
                    Overlay::Get()->Start(U8("Паттерн отсутствует"));
                }
                else
                {
                    m_PageId = PageViewManager::Get()->SetPage(cutPattern.PageId);
                    m_TmpCatalogCutPattern = cutPattern;
                }
            }
            if (ImGui::Selectable(U8("Просмотр: правый нижний паттерн")))
            {
                m_PatternState = PatternState::kShowBotRight;
                const CatalogCutPattern& cutPattern = m_Project->GetCatalogBotRightPattern();
                if (cutPattern.PageId == -1)
                {
                    Overlay::Get()->Start(U8("Паттерн отсутствует"));
                }
                else
                {
                    m_PageId = PageViewManager::Get()->SetPage(cutPattern.PageId);
                    m_TmpCatalogCutPattern = cutPattern;
                }
            }

            ImGui::EndPopup();
        }
    }

    void RawImgPageView::DrawExtras()
    {
        if (IsCutPatternCreateState())
        {
            DrawCreateCutPattern();
        }
        if (IsCutPatternShowState())
        {
            DrawShowCutPattern();
        }
    }

    void RawImgPageView::DrawCreateCutPattern()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoMove;

        const ImGuiViewport* viewport = ImGui::GetWindowViewport();

        ImVec2 workPos = ImGui::GetWindowPos();
        ImVec2 windowPos { workPos.x + ImGui::GetWindowWidth() - 40.0f,
                           workPos.y + ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f + 20.0f };
        ImVec2 windowPosPivot { 1.0f, 0.0f };

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
        ImGui::SetNextWindowViewport(viewport->ID);

        if (ImGui::Begin("CutPattern##Overlay", nullptr, windowFlags))
        {
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            ImGui::Text(U8("'/\\' - выбрать мышкой"));
            ImGui::Text(U8("'\\/' - отменить"));

            DrawPatternPointButton(PatternPointState::kFirst, 0);
            ImGui::DragFloat2(U8("Min"), glm::value_ptr(m_TmpCatalogCutPattern.PointMin), kPatternSpeed, kPatternMin,
                              kPatternMax);
            DrawPatternPointButton(PatternPointState::kSecond, 1);
            ImGui::DragFloat2(U8("Max"), glm::value_ptr(m_TmpCatalogCutPattern.PointMax), kPatternSpeed, kPatternMin,
                              kPatternMax);
            DrawPatternPointButton(PatternPointState::kCenter, 2);
            ImGui::DragFloat2(U8("Center"), glm::value_ptr(m_TmpCatalogCutPattern.CenterPoint), kPatternSpeed,
                              kPatternMin, kPatternMax);

            SavePattern();
            ImGui::SameLine();
            CancelPattern();
        }
        ImGui::End();

        glm::vec2 imgSize = (m_ImgEnd - m_ImgStart);

        if (m_PatternPointState != PatternPointState::kNone && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            glm::vec2 mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
            if ((mousePos.x <= m_ImgEnd.x && mousePos.x >= m_ImgStart.x) &&
                (mousePos.y <= m_ImgEnd.y && mousePos.y >= m_ImgStart.y))
            {
                glm::vec2 mousePosInImg = mousePos - m_ImgStart;
                if (m_PatternPointState == PatternPointState::kCenter)
                {
                    m_TmpCatalogCutPattern.CenterPoint =
                        (mousePosInImg - m_TmpCatalogCutPattern.PointMin * imgSize / 100.0f) /
                        ((m_TmpCatalogCutPattern.PointMax - m_TmpCatalogCutPattern.PointMin) * imgSize / 100.0f) *
                        100.0f;
                    LOGW(mousePosInImg.x, " ", mousePosInImg.y, "    ",
                         (m_TmpCatalogCutPattern.PointMin.x / 100 * imgSize.x), " ",
                         (m_TmpCatalogCutPattern.PointMin.y / 100 * imgSize.y) / 100);
                    LOGW(m_TmpCatalogCutPattern.CenterPoint.x, " ", m_TmpCatalogCutPattern.CenterPoint.y);
                }
                else
                {
                    glm::vec2 point = mousePosInImg / imgSize * 100.0f;
                    if (m_PatternPointState == PatternPointState::kFirst)
                    {
                        m_TmpCatalogCutPattern.PointMin = point;
                    }
                    else
                    {
                        m_TmpCatalogCutPattern.PointMax = point;
                    }
                }
                m_PatternPointState = PatternPointState::kNone;
            }
        }
        m_TmpCatalogCutPattern.Fix();

        DrawCutPattern();
    }

    void RawImgPageView::DrawShowCutPattern()
    {
        if (m_TmpCatalogCutPattern.PageId != m_PageId)
        {
            m_PatternState = PatternState::kNone;
            return;
        }

        DrawCutPattern();
    }

    void RawImgPageView::DrawCutPattern()
    {
        glm::vec2 imgSize = (m_ImgEnd - m_ImgStart);

        float circleRadius = GetCircleRadius() / 2.0f;
        float circleRadiusOuter = GetCircleRadius() / 1.5f;

        const ImU32 kColorRed = ImGui::GetColorU32(IM_COL32(255, 0, 0, 255));
        const ImU32 kColorBlue = ImGui::GetColorU32(IM_COL32(0, 0, 255, 255));
        const ImU32 kColorWhite = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        glm::vec2 minPoint = m_ImgStart + m_TmpCatalogCutPattern.PointMin / kPatternMax * imgSize;
        glm::vec2 maxPoint = m_ImgStart + m_TmpCatalogCutPattern.PointMax / kPatternMax * imgSize;
        glm::vec2 centerPoint = minPoint + m_TmpCatalogCutPattern.CenterPoint / kPatternMax * (maxPoint - minPoint);

        drawList->AddCircleFilled({ centerPoint.x, centerPoint.y }, circleRadiusOuter, kColorRed);
        drawList->AddCircleFilled({ centerPoint.x, centerPoint.y }, circleRadius, kColorBlue);

        drawList->AddCircleFilled({ minPoint.x, minPoint.y }, circleRadiusOuter, kColorBlue);
        drawList->AddCircleFilled({ minPoint.x, minPoint.y }, circleRadius, kColorRed);

        drawList->AddCircleFilled({ maxPoint.x, maxPoint.y }, circleRadiusOuter, kColorBlue);
        drawList->AddCircleFilled({ maxPoint.x, maxPoint.y }, circleRadius, kColorRed);
    }

    bool RawImgPageView::IsCutPatternCreateState() const
    {
        return m_PatternState == PatternState::kCreateTopLeft || m_PatternState == PatternState::kCreateBotRight;
    }

    bool RawImgPageView::IsCutPatternShowState() const
    {
        return m_PatternState == PatternState::kShowTopLeft || m_PatternState == PatternState::kShowBotRight;
    }

    void RawImgPageView::SavePattern()
    {
        if (ImGui::Button(U8("Сохранить")))
        {
            m_TmpCatalogCutPattern.PageId = m_PageId;

            if (m_PatternState == PatternState::kCreateTopLeft)
            {
                m_Project->SetCatalogTopLeftPattern(m_TmpCatalogCutPattern);
            }
            else if (m_PatternState == PatternState::kCreateBotRight)
            {
                m_Project->SetCatalogBotRightPattern(m_TmpCatalogCutPattern);
            }

            m_PatternState = PatternState::kNone;
        }
    }

    void RawImgPageView::CancelPattern()
    {
        if (ImGui::Button(U8("Отменить")))
        {
            m_PatternState = PatternState::kNone;
        }
    }

    void RawImgPageView::DrawPatternPointButton(PatternPointState _PointState, int id)
    {
        ImGui::PushID(id);
        if (m_PatternPointState == _PointState)
        {
            if (ImGui::Button("\\/"))
            {
                m_PatternPointState = PatternPointState::kNone;
            }
        }
        else if (ImGui::Button("/\\"))
        {
            m_PatternPointState = _PointState;
        }
        ImGui::PopID();
        ImGui::SameLine();
    }

}    // namespace LM
