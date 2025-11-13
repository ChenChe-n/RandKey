#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace randkey::i18n
{
    class Catalog
    {
    public:
        Catalog();

        /// @brief 获取给定键的翻译，若找不到则返回回退语言或默认文本
        std::string_view translate(std::string_view language, std::string_view key) const;

        /// @brief 设置回退语言，默认为 en-US
        void set_fallback(std::string language);

        /// @brief 注册某个语言下的键值对
        void insert(std::string language, std::unordered_map<std::string, std::string> entries);

    private:
        std::string fallback_;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_;
    };
}
