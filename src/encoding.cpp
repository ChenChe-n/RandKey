#include "randkey/encoding.hpp"

#include "randkey/platform/encoding.hpp"

namespace randkey
{
    std::u32string utf8_to_utf32(std::string_view input)
    {
        return platform::utf8_to_utf32(input);
    }

    std::string utf32_to_utf8(std::u32string_view input)
    {
        return platform::utf32_to_utf8(input);
    }

    std::u32string locale_to_utf32(std::string_view input)
    {
        return platform::locale_to_utf32(input);
    }

    std::string utf32_to_locale(std::u32string_view input)
    {
        return platform::utf32_to_locale(input);
    }
}
