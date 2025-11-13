#include "randkey/i18n/messages.hpp"

namespace randkey::i18n
{
    Catalog build_default_catalog()
    {
        Catalog catalog;
        catalog.insert("en-US",
                       {
                           {"help_title", "RandKey - Secure Random Key Generator"},
                           {"help_usage", "Usage: randkey [options]"},
                           {"help_options", "Options:\n"
                                             "  -h, --help            Show this help message\n"
                                             "  --version             Show version information\n"
                                             "  -S, --seed-only <n>   Use deterministic seed (no hardware entropy)\n"
                                             "  -s, --seed <n>        Mix deterministic seed with hardware entropy\n"
                                             "  -l, --length <n>      Set length of each key (default 12)\n"
                                             "  -c, --count <n>       Number of keys to generate (default 1)\n"
                                             "  -all, --all           Include all built-in character sets\n"
                                             "  -aa, --lower          Include lowercase letters\n"
                                             "  -aA, --upper          Include uppercase letters\n"
                                             "  -a0, --digits         Include digits\n"
                                             "  -a!, --special        Include special characters\n"
                                             "  -ai, --append <chars> Append custom characters\n"
                                             "  -at, --append-token <token> Append multi-character token\n"
                                             "  -af, --append-file <file> Append characters from file\n"
                                             "  -aft, --append-file-token <file> Append tokens (per line) from file\n"
                                             "  -o, --output <file>   Write results to file\n"
                                             "      --force           Overwrite output file if exists\n"
                                             "      --show-seed       Print the seed used for generation"},
                           {"error_seed", "Error: invalid seed value"},
                           {"error_length", "Error: length must be a positive integer"},
                           {"error_count", "Error: count must be a positive integer"},
                           {"error_missing_output_path", "Error: output path is required when --output is specified"},
                           {"error_unexpected_output_path", "Error: output path is only valid when using --output"},
                           {"error_conflicting_seed", "Error: --seed and --seed-only cannot be used together"},
                           {"error_unknown_flag", "Error: unknown option"},
                           {"error_missing_arg", "Error: option requires an argument"},
                           {"error_charset_file", "Error: failed to load character file"},
                           {"error_charset_empty", "Error: no characters available for generation"},
                           {"error_random_device", "Error: secure random source unavailable"},
                           {"error_output_exists", "Error: output file already exists"},
                           {"error_write_file", "Error: unable to write output file"},
                           {"info_seed_deterministic", "Deterministic seed:"},
                           {"info_seed_mixing", "Mixing seed:"},
                       });

        catalog.insert("zh-CN",
                       {
                           {"help_title", "RandKey - 安全随机密钥生成器"},
                           {"help_usage", "用法: randkey [选项]"},
                           {"help_options", "选项:\n"
                                             "  -h, --help            显示帮助信息\n"
                                             "  --version             显示版本号\n"
                                             "  -S, --seed-only <n>   使用确定性种子（不混合硬件熵）\n"
                                             "  -s, --seed <n>        使用确定性种子并混合硬件熵\n"
                                             "  -l, --length <n>      设置每个密钥长度（默认 12）\n"
                                             "  -c, --count <n>       生成密钥数量（默认 1）\n"
                                             "  -all, --all           包含全部内置字符集\n"
                                             "  -aa, --lower          包含小写字母\n"
                                             "  -aA, --upper          包含大写字母\n"
                                             "  -a0, --digits         包含数字\n"
                                             "  -a!, --special        包含特殊字符\n"
                                             "  -ai, --append <字符>  添加自定义字符\n"
                                             "  -at, --append-token <短语> 添加多字符短语\n"
                                             "  -af, --append-file <文件> 从文件追加字符\n"
                                             "  -aft, --append-file-token <文件> 按行追加短语\n"
                                             "  -o, --output <文件>   将结果写入文件\n"
                                             "      --force           若文件存在则覆盖写入\n"
                                             "      --show-seed       输出所使用的种子"},
                           {"error_seed", "错误: 种子无效"},
                           {"error_length", "错误: 长度必须是正整数"},
                           {"error_count", "错误: 数量必须是正整数"},
                           {"error_missing_output_path", "错误: 使用 --output 时必须提供文件路径"},
                           {"error_unexpected_output_path", "错误: 仅在使用 --output 时才能提供文件路径"},
                           {"error_conflicting_seed", "错误: --seed 与 --seed-only 不能同时使用"},
                           {"error_unknown_flag", "错误: 未知选项"},
                           {"error_missing_arg", "错误: 选项缺少参数"},
                           {"error_charset_file", "错误: 读取字符集文件失败"},
                           {"error_charset_empty", "错误: 字符集为空"},
                           {"error_random_device", "错误: 安全随机源不可用"},
                           {"error_output_exists", "错误: 输出文件已存在"},
                           {"error_write_file", "错误: 写入输出文件失败"},
                           {"info_seed_deterministic", "确定性种子:"},
                           {"info_seed_mixing", "混合种子:"},
                       });

        return catalog;
    }
}
