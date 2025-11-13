#include "randkey/platform/random_device.hpp"

#include <windows.h>
#include <bcrypt.h>

#include <stdexcept>

namespace randkey::platform
{
    namespace
    {
        class AlgorithmHandle
        {
        public:
            AlgorithmHandle()
            {
                NTSTATUS status = BCryptOpenAlgorithmProvider(&handle_, BCRYPT_RNG_ALGORITHM, nullptr, 0);
                if (!BCRYPT_SUCCESS(status))
                {
                    handle_ = nullptr;
                }
            }

            ~AlgorithmHandle()
            {
                if (handle_ != nullptr)
                {
                    BCryptCloseAlgorithmProvider(handle_, 0);
                }
            }

            AlgorithmHandle(const AlgorithmHandle &) = delete;
            AlgorithmHandle &operator=(const AlgorithmHandle &) = delete;

            BCRYPT_ALG_HANDLE get() const noexcept
            {
                return handle_;
            }

        private:
            BCRYPT_ALG_HANDLE handle_{};
        };

        AlgorithmHandle &get_handle()
        {
            static AlgorithmHandle instance;
            return instance;
        }
    }

    bool secure_random_available() noexcept
    {
        return get_handle().get() != nullptr;
    }

    void secure_random_fill(std::span<std::byte> buffer)
    {
        if (buffer.empty())
        {
            return;
        }

        auto handle = get_handle().get();
        if (handle == nullptr)
        {
            throw std::runtime_error("BCrypt 随机数提供者不可用");
        }

        NTSTATUS status = BCryptGenRandom(handle,
                                          reinterpret_cast<PUCHAR>(buffer.data()),
                                          static_cast<ULONG>(buffer.size()),
                                          0);
        if (!BCRYPT_SUCCESS(status))
        {
            throw std::runtime_error("BCryptGenRandom 调用失败");
        }
    }
}
