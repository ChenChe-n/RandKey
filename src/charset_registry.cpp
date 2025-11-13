#include "randkey/charset_registry.hpp"

#include "randkey/encoding.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace randkey
{
    namespace
    {
        constexpr std::u32string_view LOWERCASE = U"abcdefghijklmnopqrstuvwxyz";
        constexpr std::u32string_view UPPERCASE = U"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr std::u32string_view DIGITS = U"0123456789";
        constexpr std::u32string_view SPECIAL = U"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    }

    CharsetRegistry::CharsetRegistry() = default;

    void CharsetRegistry::include(BuiltinCharset kind)
    {
        switch (kind)
        {
        case BuiltinCharset::Lowercase:
            add_characters(LOWERCASE);
            break;
        case BuiltinCharset::Uppercase:
            add_characters(UPPERCASE);
            break;
        case BuiltinCharset::Digits:
            add_characters(DIGITS);
            break;
        case BuiltinCharset::Special:
            add_characters(SPECIAL);
            break;
        }
    }

    void CharsetRegistry::include_all_builtins()
    {
        include(BuiltinCharset::Lowercase);
        include(BuiltinCharset::Uppercase);
        include(BuiltinCharset::Digits);
        include(BuiltinCharset::Special);
    }

    void CharsetRegistry::add_characters(std::u32string_view chars)
    {
        for (char32_t c : chars)
        {
            std::u32string token(1, c);
            append_unique_token(std::move(token));
        }
    }

    void CharsetRegistry::add_token(std::u32string token)
    {
        append_unique_token(std::move(token));
    }

    void CharsetRegistry::add_from_file(const std::filesystem::path &path, bool treat_line_as_token)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("error_charset_file:" + path.string());
        }

        std::string line;
        while (std::getline(file, line))
        {
            auto converted = locale_to_utf32(line);
            if (converted.empty())
            {
                continue;
            }

            if (treat_line_as_token)
            {
                add_token(std::move(converted));
            }
            else
            {
                add_characters(converted);
            }
        }
    }

    void CharsetRegistry::ensure_default()
    {
        if (!tokens_.empty())
        {
            return;
        }

        add_characters(LOWERCASE);
        add_characters(DIGITS);
    }

    std::vector<std::u32string> CharsetRegistry::materialize() const
    {
        return tokens_;
    }

    void CharsetRegistry::append_unique_token(std::u32string token)
    {
        if (token.empty())
        {
            return;
        }

        if (seen_.insert(token).second)
        {
            tokens_.push_back(std::move(token));
        }
    }
}
