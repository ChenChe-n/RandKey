#ifndef PSTRING_HPP
#define PSTRING_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cctype>
#include <locale>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#elif defined(__linux__)
#include <iconv.h>
#include <errno.h>
#include <clocale>
#else
#error "Unsupported platform"
#endif

namespace uconv
{
    /*
        非通用模块
    */
    namespace __module
    {
#if defined(_WIN32) || defined(_WIN64)
        namespace windows
        {
            inline std::u16string __locale_to_utf16(const std::string &str)
            {
                if (str.empty())
                    return std::u16string();

                // 获取所需缓冲区大小
                int size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
                if (size == 0)
                {
                    DWORD error = GetLastError();
                    if (error == ERROR_INVALID_PARAMETER && str.empty())
                    {
                        return std::u16string();
                    }
                    throw std::runtime_error("MultiByteToWideChar failed");
                }

                // 转换为UTF-16
                std::vector<char16_t> buffer(size);
                if (MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1,
                                        reinterpret_cast<LPWSTR>(buffer.data()), size) == 0)
                {
                    throw std::runtime_error("MultiByteToWideChar conversion failed");
                }

                // 移除末尾的null字符
                if (size > 0 && buffer[size - 1] == 0)
                {
                    buffer.resize(size - 1);
                }
                return std::u16string(buffer.data(), buffer.size());
            }

            inline std::string __utf16_to_locale(const std::u16string &str)
            {
                if (str.empty())
                    return std::string();

                // 获取所需缓冲区大小
                int size = WideCharToMultiByte(CP_ACP, 0,
                                               reinterpret_cast<LPCWSTR>(str.c_str()), -1,
                                               nullptr, 0, nullptr, nullptr);
                if (size == 0)
                {
                    DWORD error = GetLastError();
                    if (error == ERROR_INVALID_PARAMETER && str.empty())
                    {
                        return std::string();
                    }
                    throw std::runtime_error("WideCharToMultiByte failed");
                }

                // 转换为本地编码
                std::vector<char> buffer(size);
                if (WideCharToMultiByte(CP_ACP, 0,
                                        reinterpret_cast<LPCWSTR>(str.c_str()), -1,
                                        buffer.data(), size, nullptr, nullptr) == 0)
                {
                    throw std::runtime_error("WideCharToMultiByte conversion failed");
                }

                // 移除末尾的null字符
                if (size > 0 && buffer[size - 1] == 0)
                {
                    buffer.resize(size - 1);
                }
                return std::string(buffer.data(), buffer.size());
            }
        }
#endif
#if defined(__linux__)
        namespace linux
        {
            // 获取当前系统的locale编码
            inline std::string get_system_locale_encoding()
            {
                const char *locale = std::setlocale(LC_CTYPE, nullptr);
                if (!locale)
                {
                    return "UTF-8"; // 默认回退到UTF-8
                }

                std::string loc(locale);
                size_t dot_pos = loc.find('.');
                if (dot_pos != std::string::npos && dot_pos + 1 < loc.size())
                {
                    return loc.substr(dot_pos + 1);
                }
                return "UTF-8"; // 默认回退到UTF-8
            }

            // 使用iconv进行编码转换的通用函数
            inline std::string convert_encoding(const std::string &input,
                                                const char *from_encoding,
                                                const char *to_encoding)
            {
                iconv_t cd = iconv_open(to_encoding, from_encoding);
                if (cd == (iconv_t)-1)
                {
                    throw std::runtime_error("iconv_open failed");
                }

                // 分配足够大的输出缓冲区(输入长度的4倍，确保足够空间)
                size_t in_bytes = input.size();
                size_t out_bytes = in_bytes * 4;
                std::vector<char> buffer(out_bytes);

                char *in_ptr = const_cast<char *>(input.data());
                char *out_ptr = buffer.data();
                size_t out_bytes_left = out_bytes;

                // 执行转换
                if (iconv(cd, &in_ptr, &in_bytes, &out_ptr, &out_bytes_left) == (size_t)-1)
                {
                    iconv_close(cd);
                    throw std::runtime_error("iconv conversion failed");
                }

                iconv_close(cd);

                // 计算实际转换后的长度
                size_t converted_size = out_bytes - out_bytes_left;
                return std::string(buffer.data(), converted_size);
            }

            // 本地编码转UTF-8
            inline std::u8string __locale_to_utf8(const std::string &str)
            {
                if (str.empty())
                    return std::u8string();

                std::string locale_enc = get_system_locale_encoding();
                if (locale_enc == "UTF-8")
                {
                    return std::u8string(reinterpret_cast<const char8_t *>(str.data()), str.size());
                }

                // 非UTF-8本地编码需要转换
                std::string utf8_str = convert_encoding(str, locale_enc.c_str(), "UTF-8");
                return std::u8string(reinterpret_cast<const char8_t *>(utf8_str.data()), utf8_str.size());
            }

            // UTF-8转本地编码
            inline std::string __utf8_to_locale(const std::u8string &str)
            {
                if (str.empty())
                    return std::string();

                std::string locale_enc = get_system_locale_encoding();
                if (locale_enc == "UTF-8")
                {
                    return std::string(reinterpret_cast<const char *>(str.data()), str.size());
                }

                // 将u8string转为普通string便于处理
                std::string utf8_str(reinterpret_cast<const char *>(str.data()), str.size());
                return convert_encoding(utf8_str, "UTF-8", locale_enc.c_str());
            }
        }
#endif
    }

    // UTF-32到UTF-8转换
    inline std::u8string utf32_to_utf8(const std::u32string &str)
    {
        std::u8string result;

        for (char32_t c : str)
        {
            if (c <= 0x7F)
            {
                // 1字节序列
                result.push_back(static_cast<char8_t>(c));
            }
            else if (c <= 0x7FF)
            {
                // 2字节序列
                result.push_back(static_cast<char8_t>(0xC0 | ((c >> 6) & 0x1F)));
                result.push_back(static_cast<char8_t>(0x80 | (c & 0x3F)));
            }
            else if (c <= 0xFFFF)
            {
                // 检查是否在代理项范围内
                if (c >= 0xD800 && c <= 0xDFFF)
                {
                    throw std::runtime_error("Invalid UTF-32 character (surrogate half)");
                }

                // 3字节序列
                result.push_back(static_cast<char8_t>(0xE0 | ((c >> 12) & 0x0F)));
                result.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & 0x3F)));
                result.push_back(static_cast<char8_t>(0x80 | (c & 0x3F)));
            }
            else if (c <= 0x10FFFF)
            {
                // 4字节序列
                result.push_back(static_cast<char8_t>(0xF0 | ((c >> 18) & 0x07)));
                result.push_back(static_cast<char8_t>(0x80 | ((c >> 12) & 0x3F)));
                result.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & 0x3F)));
                result.push_back(static_cast<char8_t>(0x80 | (c & 0x3F)));
            }
            else
            {
                throw std::runtime_error("Invalid UTF-32 character");
            }
        }

        return result;
    }

    // UTF-32到UTF-16转换
    inline std::u16string utf32_to_utf16(const std::u32string &str)
    {
        std::u16string result;

        for (char32_t c : str)
        {
            if (c <= 0xFFFF)
            {
                // 基本多文种平面（BMP）
                if (c >= 0xD800 && c <= 0xDFFF)
                {
                    throw std::runtime_error("Invalid UTF-32 character (surrogate half)");
                }
                result.push_back(static_cast<char16_t>(c));
            }
            else if (c <= 0x10FFFF)
            {
                // 辅助平面 - 使用代理对
                c -= 0x10000;
                char16_t high = static_cast<char16_t>(0xD800 | ((c >> 10) & 0x3FF));
                char16_t low = static_cast<char16_t>(0xDC00 | (c & 0x3FF));
                result.push_back(high);
                result.push_back(low);
            }
            else
            {
                throw std::runtime_error("Invalid UTF-32 character");
            }
        }

        return result;
    }

    // UTF-8到UTF-32转换
    inline std::u32string utf8_to_utf32(const std::u8string &str)
    {
        std::u32string result;
        size_t i = 0;
        size_t len = str.size();

        while (i < len)
        {
            char8_t c = str[i++];
            char32_t code_point = 0;
            size_t remaining = 0;

            if ((c & 0x80) == 0)
            {
                // 1字节序列
                code_point = c;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                // 2字节序列
                remaining = 1;
                if (i + remaining > len)
                    throw std::runtime_error("Invalid UTF-8 sequence");
                code_point = (c & 0x1F) << 6;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                // 3字节序列
                remaining = 2;
                if (i + remaining > len)
                    throw std::runtime_error("Invalid UTF-8 sequence");
                code_point = (c & 0x0F) << 12;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                // 4字节序列
                remaining = 3;
                if (i + remaining > len)
                    throw std::runtime_error("Invalid UTF-8 sequence");
                code_point = (c & 0x07) << 18;
            }
            else
            {
                throw std::runtime_error("Invalid UTF-8 sequence");
            }

            // 处理后续字节
            for (size_t j = 0; j < remaining; j++)
            {
                if ((str[i] & 0xC0) != 0x80)
                {
                    throw std::runtime_error("Invalid UTF-8 continuation byte");
                }
                code_point |= (str[i] & 0x3F) << (6 * (remaining - 1 - j));
                i++;
            }

            // 检查码点有效性
            if (code_point > 0x10FFFF ||
                (code_point >= 0xD800 && code_point <= 0xDFFF))
            {
                throw std::runtime_error("Invalid Unicode code point");
            }

            result.push_back(code_point);
        }

        return result;
    }

    // UTF-16到UTF-32转换
    inline std::u32string utf16_to_utf32(const std::u16string &str)
    {
        std::u32string result;
        size_t i = 0;
        size_t len = str.size();

        while (i < len)
        {
            char16_t high = str[i++];

            if (high >= 0xD800 && high <= 0xDBFF)
            {
                // 高代理项 - 期待低代理项
                if (i >= len)
                    throw std::runtime_error("Invalid UTF-16 sequence (missing low surrogate)");

                char16_t low = str[i++];
                if (low < 0xDC00 || low > 0xDFFF)
                {
                    throw std::runtime_error("Invalid UTF-16 sequence (invalid low surrogate)");
                }

                // 计算辅助平面码点
                char32_t code_point = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00);
                result.push_back(code_point);
            }
            else if (high >= 0xDC00 && high <= 0xDFFF)
            {
                // 意外的低代理项
                throw std::runtime_error("Invalid UTF-16 sequence (unexpected low surrogate)");
            }
            else
            {
                // 基本多文种平面（BMP）
                result.push_back(high);
            }
        }

        return result;
    }

    inline std::u32string locale_to_utf32(const std::string &str)
    {
#if defined(_WIN32) || defined(_WIN64)
        auto result = __module::windows::__locale_to_utf16(str);
        return utf16_to_utf32(result);
#elif defined(__linux__)
        auto result = __module::linux::__locale_to_utf8(str);
        return utf8_to_utf32(result);
#endif
    }

    inline std::string utf32_to_locale(const std::u32string &str)
    {
#if defined(_WIN32) || defined(_WIN64)
        auto result = utf32_to_utf16(str);
        return __module::windows::__utf16_to_locale(result);
#elif defined(__linux__)
        auto result = utf32_to_utf8(str);
        return __module::linux::__utf8_to_locale(result);
#endif
    }
}
#endif