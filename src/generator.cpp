#include "randkey/generator.hpp"

#include "randkey/encoding.hpp"
#include "randkey/random_engine.hpp"

#include <array>
#include <random>
#include <stdexcept>
#include <vector>

namespace randkey
{
    namespace
    {
        constexpr std::uint64_t GOLDEN = 0x9E3779B97F4A7C15ULL;

        std::uint64_t read_random_u64()
        {
            std::array<std::byte, sizeof(std::uint64_t)> buffer{};
            SecureRandom::fill(buffer);
            std::uint64_t value = 0;
            for (auto byte : buffer)
            {
                value = (value << 8U) | static_cast<std::uint64_t>(std::to_integer<unsigned char>(byte));
            }
            return value;
        }
    }

    GenerationOutcome RandomKeyGenerator::generate(const GenerationOptions &options,
                                                   std::optional<std::uint64_t> deterministic_seed_only,
                                                   std::optional<std::uint64_t> mixing_seed) const
    {
        GenerationOptions effective = options;
        effective.registry.ensure_default();
        const auto tokens = effective.registry.materialize();

        if (tokens.empty())
        {
            throw std::runtime_error("error_charset_empty");
        }

        GenerationOutcome outcome{};
        outcome.deterministic_seed = deterministic_seed_only;

        if (!deterministic_seed_only.has_value())
        {
            outcome.mixing_seed = mixing_seed.has_value() ? mixing_seed : std::optional<std::uint64_t>(read_random_u64());
        }
        else if (mixing_seed.has_value())
        {
            throw std::logic_error("error_conflicting_seed");
        }

        outcome.keys.reserve(effective.count);
        for (std::size_t i = 0; i < effective.count; ++i)
        {
            outcome.keys.push_back(generate_single(tokens,
                                                   effective.length,
                                                   deterministic_seed_only,
                                                   outcome.mixing_seed,
                                                   i));
        }

        return outcome;
    }

    std::u32string RandomKeyGenerator::generate_single(const std::vector<std::u32string> &tokens,
                                                       std::size_t length,
                                                       std::optional<std::uint64_t> deterministic_seed_only,
                                                       std::optional<std::uint64_t> mixing_seed,
                                                       std::size_t index)
    {
        if (tokens.empty())
        {
            throw std::runtime_error("error_charset_empty");
        }

        std::u32string result;
        if (deterministic_seed_only.has_value())
        {
            const std::uint64_t seed = deterministic_seed_only.value() + static_cast<std::uint64_t>(index) * GOLDEN;
            std::mt19937_64 engine(seed);
            std::uniform_int_distribution<std::size_t> distribution(0, tokens.size() - 1);

            for (std::size_t i = 0; i < length; ++i)
            {
                const std::u32string &token = tokens[distribution(engine)];
                result.insert(result.end(), token.begin(), token.end());
            }
            return result;
        }

        const std::uint64_t offset_seed = mixing_seed.has_value() ? (mixing_seed.value() + static_cast<std::uint64_t>(index) * GOLDEN) : 0ULL;

        for (std::size_t i = 0; i < length; ++i)
        {
            const std::uint64_t raw = SecureRandom::uniform(static_cast<std::uint64_t>(tokens.size()));
            std::size_t choice = static_cast<std::size_t>(raw);

            if (mixing_seed.has_value() && tokens.size() > 1)
            {
                const std::uint64_t tweak = (offset_seed + static_cast<std::uint64_t>(i)) % static_cast<std::uint64_t>(tokens.size());
                choice = static_cast<std::size_t>((choice + tweak) % static_cast<std::uint64_t>(tokens.size()));
            }

            const std::u32string &token = tokens[choice];
            result.insert(result.end(), token.begin(), token.end());
        }

        return result;
    }
}
