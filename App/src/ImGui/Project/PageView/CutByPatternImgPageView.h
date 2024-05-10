#include "IImgPageView.h"

#include "Engine/Utils/utf8.h"

namespace LM
{

    class CutByPatternImgPageView : public IImgPageView
    {
    protected:
        virtual std::string GetBasePath() const override { return m_Project->GetCutByPatternImgsPath(); }
        virtual const char* GetWindowName() const override { return U8("�������� �� ��������"); }

    protected:
    };

}    // namespace LM
