#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>
#include <locale>
#include <codecvt>
#include <clocale>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__linux__) || defined(__unix__)
#include <langinfo.h>
#endif

// 获取系统语言
std::string get_lang()
{
    static const std::map<std::string, std::string> lang_table = {
        // Windows LCID → BCP 47
        {"0409", "en-us"},
        {"0804", "zh-cn"},
        {"0404", "zh-tw"},
        {"0411", "ja-jp"},

        // macOS/CFLocale → BCP 47
        {"zh-hans-cn", "zh-cn"},
        {"zh-hant-tw", "zh-tw"},
        {"en-us", "en-us"},
        {"ja-jp", "ja-jp"},

        // Linux/Unix → BCP 47
        {"zh_cn.utf-8", "zh-cn"},
        {"zh_tw.utf-8", "zh-tw"},
        {"en_us.utf-8", "en-us"},
        {"ja_jp.utf-8", "ja-jp"},
        {"c", "en-us"} // 默认C区域
    };

    std::string raw_lang;

#if defined(_WIN32) || defined(_WIN64)
    LANGID langID = GetUserDefaultUILanguage();
    char lang_code[5] = {0};
    snprintf(lang_code, sizeof(lang_code), "%04X", langID);
    raw_lang = lang_code;

#elif defined(__APPLE__)
    CFArrayRef preferredLangs = CFLocaleCopyPreferredLanguages();
    if (preferredLangs && CFArrayGetCount(preferredLangs) > 0)
    {
        CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(preferredLangs, 0);
        char buffer[32] = {0};
        if (CFStringGetCString(lang, buffer, sizeof(buffer), kCFStringEncodingUTF8))
        {
            raw_lang = buffer;
        }
        CFRelease(preferredLangs);
    }

#elif defined(__linux__) || defined(__unix__)
    setlocale(LC_ALL, "");
    const char *lang_c = setlocale(LC_CTYPE, nullptr);
    if (lang_c)
    {
        raw_lang = lang_c;
    }
#endif

    // 转换为小写
    std::transform(raw_lang.begin(), raw_lang.end(), raw_lang.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    // 查找映射表
    auto it = lang_table.find(raw_lang);
    if (it != lang_table.end())
    {
        return it->second;
    }

    // 尝试提取前2个字符
    if (raw_lang.size() >= 2)
    {
        std::string lang_short = raw_lang.substr(0, 2);
        if (lang_short == "zh")
        {
            // 中文简体和繁体的后备
            if (raw_lang.find("tw") != std::string::npos || raw_lang.find("hant") != std::string::npos)
            {
                return "zh-tw";
            }
            return "zh-cn";
        }
        return lang_short;
    }

    return "en-us";
}

#include <string>
#include <codecvt>
#include <locale>
#include <stdexcept>

#include <string>
#include <cstdint>

// UTF-8 到 UTF-32 转换（无检查）
std::u32string utf8_to_utf32(const std::u8string &utf8_str)
{
    std::u32string result;
    result.reserve(utf8_str.size()); // 最小预分配

    for (size_t i = 0; i < utf8_str.size();)
    {
        uint8_t c = utf8_str[i];
        size_t len = 0;
        char32_t code_point = 0;

        // 确定码点长度
        if (c < 0x80)
        {
            len = 1;
            code_point = c;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            len = 2;
            code_point = c & 0x1F;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            len = 3;
            code_point = c & 0x0F;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            len = 4;
            code_point = c & 0x07;
        }
        else
        {
            // 无效的UTF-8首字节，按单字节处理
            len = 1;
            code_point = c;
        }

        // 处理后续字节
        for (size_t j = 1; j < len && (i + j) < utf8_str.size(); ++j)
        {
            code_point = (code_point << 6) | (utf8_str[i + j] & 0x3F);
        }

        result.push_back(code_point);
        i += (len > 0) ? len : 1;
    }

    return result;
}

// UTF-32 到 UTF-8 转换（无检查）
std::u8string utf32_to_utf8(const std::u32string &u32_str)
{
    std::u8string result;
    result.reserve(u32_str.size() * 4); // 最大预分配

    for (char32_t code_point : u32_str)
    {
        // 转换为UTF-8
        if (code_point < 0x80)
        {
            result.push_back(static_cast<char8_t>(code_point));
        }
        else if (code_point < 0x800)
        {
            result.push_back(static_cast<char8_t>(0xC0 | (code_point >> 6)));
            result.push_back(static_cast<char8_t>(0x80 | (code_point & 0x3F)));
        }
        else if (code_point < 0x10000)
        {
            result.push_back(static_cast<char8_t>(0xE0 | (code_point >> 12)));
            result.push_back(static_cast<char8_t>(0x80 | ((code_point >> 6) & 0x3F)));
            result.push_back(static_cast<char8_t>(0x80 | (code_point & 0x3F)));
        }
        else
        {
            result.push_back(static_cast<char8_t>(0xF0 | (code_point >> 18)));
            result.push_back(static_cast<char8_t>(0x80 | ((code_point >> 12) & 0x3F)));
            result.push_back(static_cast<char8_t>(0x80 | ((code_point >> 6) & 0x3F)));
            result.push_back(static_cast<char8_t>(0x80 | (code_point & 0x3F)));
        }
    }

    return result;
}

// 参数定义表
static const std::map<std::u32string, int32_t> arg_table = {
    {U"-help", 0},
    {U"-h", 0},
    {U"-S", 1},
    {U"-s", 2},
    {U"-l", 3},
    {U"-c", 4},
    {U"-all", 100},
    {U"-aa", 101},
    {U"-aA", 102},
    {U"-a0", 103},
    {U"-a!", 104},
    {U"-ai", 105}};

// 多语言字符串表
static const std::map<std::string, std::map<std::u32string, std::u32string>> lang_table = {
    {"en-us",
     {{U"help_title", U"Random String Generator - Help"},
      {U"help_usage", U"Usage: randstr [options]"},
      {U"help_options", U"Options:\n"
                        U"  -h, -help      Show this help message\n"
                        U"  -S <seed>      Set random seed (64-bit integer)\n"
                        U"  -s <seed>      Set seed with hardware entropy\n"
                        U"  -l <length>    Set string length (default: 12)\n"
                        U"  -c <count>     Set number of strings (default: 1)\n"
                        U"  -all           Use all character sets\n"
                        U"  -aa            Add lowercase letters (a-z)\n"
                        U"  -aA            Add uppercase letters (A-Z)\n"
                        U"  -a0            Add digits (0-9)\n"
                        U"  -a!            Add special characters\n"
                        U"  -ai <chars>    Add custom characters"},
      {U"error_seed", U"Error: Seed must be a 64-bit integer"},
      {U"error_length", U"Error: Length must be a positive integer"},
      {U"error_count", U"Error: Count must be a positive integer"},
      {U"error_no_chars", U"Error: No character set specified"}}},
    {"zh-cn",
     {{U"help_title", U"随机字符串生成器 - 帮助"},
      {U"help_usage", U"用法: randstr [选项]"},
      {U"help_options", U"选项:\n"
                        U"  -h, -help      显示帮助信息\n"
                        U"  -S <种子>      设置随机数种子 (64位整数)\n"
                        U"  -s <种子>      设置种子并添加硬件熵\n"
                        U"  -l <长度>      设置字符串长度 (默认: 12)\n"
                        U"  -c <数量>      设置生成字符串数量 (默认: 1)\n"
                        U"  -all           使用所有字符集\n"
                        U"  -aa            添加小写字母 (a-z)\n"
                        U"  -aA            添加大写字母 (A-Z)\n"
                        U"  -a0            添加数字 (0-9)\n"
                        U"  -a!            添加特殊字符\n"
                        U"  -ai <字符>     添加自定义字符"},
      {U"error_seed", U"错误: 种子必须是64位整数"},
      {U"error_length", U"错误: 长度必须是正整数"},
      {U"error_count", U"错误: 数量必须是正整数"},
      {U"error_no_chars", U"错误: 未指定字符集"}}},
    {"zh-tw",
     {{U"help_title", U"隨機字串生成器 - 幫助"},
      {U"help_usage", U"用法: randstr [選項]"},
      {U"help_options", U"選項:\n"
                        U"  -h, -help      顯示幫助訊息\n"
                        U"  -S <種子>      設置隨機種子 (64位整數)\n"
                        U"  -s <種子>      設置種子並添加硬體熵\n"
                        U"  -l <長度>      設置字串長度 (預設: 12)\n"
                        U"  -c <數量>      設置生成字串數量 (預設: 1)\n"
                        U"  -all           使用所有字符集\n"
                        U"  -aa            添加小寫字母 (a-z)\n"
                        U"  -aA            添加大寫字母 (A-Z)\n"
                        U"  -a0            添加數字 (0-9)\n"
                        U"  -a!            添加特殊字符\n"
                        U"  -ai <字符>     添加自定義字符"},
      {U"error_seed", U"錯誤: 種子必須是64位整數"},
      {U"error_length", U"錯誤: 長度必須是正整數"},
      {U"error_count", U"錯誤: 數量必須是正整數"},
      {U"error_no_chars", U"錯誤: 未指定字符集"}}},
    {"ja-jp",
     {{U"help_title", U"ランダム文字列生成器 - ヘルプ"},
      {U"help_usage", U"使用法: randstr [オプション]"},
      {U"help_options", U"オプション:\n"
                        U"  -h, -help      ヘルプを表示\n"
                        U"  -S <シード>    乱数シードを設定 (64ビット整数)\n"
                        U"  -s <シード>    シードにハードウェアエントロピーを追加\n"
                        U"  -l <長さ>      文字列の長さを設定 (デフォルト: 12)\n"
                        U"  -c <数>        生成する文字列の数を設定 (デフォルト: 1)\n"
                        U"  -all           すべての文字セットを使用\n"
                        U"  -aa            小文字 (a-z) を追加\n"
                        U"  -aA            大文字 (A-Z) を追加\n"
                        U"  -a0            数字 (0-9) を追加\n"
                        U"  -a!            特殊文字を追加\n"
                        U"  -ai <文字>     カスタム文字を追加"},
      {U"error_seed", U"エラー: シードは64ビット整数である必要があります"},
      {U"error_length", U"エラー: 長さは正の整数である必要があります"},
      {U"error_count", U"エラー: 数は正の整数である必要があります"},
      {U"error_no_chars", U"エラー: 文字セットが指定されていません"}}}};

// 获取本地化字符串
std::u32string get_lang_string(const std::string &lang, const std::u32string &key)
{
    auto lang_iter = lang_table.find(lang);
    if (lang_iter != lang_table.end())
    {
        const auto &lang_map = lang_iter->second;
        auto str_iter = lang_map.find(key);
        if (str_iter != lang_map.end())
        {
            return str_iter->second;
        }
    }

    // 后备到英语
    auto en_iter = lang_table.find("en-us");
    if (en_iter != lang_table.end())
    {
        const auto &en_map = en_iter->second;
        auto str_iter = en_map.find(key);
        if (str_iter != en_map.end())
        {
            return str_iter->second;
        }
    }

    return U"[" + key + U"]";
}

// 解析64位整数
bool parse_uint64(const std::u32string &str, uint64_t &value)
{
    if (str.empty())
        return false;

    for (char32_t c : str)
    {
        if (c < U'0' || c > U'9')
        {
            return false;
        }
    }

    uint64_t sum = 0;
    for (char32_t c : str)
    {
        if (sum * 10 + (c - U'0') < sum)
        {
            return false;
        }
        sum = sum * 10 + (c - U'0');
    }
    value = sum;
    return true;
}

// 本地编码转 UTF-8
std::u8string local_to_utf8(const std::string &str)
{
    if (str.empty())
    {
        return std::u8string();
    }
#if defined(_WIN32) || defined(_WIN64)
    // 检查是否启用了全局 UTF-8（代码页 65001）
    bool UTF8_ENABLED = (GetACP() == 65001);

    if (UTF8_ENABLED)
    {
        // 如果系统编码为 UTF-8，直接返回
        return std::u8string(str.begin(), str.end());
    }
    else
    {
        // 获取终端代码页（例如 CP936 是 GBK）
        UINT codePage = GetConsoleOutputCP();

        // 多字节转宽字符
        int wideLen = MultiByteToWideChar(codePage, 0, str.c_str(), -1, nullptr, 0);
        std::wstring wideStr(wideLen, 0);
        MultiByteToWideChar(codePage, 0, str.c_str(), -1, wideStr.data(), wideLen);

        // 宽字符转 UTF-8
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string utf8Str(utf8Len, 0);
        WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, utf8Str.data(), utf8Len, nullptr, nullptr);

        return std::u8string(reinterpret_cast<const char8_t *>(utf8Str.c_str()));
    }
#else
    // 非 Windows 平台，假设本地编码为 UTF-8
    return std::u8string(str.begin(), str.end());
#endif
}

// UTF-8 转本地编码
std::string utf8_to_local(const std::u8string &str)
{
    if (str.empty())
    {
        return std::string();
    }
#if defined(_WIN32) || defined(_WIN64)
    // 检查是否启用了全局 UTF-8（代码页 65001）
    bool UTF8_ENABLED = (GetACP() == 65001);

    if (UTF8_ENABLED)
    {
        // 如果系统编码为 UTF-8，直接返回
        return std::string(str.begin(), str.end());
    }
    else
    {
        // 获取终端代码页（如 CP936 是 GBK）
        UINT codePage = GetConsoleOutputCP();

        // UTF-8 转宽字符
        const char *utf8CStr = reinterpret_cast<const char *>(str.c_str());
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8CStr, -1, nullptr, 0);
        std::wstring wideStr(wideLen, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8CStr, -1, wideStr.data(), wideLen);

        // 宽字符转终端编码
        int localLen = WideCharToMultiByte(codePage, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string localStr(localLen, 0);
        WideCharToMultiByte(codePage, 0, wideStr.c_str(), -1, localStr.data(), localLen, nullptr, nullptr);

        return localStr;
    }
#else
    // 非 Windows 平台，假设本地编码为 UTF-8
    return str;
#endif
}

// 生成随机字符串
std::u32string generate_random_string(const std::u32string &char_set,
                                      size_t length,
                                      uint64_t seed,
                                      bool use_hw_entropy)
{
    if (char_set.empty())
    {
        return U"";
    }

    std::uniform_int_distribution<size_t> dist(0, char_set.size() - 1);
    std::u32string result;
    result.reserve(length);
    if (use_hw_entropy)
    {

        for (size_t i = 0; i < length; ++i)
        {
            // 每一位字符重新生成密钥，增加安全性
            std::mt19937_64 rng(seed + std::random_device()());
            result.push_back(char_set[dist(rng)]);
        }

        return result;
    }
    else
    {
        std::mt19937_64 rng(seed);

        for (size_t i = 0; i < length; ++i)
        {
            result.push_back(char_set[dist(rng)]);
        }

        return result;
    }
    return U"";
}

int main(int argc, const char *argv[])
{
    // 设置控制台为UTF-8
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::string lang = get_lang(); // 获取系统语言
    std::u32string char_set;       // 字符集

    // 默认参数
    uint64_t seed = 0;          // 种子
    size_t length = 12;         // 生成长度
    size_t count = 3;           // 生成数量
    bool use_hw_entropy = true; // 使用硬件熵
    bool show_help = false;     // 显示帮助

    // 转换参数为UTF-32
    std::vector<std::u32string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(utf8_to_utf32(local_to_utf8(argv[i])));
    }

    // 解析参数
    for (size_t i = 1; i < args.size(); i++)
    {
        const auto &arg = args[i];
        auto it = arg_table.find(arg);

        if (it != arg_table.end())
        {
            switch (it->second)
            {
            case 0: // -help, -h
                show_help = true;
                break;

            case 1: // -S (设置种子)
                use_hw_entropy = false;
                if (i + 1 < args.size())
                {
                    if (!parse_uint64(args[++i], seed))
                    {
                        std::cerr << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"error_seed"))) << std::endl;
                        return 1;
                    }
                    else
                    {
                        std::cout << seed << std::endl;
                    }
                }
                break;

            case 2: // -s (设置种子+硬件熵)
                use_hw_entropy = true;
                if (i + 1 < args.size())
                {
                    if (!parse_uint64(args[++i], seed))
                    {
                        std::cerr << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"error_seed"))) << std::endl;
                        return 1;
                    }
                    else
                    {
                        std::cout << seed << std::endl;
                    }
                }
                break;

            case 3: // -l (设置长度)
                if (i + 1 < args.size())
                {
                    uint64_t len_val;
                    if (parse_uint64(args[++i], len_val) && len_val > 0)
                    {
                        length = static_cast<size_t>(len_val);
                    }
                    else
                    {
                        std::cerr << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"error_length"))) << std::endl;
                        return 1;
                    }
                }
                break;

            case 4: // -c (设置数量)
                if (i + 1 < args.size())
                {
                    uint64_t cnt_val;
                    if (parse_uint64(args[++i], cnt_val) && cnt_val > 0)
                    {
                        count = static_cast<size_t>(cnt_val);
                    }
                    else
                    {
                        std::cerr << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"error_count"))) << std::endl;
                        return 1;
                    }
                }
                break;

            case 100: // -all (所有字符)
                char_set += U"abcdefghijklmnopqrstuvwxyz";
                char_set += U"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                char_set += U"0123456789";
                char_set += U"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
                break;

            case 101: // -aa (小写字母)
                char_set += U"abcdefghijklmnopqrstuvwxyz";
                break;

            case 102: // -aA (大写字母)
                char_set += U"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                break;

            case 103: // -a0 (数字)
                char_set += U"0123456789";
                break;

            case 104: // -a! (特殊字符)
                char_set += U"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
                break;

            case 105: // -ai (自定义字符)
                if (i + 1 < args.size())
                {
                    const std::u32string &custom = args[++i];
                    char_set += custom;
                }
                break;
            }
        }
    }

    // 显示帮助
    if (show_help)
    {
        std::cout << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"help_title"))) << std::endl;
        std::cout << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"help_usage"))) << std::endl;
        std::cout << utf8_to_local(utf32_to_utf8(get_lang_string(lang, U"help_options"))) << std::endl;
        return 0;
    }

    // 如果没有指定字符集，使用默认（小写字母+数字）
    if (char_set.empty())
    {
        char_set += U"abcdefghijklmnopqrstuvwxyz";
        char_set += U"0123456789";
    }

    // 去重
    std::sort(char_set.begin(), char_set.end());
    auto last = std::unique(char_set.begin(), char_set.end());
    char_set.erase(last, char_set.end());

    // 初始化随机引擎
    std::random_device rd;
    uint64_t final_seed = seed;

    if (use_hw_entropy)
    {
        final_seed ^= static_cast<uint64_t>(rd()) << 32;
        final_seed ^= rd();
    }
    else if (seed == 0)
    {
        final_seed = static_cast<uint64_t>(rd()) << 32 | rd();
    }

    // 生成并输出随机字符串
    for (size_t i = 0; i < count; i++)
    {
        std::u32string rand_str = generate_random_string(char_set, length, seed, use_hw_entropy);
        std::cout << utf8_to_local(utf32_to_utf8(rand_str)) << std::endl;
    }

    return 0;
}