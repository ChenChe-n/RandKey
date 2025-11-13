#pragma once

#include <cstdint>
#include <span>

namespace randkey
{
    /// @brief 跨平台密码学安全随机源
    class SecureRandom
    {
    public:
        /// @brief 当前平台是否提供安全随机源
        static bool available() noexcept;

        /// @brief 使用安全随机源填充缓冲区
        /// @throws std::runtime_error 当随机源不可用或读取失败
        static void fill(std::span<std::byte> buffer);

        /// @brief 生成 [0, upper) 区间内的均匀随机数
        /// @param upper 上界（必须 > 0）
        /// @throws std::invalid_argument 当 upper 为 0
        static std::uint64_t uniform(std::uint64_t upper);
    };
}
