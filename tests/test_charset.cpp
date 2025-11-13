#include <filesystem>
#include <fstream>
#include <iostream>

#include "randkey/charset_registry.hpp"

namespace
{
    int failures = 0;

    void expect(bool condition, const char *message)
    {
        if (!condition)
        {
            std::cerr << "[charset] " << message << std::endl;
            ++failures;
        }
    }
}

int run_charset_tests()
{
    using namespace randkey;

    {
        CharsetRegistry registry;
        registry.ensure_default();
        auto tokens = registry.materialize();
        expect(!tokens.empty(), "default charset should not be empty");
        expect(tokens[0].size() == 1, "default tokens should be single characters");
    }

    {
        CharsetRegistry registry;
        registry.include(BuiltinCharset::Lowercase);
        registry.include(BuiltinCharset::Uppercase);
        registry.include(BuiltinCharset::Lowercase);
        auto tokens = registry.materialize();
        expect(tokens.size() >= 52, "duplicate includes should be deduplicated");
    }

    {
        const auto temp_path = std::filesystem::temp_directory_path() / "randkey_charset_test.txt";
        std::ofstream out(temp_path, std::ios::binary);
        out << "abc" << std::endl;
        out.close();

        CharsetRegistry registry;
        registry.add_from_file(temp_path);
        auto tokens = registry.materialize();
        bool found = false;
        for (const auto &token : tokens)
        {
            if (!token.empty() && token[0] == U'a')
            {
                found = true;
                break;
            }
        }
        expect(found, "characters from file should be loaded");

        std::filesystem::remove(temp_path);
    }

    {
        CharsetRegistry registry;
        registry.add_token(U"語言");
        auto tokens = registry.materialize();
        expect(tokens.size() == 1 && tokens[0] == U"語言", "tokens should support multi-character phrases");
    }

    return failures;
}
