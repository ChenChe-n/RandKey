#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "randkey/encoding.hpp"
#include "randkey/generator.hpp"
#include "randkey/i18n/messages.hpp"
#include "randkey/options.hpp"
#include "randkey/platform/language.hpp"

namespace randkey
{
    namespace
    {
        std::string format_message(const i18n::Catalog &catalog,
                                   const std::string &lang,
                                   const std::string &code)
        {
            auto pos = code.find(':');
            std::string key = code.substr(0, pos);
            std::string detail = (pos == std::string::npos) ? std::string() : code.substr(pos + 1);

            std::string_view translated = catalog.translate(lang, key);
            if (translated.empty())
            {
                translated = catalog.translate("en-US", key);
            }

            std::string result(translated);
            if (!detail.empty())
            {
                if (!result.empty())
                {
                    result.append(" ");
                }
                result.append(detail);
            }
            if (result.empty())
            {
                result = code;
            }
            return result;
        }

        void print_help(const i18n::Catalog &catalog, const std::string &lang)
        {
            std::cout << catalog.translate(lang, "help_title") << "\n";
            std::cout << catalog.translate(lang, "help_usage") << "\n";
            std::cout << catalog.translate(lang, "help_options") << "\n";
        }

        void write_output(const GenerationOptions &options,
                          const GenerationOutcome &outcome)
        {
            if (options.target == OutputTarget::Stdout)
            {
                for (const auto &key : outcome.keys)
                {
                    std::cout << utf32_to_locale(key) << "\n";
                }
                return;
            }

            const auto &path = options.output_path.value();
            if (std::filesystem::exists(path) && !options.force_overwrite)
            {
                throw std::runtime_error("error_output_exists:" + path.string());
            }

            std::ofstream out(path, std::ios::binary);
            if (!out)
            {
                throw std::runtime_error("error_write_file:" + path.string());
            }

            for (const auto &key : outcome.keys)
            {
                out << utf32_to_locale(key) << '\n';
            }
            if (!out)
            {
                throw std::runtime_error("error_write_file:" + path.string());
            }
        }

        void maybe_print_seed(const i18n::Catalog &catalog,
                              const std::string &lang,
                              const GenerationOutcome &outcome,
                              const ParsedArguments &args)
        {
            if (!args.options.show_seed)
            {
                return;
            }

            if (outcome.deterministic_seed.has_value())
            {
                std::cout << catalog.translate(lang, "info_seed_deterministic")
                          << ' ' << outcome.deterministic_seed.value() << "\n";
            }
            if (outcome.mixing_seed.has_value())
            {
                std::cout << catalog.translate(lang, "info_seed_mixing")
                          << ' ' << outcome.mixing_seed.value() << "\n";
            }
        }
    }
}

int main(int argc, const char *argv[])
{
    using namespace randkey;

    const auto language = platform::detect_system_language();
    auto catalog = i18n::build_default_catalog();
    catalog.set_fallback("en-US");

    ArgumentParser parser;
    ParsedArguments parsed;

    try
    {
        parsed = parser.parse(argc, argv);
    }
    catch (const std::exception &ex)
    {
        std::cerr << format_message(catalog, language, ex.what()) << "\n";
        return 1;
    }

    if (parsed.request_help)
    {
        print_help(catalog, language);
        return 0;
    }

    if (parsed.request_version)
    {
        std::cout << "RandKey " << RANDKEY_VERSION << "\n";
        return 0;
    }

    try
    {
        RandomKeyGenerator generator;
        const GenerationOutcome outcome = generator.generate(parsed.options,
                                                             parsed.deterministic_seed,
                                                             parsed.mixing_seed);

        write_output(parsed.options, outcome);
        maybe_print_seed(catalog, language, outcome, parsed);
    }
    catch (const std::exception &ex)
    {
        std::cerr << format_message(catalog, language, ex.what()) << "\n";
        return 1;
    }

    return 0;
}
