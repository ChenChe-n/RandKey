#include "randkey/options.hpp"

#include "randkey/encoding.hpp"

#include <array>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace randkey
{
    namespace
    {
        std::uint64_t parse_positive_integer(std::u32string_view value, std::string_view error_key)
        {
            if (value.empty())
            {
                throw std::runtime_error(std::string(error_key));
            }

            std::uint64_t result = 0;
            for (char32_t ch : value)
            {
                if (ch < U'0' || ch > U'9')
                {
                    throw std::runtime_error(std::string(error_key));
                }

                std::uint64_t digit = static_cast<std::uint64_t>(ch - U'0');
                if (result > (std::numeric_limits<std::uint64_t>::max() - digit) / 10)
                {
                    throw std::runtime_error(std::string(error_key));
                }
                result = result * 10 + digit;
            }

            if (result == 0)
            {
                throw std::runtime_error(std::string(error_key));
            }

            return result;
        }
    }

    ParsedArguments ArgumentParser::parse(int argc, const char *const *argv) const
    {
        std::vector<std::u32string> args;
        args.reserve(static_cast<std::size_t>(argc));
        for (int i = 0; i < argc; ++i)
        {
            args.push_back(locale_to_utf32(argv[i]));
        }

        ParsedArguments result;

        for (std::size_t index = 1; index < args.size(); ++index)
        {
            handle_flag(args[index], index, args, result);
        }

        result.validate();
        return result;
    }

    void ParsedArguments::validate() const
    {
        if (deterministic_seed.has_value() && mixing_seed.has_value())
        {
            throw std::runtime_error("error_conflicting_seed");
        }

        if (options.target == OutputTarget::File)
        {
            if (!options.output_path.has_value())
            {
                throw std::runtime_error("error_missing_output_path");
            }
        }
        else if (options.output_path.has_value())
        {
            throw std::runtime_error("error_unexpected_output_path");
        }
    }

    void ArgumentParser::handle_flag(std::u32string_view flag,
                                     std::size_t &index,
                                     const std::vector<std::u32string> &args,
                                     ParsedArguments &result) const
    {
        auto flag_utf8 = utf32_to_utf8(flag);

        if (flag == U"-h" || flag == U"--help")
        {
            result.request_help = true;
            return;
        }
        if (flag == U"--version")
        {
            result.request_version = true;
            return;
        }
        if (flag == U"-S" || flag == U"--seed-only")
        {
            auto value = expect_value(args, index, flag);
            result.deterministic_seed = parse_positive_integer(value, "error_seed");
            return;
        }
        if (flag == U"-s" || flag == U"--seed")
        {
            auto value = expect_value(args, index, flag);
            result.mixing_seed = parse_positive_integer(value, "error_seed");
            return;
        }
        if (flag == U"-l" || flag == U"--length")
        {
            auto value = expect_value(args, index, flag);
            result.options.length = static_cast<std::size_t>(parse_positive_integer(value, "error_length"));
            return;
        }
        if (flag == U"-c" || flag == U"--count")
        {
            auto value = expect_value(args, index, flag);
            result.options.count = static_cast<std::size_t>(parse_positive_integer(value, "error_count"));
            return;
        }
        if (flag == U"-all" || flag == U"--all")
        {
            result.options.registry.include_all_builtins();
            return;
        }
        if (flag == U"-aa" || flag == U"--lower")
        {
            result.options.registry.include(BuiltinCharset::Lowercase);
            return;
        }
        if (flag == U"-aA" || flag == U"--upper")
        {
            result.options.registry.include(BuiltinCharset::Uppercase);
            return;
        }
        if (flag == U"-a0" || flag == U"--digits")
        {
            result.options.registry.include(BuiltinCharset::Digits);
            return;
        }
        if (flag == U"-a!" || flag == U"--special")
        {
            result.options.registry.include(BuiltinCharset::Special);
            return;
        }
        if (flag == U"-ai" || flag == U"--append")
        {
            auto value = expect_value(args, index, flag);
            result.options.registry.add_characters(value);
            return;
        }
        if (flag == U"-at" || flag == U"--append-token")
        {
            auto value = expect_value(args, index, flag);
            result.options.registry.add_token(std::u32string(value));
            return;
        }
        if (flag == U"-af" || flag == U"--append-file")
        {
            auto value = expect_value(args, index, flag);
            const auto utf8 = utf32_to_utf8(value);
            std::u8string u8(utf8.begin(), utf8.end());
            result.options.registry.add_from_file(std::filesystem::path(u8));
            return;
        }
        if (flag == U"-aft" || flag == U"--append-file-token")
        {
            auto value = expect_value(args, index, flag);
            const auto utf8 = utf32_to_utf8(value);
            std::u8string u8(utf8.begin(), utf8.end());
            result.options.registry.add_from_file(std::filesystem::path(u8), true);
            return;
        }
        if (flag == U"-o" || flag == U"--output")
        {
            auto value = expect_value(args, index, flag);
            result.options.target = OutputTarget::File;
            const auto utf8 = utf32_to_utf8(value);
            std::u8string u8(utf8.begin(), utf8.end());
            result.options.output_path = std::filesystem::path(u8);
            return;
        }
        if (flag == U"--force")
        {
            result.options.force_overwrite = true;
            return;
        }
        if (flag == U"--show-seed")
        {
            result.options.show_seed = true;
            return;
        }

        throw std::runtime_error("error_unknown_flag:" + flag_utf8);
    }

    std::u32string_view ArgumentParser::expect_value(const std::vector<std::u32string> &args,
                                                     std::size_t &index,
                                                     std::u32string_view flag)
    {
        if (index + 1 >= args.size())
        {
            auto flag_utf8 = utf32_to_utf8(flag);
            throw std::runtime_error("error_missing_arg:" + flag_utf8);
        }
        ++index;
        return args[index];
    }
}
