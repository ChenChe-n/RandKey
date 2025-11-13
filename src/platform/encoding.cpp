#include "randkey/platform/encoding.hpp"

#include <codecvt>
#include <locale>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <iconv.h>
#include <clocale>
#include <cstring>
#include <errno.h>
#endif

namespace randkey::platform
{
    namespace
    {
        std::u32string utf8_to_utf32_fallback(std::string_view input)
        {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            try
            {
                return converter.from_bytes(input.data(), input.data() + input.size());
            }
            catch (const std::range_error &)
            {
                throw std::runtime_error("UTF-8 到 UTF-32 转换失败");
            }
        }

        std::string utf32_to_utf8_fallback(std::u32string_view input)
        {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            try
            {
                return converter.to_bytes(input.data(), input.data() + input.size());
            }
            catch (const std::range_error &)
            {
                throw std::runtime_error("UTF-32 到 UTF-8 转换失败");
            }
        }

#if defined(__unix__) || defined(__APPLE__)
        std::string get_locale_encoding()
        {
            const char *locale = std::setlocale(LC_CTYPE, nullptr);
            if (!locale)
            {
                return "UTF-8";
            }

            std::string loc(locale);
            const auto dot_pos = loc.find('.');
            if (dot_pos != std::string::npos && dot_pos + 1 < loc.size())
            {
                return loc.substr(dot_pos + 1);
            }
            return "UTF-8";
        }

        std::string convert_encoding(const std::string &input,
                                     const char *from_encoding,
                                     const char *to_encoding)
        {
            iconv_t cd = iconv_open(to_encoding, from_encoding);
            if (cd == (iconv_t)-1)
            {
                throw std::runtime_error("iconv_open 失败");
            }

            std::size_t in_bytes = input.size();
            std::size_t out_bytes = in_bytes * 4 + 16;
            std::vector<char> buffer(out_bytes);

            char *in_ptr = const_cast<char *>(input.data());
            char *out_ptr = buffer.data();
            std::size_t out_left = out_bytes;

            while (in_bytes > 0)
            {
                if (iconv(cd, &in_ptr, &in_bytes, &out_ptr, &out_left) == (std::size_t)-1)
                {
                    if (errno == EILSEQ || errno == EINVAL)
                    {
                        iconv_close(cd);
                        throw std::runtime_error("字符编码转换失败");
                    }
                    if (errno == E2BIG)
                    {
                        const std::size_t used = out_ptr - buffer.data();
                        buffer.resize(buffer.size() * 2);
                        out_ptr = buffer.data() + used;
                        out_left = buffer.size() - used;
                        continue;
                    }
                    iconv_close(cd);
                    throw std::runtime_error("iconv 运行时错误");
                }
            }

            iconv_close(cd);

            return std::string(buffer.data(), buffer.size() - out_left);
        }
#endif
    }

    std::u32string utf8_to_utf32(std::string_view input)
    {
        return utf8_to_utf32_fallback(input);
    }

    std::string utf32_to_utf8(std::u32string_view input)
    {
        return utf32_to_utf8_fallback(input);
    }

    std::u32string locale_to_utf32(std::string_view input)
    {
        if (input.empty())
        {
            return {};
        }

#if defined(_WIN32)
        const int wide_size = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
                                                  input.data(), static_cast<int>(input.size()), nullptr, 0);
        if (wide_size <= 0)
        {
            throw std::runtime_error("本地编码到 UTF-16 转换失败");
        }

        std::wstring wide(static_cast<std::size_t>(wide_size), L'\0');
        if (MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
                                input.data(), static_cast<int>(input.size()), wide.data(), wide_size) <= 0)
        {
            throw std::runtime_error("本地编码到 UTF-16 转换失败");
        }

        const int utf8_size = WideCharToMultiByte(CP_UTF8, 0,
                                                  wide.c_str(), wide_size, nullptr, 0, nullptr, nullptr);
        if (utf8_size <= 0)
        {
            throw std::runtime_error("UTF-16 到 UTF-8 转换失败");
        }

        std::string utf8(static_cast<std::size_t>(utf8_size), '\0');
        if (WideCharToMultiByte(CP_UTF8, 0,
                                wide.c_str(), wide_size, utf8.data(), utf8_size, nullptr, nullptr) <= 0)
        {
            throw std::runtime_error("UTF-16 到 UTF-8 转换失败");
        }

        return utf8_to_utf32_fallback(utf8);
#elif defined(__unix__) || defined(__APPLE__)
        std::string locale_enc = get_locale_encoding();
        std::string source(input);
        if (locale_enc == "UTF-8")
        {
            return utf8_to_utf32_fallback(source);
        }

        std::string utf8 = convert_encoding(source, locale_enc.c_str(), "UTF-8");
        return utf8_to_utf32_fallback(utf8);
#else
        return utf8_to_utf32_fallback(std::string(input));
#endif
    }

    std::string utf32_to_locale(std::u32string_view input)
    {
        if (input.empty())
        {
            return {};
        }

#if defined(_WIN32)
        std::string utf8 = utf32_to_utf8_fallback(input);

        const int wide_size = MultiByteToWideChar(CP_UTF8, 0,
                                                  utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
        if (wide_size <= 0)
        {
            throw std::runtime_error("UTF-8 到 UTF-16 转换失败");
        }

        std::wstring wide(static_cast<std::size_t>(wide_size), L'\0');
        if (MultiByteToWideChar(CP_UTF8, 0,
                                utf8.data(), static_cast<int>(utf8.size()), wide.data(), wide_size) <= 0)
        {
            throw std::runtime_error("UTF-8 到 UTF-16 转换失败");
        }

        const int locale_size = WideCharToMultiByte(CP_ACP, 0,
                                                    wide.c_str(), wide_size, nullptr, 0, nullptr, nullptr);
        if (locale_size <= 0)
        {
            throw std::runtime_error("UTF-16 到本地编码转换失败");
        }

        std::string local(static_cast<std::size_t>(locale_size), '\0');
        if (WideCharToMultiByte(CP_ACP, 0,
                                wide.c_str(), wide_size, local.data(), locale_size, nullptr, nullptr) <= 0)
        {
            throw std::runtime_error("UTF-16 到本地编码转换失败");
        }

        return local;
#elif defined(__unix__) || defined(__APPLE__)
        std::string utf8 = utf32_to_utf8_fallback(input);
        std::string locale_enc = get_locale_encoding();
        if (locale_enc == "UTF-8")
        {
            return utf8;
        }

        return convert_encoding(utf8, "UTF-8", locale_enc.c_str());
#else
        return utf32_to_utf8_fallback(input);
#endif
    }
}
