#include <array>
#include <iostream>
#include <stdexcept>

#include "randkey/generator.hpp"

namespace
{
    int failures = 0;

    void expect(bool condition, const char *message)
    {
        if (!condition)
        {
            std::cerr << "[generator] " << message << std::endl;
            ++failures;
        }
    }

    void expect_throw(const char *message, auto &&callable)
    {
        try
        {
            callable();
            std::cerr << "[generator] " << message << std::endl;
            ++failures;
        }
        catch (const std::exception &)
        {
        }
    }
}

int run_generator_tests()
{
    using namespace randkey;

    RandomKeyGenerator generator;

    {
        GenerationOptions options;
        options.length = 8;
        options.count = 2;
        options.registry.include(BuiltinCharset::Digits);

        auto first = generator.generate(options, 12345ULL, std::nullopt);
        auto second = generator.generate(options, 12345ULL, std::nullopt);

        expect(first.deterministic_seed.has_value() && first.deterministic_seed == second.deterministic_seed,
               "deterministic seed should be preserved");
        expect(!first.mixing_seed.has_value(), "deterministic mode should not set mixing seed");
        expect(first.keys == second.keys, "deterministic generation should be reproducible");
    }

    {
        GenerationOptions options;
        options.length = 8;
        options.count = 1;
        options.registry.include(BuiltinCharset::Lowercase);

        auto outcome = generator.generate(options, std::nullopt, 42ULL);
        expect(outcome.mixing_seed.has_value() && outcome.mixing_seed.value() == 42ULL,
               "mixing seed should be propagated");
        expect(!outcome.keys.empty(), "should generate at least one key");
    }

    {
        GenerationOptions options;
        options.length = 4;
        options.count = 1;
        auto outcome = generator.generate(options, std::nullopt, 1ULL);
        expect(!outcome.keys.empty(), "default charset should be applied when none specified");
    }

    {
        GenerationOptions options;
        options.length = 3;
        options.count = 1;
        options.registry.add_token(U"語言");
        options.registry.add_token(U"テスト");

        auto deterministic = generator.generate(options, 12345ULL, std::nullopt);
        expect(deterministic.keys.size() == 1, "should produce one key");

        const std::array<std::u32string, 2> tokens = {U"語言", U"テスト"};
        const std::u32string &key = deterministic.keys[0];
        std::size_t offset = 0;
        bool segmentation_ok = true;
        while (offset < key.size())
        {
            bool matched = false;
            for (const auto &token : tokens)
            {
                if (offset + token.size() <= key.size() && key.compare(offset, token.size(), token) == 0)
                {
                    offset += token.size();
                    matched = true;
                    break;
                }
            }

            if (!matched)
            {
                segmentation_ok = false;
                break;
            }
        }

        expect(segmentation_ok && offset == key.size(), "key should be concatenation of full tokens");

        auto outcome = generator.generate(options, std::nullopt, 7ULL);
        expect(outcome.keys.size() == 1, "should still generate with mixing seed");
    }

    return failures;
}
