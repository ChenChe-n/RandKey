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

        auto outcome = generator.generate(options, std::nullopt, 7ULL);
        expect(outcome.keys.size() == 1, "should produce one key");
        expect(outcome.keys[0].size() % 2 == 0, "multi-character tokens should concatenate as units");
        expect(outcome.keys[0].size() >= 6, "token length should reflect phrase size");
    }

    return failures;
}
