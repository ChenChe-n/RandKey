#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "randkey/encoding.hpp"
#include "randkey/generator.hpp"
#include "randkey/options.hpp"

namespace
{
    int failures = 0;

    void expect(bool condition, const std::string &message)
    {
        if (!condition)
        {
            std::cerr << "[cli] " << message << std::endl;
            ++failures;
        }
    }
}

int run_cli_tests()
{
    using namespace randkey;

    const char *argv[] = {
        "randkey",
        "--seed-only",
        "123",
        "--length",
        "6",
        "--count",
        "2",
        "--append",
        "abc",
        "--output",
        "cli_test_output.txt",
        "--force",
    };

    ArgumentParser parser;
    ParsedArguments parsed;

    try
    {
        parsed = parser.parse(static_cast<int>(std::size(argv)), argv);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[cli] parse failed: " << ex.what() << std::endl;
        ++failures;
        return failures;
    }

    expect(parsed.deterministic_seed.has_value(), "deterministic seed should be parsed");
    expect(!parsed.mixing_seed.has_value(), "mixing seed should not be set in deterministic mode");
    expect(parsed.options.output_path.has_value(), "output path should be captured");

    std::filesystem::path path = parsed.options.output_path.value();
    RandomKeyGenerator generator;
    auto outcome = generator.generate(parsed.options, parsed.deterministic_seed, parsed.mixing_seed);
    expect(outcome.keys.size() == 2, "should generate requested count");

    {
        std::ofstream out(path, std::ios::binary);
        for (const auto &key : outcome.keys)
        {
            out << randkey::utf32_to_locale(key) << '\n';
        }
    }

    if (std::filesystem::exists(path))
    {
        std::ifstream file(path, std::ios::binary);
        std::string line;
        std::size_t lines = 0;
        while (std::getline(file, line))
        {
            ++lines;
        }
        expect(lines == 2, "output file should contain generated keys");
        file.close();
        std::filesystem::remove(path);
    }
    else
    {
        expect(false, "output file should exist after manual write");
    }

    return failures;
}
