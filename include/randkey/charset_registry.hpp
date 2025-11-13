#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace randkey
{
    enum class BuiltinCharset
    {
        Lowercase,
        Uppercase,
        Digits,
        Special,
    };

    class CharsetRegistry
    {
    public:
        CharsetRegistry();

        void include(BuiltinCharset kind);
        void include_all_builtins();
        void add_characters(std::u32string_view chars);
        void add_token(std::u32string token);
        void add_from_file(const std::filesystem::path &path, bool treat_line_as_token = false);

        /// @brief 如果当前集合为空则填充默认字符集（小写+数字）
        void ensure_default();

        std::vector<std::u32string> materialize() const;

    private:
        void append_unique_token(std::u32string token);

        std::vector<std::u32string> tokens_;
        std::unordered_set<std::u32string> seen_;
    };
}
