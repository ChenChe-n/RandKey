#pragma once

#include <cstddef>
#include <span>

namespace randkey::platform
{
    bool secure_random_available() noexcept;

    void secure_random_fill(std::span<std::byte> buffer);
}
