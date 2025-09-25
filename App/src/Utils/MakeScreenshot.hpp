#pragma once

#include <string>

namespace LM
{

    bool MakeScreenshot(const std::string& _OutputPath);

    bool MakeScreenshotFromClipboard(const std::string& _OutputPath);

}    // namespace LM
