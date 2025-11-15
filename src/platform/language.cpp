#include "randkey/platform/language.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <locale>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFLocale.h>
#elif defined(__linux__) || defined(__unix__) || defined(__ANDROID__)
#include <cstring>
#endif

namespace randkey::platform
{
    namespace
    {
        std::string normalize_language(std::string value)
        {
            std::replace(value.begin(), value.end(), '_', '-');
            auto dash_pos = value.find('-');
            if (dash_pos != std::string::npos)
            {
                std::transform(value.begin(), value.begin() + dash_pos, value.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                std::transform(value.begin() + dash_pos + 1, value.end(), value.begin() + dash_pos + 1, [](unsigned char c) {
                    return static_cast<char>(std::toupper(c));
                });
            }
            return value;
        }
    }

    std::string detect_system_language()
    {
        const std::string fallback = "en-US";

#if defined(_WIN32)
        wchar_t buffer[LOCALE_NAME_MAX_LENGTH] = {0};
        if (GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH) == 0)
        {
            return fallback;
        }

        int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0)
        {
            return fallback;
        }

        std::string result(static_cast<std::size_t>(size), '\0');
        int written = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, result.data(), size, nullptr, nullptr);
        if (written <= 0)
        {
            return fallback;
        }
        result.resize(static_cast<std::size_t>(written) - 1);
        return normalize_language(result);
#elif defined(__APPLE__)
        CFLocaleRef locale = CFLocaleCopyCurrent();
        if (!locale)
        {
            return fallback;
        }

        CFStringRef identifier = CFLocaleGetIdentifier(locale);
        CFIndex max_size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(identifier), kCFStringEncodingUTF8) + 1;
        std::string result(static_cast<std::size_t>(max_size), '\0');
        if (!CFStringGetCString(identifier, result.data(), max_size, kCFStringEncodingUTF8))
        {
            CFRelease(locale);
            return fallback;
        }
        CFRelease(locale);
        result.erase(result.find_first_of('\0'));
        return normalize_language(result);
#elif defined(__linux__) || defined(__unix__) || defined(__ANDROID__)
        const char *lang = std::getenv("LC_ALL");
        if (!lang || lang[0] == '\0')
        {
            lang = std::getenv("LC_MESSAGES");
        }
        if (!lang || lang[0] == '\0')
        {
            lang = std::getenv("LANG");
        }
        if (!lang || lang[0] == '\0')
        {
            return fallback;
        }

        std::string value = lang;
        if (value == "C" || value == "POSIX")
        {
            return fallback;
        }

        auto dot_pos = value.find('.');
        if (dot_pos != std::string::npos)
        {
            value.erase(dot_pos);
        }
        auto at_pos = value.find('@');
        if (at_pos != std::string::npos)
        {
            value.erase(at_pos);
        }

        return normalize_language(value);
#else
        return fallback;
#endif
    }
}
