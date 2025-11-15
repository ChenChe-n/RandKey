#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace randkey
{
    std::u32string utf8_to_utf32(std::string_view input);
    std::string utf32_to_utf8(std::u32string_view input);

    std::u32string locale_to_utf32(std::string_view input);
    std::string utf32_to_locale(std::u32string_view input);
}
