#include "randkey/random_engine.hpp"

#include "randkey/platform/random_device.hpp"

#include <array>
#include <limits>
#include <stdexcept>

namespace randkey
{
    bool SecureRandom::available() noexcept
    {
        return platform::secure_random_available();
    }

    void SecureRandom::fill(std::span<std::byte> buffer)
    {
        if (buffer.empty())
        {
            return;
        }

        if (!available())
        {
            throw std::runtime_error("error_random_device");
        }

        try
        {
            platform::secure_random_fill(buffer);
        }
        catch (const std::exception &)
        {
            throw std::runtime_error("error_random_device");
        }
    }

    std::uint64_t SecureRandom::uniform(std::uint64_t upper)
    {
        if (upper == 0)
        {
            throw std::invalid_argument("uniform 上界必须大于 0");
        }

        const std::uint64_t range = std::numeric_limits<std::uint64_t>::max();
        const std::uint64_t threshold = range - (range % upper);

        while (true)
        {
            std::array<std::byte, sizeof(std::uint64_t)> data{};
            SecureRandom::fill(data);
            std::uint64_t value = 0;
            for (std::size_t i = 0; i < data.size(); ++i)
            {
                value <<= 8U;
                value |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(data[i]));
            }

            if (value < threshold)
            {
                return value % upper;
            }
        }
    }
}
