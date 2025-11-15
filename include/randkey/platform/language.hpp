#pragma once

#include <string>

namespace randkey::platform
{
    /// @brief 获取当前系统语言（IETF 标准，如 en-US）
    std::string detect_system_language();
}
