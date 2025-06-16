#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <filesystem>
#include <fstream>

#include "uconv.hpp"
#include "get_sys_lang.hpp"

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
    {U"-ai", 105},
    {U"-af", 106},
    {U"-o", 200},
};

// 多语言字符串表
static const std::map<std::string, std::map<std::u32string, std::u32string>> lang_table = {
    {"zh-CN",
     {
         {U"help_title", U"随机字符串生成器 - 帮助"},
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
                           U"  -ai <字符>     添加自定义字符\n"
                           U"  -af <文件名>   从文件中读取字符\n"
                           U"  -o <文件名>    输出到文件\n"},
         {U"error_seed", U"错误: 种子必须是64位整数"},
         {U"error_length", U"错误: 长度必须是正整数"},
         {U"error_count", U"错误: 数量必须是正整数"},
         {U"error_missing_arg", U"错误: 缺少参数 "},
     }},
    {"en-US",
     {
         {U"help_title", U"Random String Generator - Help"},
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
                           U"  -ai <chars>    Add custom characters\n"
                           U"  -af <file>     Read custom characters from file\n"
                           U"  -o <file>       Write passwords to file\n"},
         {U"error_seed", U"Error: Seed must be a 64-bit integer"},
         {U"error_length", U"Error: Length must be a positive integer"},
         {U"error_count", U"Error: Count must be a positive integer"},
         {U"error_missing_arg", U"Error: Missing argument"},
     }},
    {"ja-JP",
     {
         {U"help_title", U"ランダム文字列生成器 - ヘルプ"},
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
                           U"  -ai <文字>     カスタム文字を追加\n"
                           U"  -af <ファイル>  カスタム文字をファイルから読み込む\n"
                           U"  -o <ファイル>  出力先を指定\n"},
         {U"error_seed", U"エラー: シードは64ビット整数である必要があります"},
         {U"error_length", U"エラー: 長さは正の整数である必要があります"},
         {U"error_count", U"エラー: 数は正の整数である必要があります"},
         {U"error_missing_arg", U"エラー: 引数が不足しています"},
     }}};

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
    auto en_iter = lang_table.find("en-US");
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

int run(int argc, const char *argv[])
{
    std::string lang = get_sys_lang(); // 获取系统语言
    std::u32string char_set;           // 字符集

    // 默认参数
    uint64_t seed = 0;                     // 种子
    size_t length = 12;                    // 生成长度
    size_t count = 1;                      // 生成数量
    bool use_hw_entropy = true;            // 使用硬件熵
    bool show_help = false;                // 显示帮助
    bool is_write_file = false;            // 写入文件
    std::filesystem::path write_file_path; // 文件路径

    // 转换参数为UTF-32
    std::vector<std::u32string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(uconv::locale_to_utf32(argv[i]));
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
                        std::cerr << uconv::utf32_to_locale(get_lang_string(lang, U"error_seed")) << std::endl;
                        return 1;
                    }
                    else
                    {
                        std::cout << seed << std::endl;
                    }
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
                }
                break;

            case 2: // -s (设置种子+硬件熵)
                use_hw_entropy = true;
                if (i + 1 < args.size())
                {
                    if (!parse_uint64(args[++i], seed))
                    {
                        std::cerr << uconv::utf32_to_locale(get_lang_string(lang, U"error_seed")) << std::endl;
                        return 1;
                    }
                    else
                    {
                        std::cout << seed << std::endl;
                    }
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
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
                        std::cerr << uconv::utf32_to_locale(get_lang_string(lang, U"error_length")) << std::endl;
                        return 1;
                    }
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
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
                        std::cerr << uconv::utf32_to_locale(get_lang_string(lang, U"error_count")) << std::endl;
                        return 1;
                    }
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
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
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
                }
                break;
            case 106: // -af (从文件读取字符)
                if (i + 1 < args.size())
                {
                    const std::u32string &custom = args[++i];
                    std::filesystem::path path(custom);
                    if (std::filesystem::exists(path)) // 文件存在
                    {
                        std::ifstream file(path);
                        std::u32string line;
                        std::string buffer;
                        while (std::getline(file, buffer)) // 读取文件
                        {
                            char_set += uconv::utf8_to_utf32({buffer.begin(), buffer.end()});
                        }
                    }
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
                }
                break;
            case 200: // -o (输出到文件)
                if (i + 1 < args.size())
                {
                    const std::u32string &custom = args[++i];
                    is_write_file = true;
                    write_file_path = custom;
                }
                else
                {
                    std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"error_missing_arg")) << std::endl;
                    std::cout << "  " << uconv::utf32_to_locale(it->first) << std::endl;
                    return 1;
                }
                break;
            }
        }
    }

    // 显示帮助
    if (show_help)
    {
        std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"help_title")) << std::endl;
        std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"help_usage")) << std::endl;
        std::cout << uconv::utf32_to_locale(get_lang_string(lang, U"help_options")) << std::endl;
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
    if (is_write_file)
    {
        if (std::filesystem::exists(write_file_path)) // 文件存在，则删除
        {
            std::filesystem::remove(write_file_path);
        }
        std::ofstream out(write_file_path);
        for (size_t i = 0; i < count; i++)
        {
            out << uconv::utf32_to_locale(generate_random_string(char_set, length, seed, use_hw_entropy)) << std::endl;
        }
    }
    else
    {
        for (size_t i = 0; i < count; i++)
        {
            std::cout << uconv::utf32_to_locale(generate_random_string(char_set, length, seed, use_hw_entropy)) << std::endl;
        }
    }
    return 0;
}

int main(int argc, const char *argv[])
{
    try
    {
        run(argc, argv);
    }
    catch (const std::runtime_error &e)
    {
        std::string msg = e.what();
    }
    return 0;
}