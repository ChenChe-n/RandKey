#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "randkey/charset_registry.hpp"

namespace randkey
{
    enum class OutputTarget
    {
        Stdout,
        File,
    };

    struct GenerationOptions
    {
        CharsetRegistry registry;
        std::size_t length{12};
        std::size_t count{1};

        OutputTarget target{OutputTarget::Stdout};
        std::optional<std::filesystem::path> output_path{};

        bool force_overwrite{false};
        bool show_seed{false};
    };

    struct ParsedArguments
    {
        bool request_help{false};
        bool request_version{false};
        std::optional<std::uint64_t> mixing_seed{};
        std::optional<std::uint64_t> deterministic_seed{};
        GenerationOptions options{};

        void validate() const;
    };

    class ArgumentParser
    {
    public:
        ParsedArguments parse(int argc, const char *const *argv) const;

    private:
        void handle_flag(std::u32string_view flag,
                         std::size_t &index,
                         const std::vector<std::u32string> &args,
                         ParsedArguments &result) const;

        static std::u32string_view expect_value(const std::vector<std::u32string> &args,
                                                std::size_t &index,
                                                std::u32string_view flag);
    };
}
