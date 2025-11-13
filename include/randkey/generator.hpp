#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "randkey/options.hpp"

namespace randkey
{
    struct GenerationOutcome
    {
        std::vector<std::u32string> keys;
        std::optional<std::uint64_t> deterministic_seed;
        std::optional<std::uint64_t> mixing_seed;
    };

    class RandomKeyGenerator
    {
    public:
        GenerationOutcome generate(const GenerationOptions &options,
                                   std::optional<std::uint64_t> deterministic_seed_only = std::nullopt,
                                   std::optional<std::uint64_t> mixing_seed = std::nullopt) const;

    private:
        static std::u32string generate_single(const std::vector<std::u32string> &tokens,
                                              std::size_t length,
                                              std::optional<std::uint64_t> deterministic_seed_only,
                                              std::optional<std::uint64_t> mixing_seed,
                                              std::size_t index);
    };

}
