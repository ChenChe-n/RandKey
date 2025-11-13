#include "randkey/platform/random_device.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace randkey::platform
{
    namespace
    {
        int open_device()
        {
            static int descriptor = [] {
                int fd = ::open("/dev/urandom", O_RDONLY | O_CLOEXEC);
                return fd;
            }();
            return descriptor;
        }

        void read_bytes(int fd, std::span<std::byte> buffer)
        {
            std::byte *data = buffer.data();
            std::size_t remaining = buffer.size();

            while (remaining > 0)
            {
                ssize_t read_result = ::read(fd, data, remaining);
                if (read_result < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    throw std::runtime_error("读取 /dev/urandom 失败: " + std::string(std::strerror(errno)));
                }
                if (read_result == 0)
                {
                    throw std::runtime_error("读取 /dev/urandom 返回 EOF");
                }

                data += static_cast<std::size_t>(read_result);
                remaining -= static_cast<std::size_t>(read_result);
            }
        }
    }

    bool secure_random_available() noexcept
    {
        return open_device() >= 0;
    }

    void secure_random_fill(std::span<std::byte> buffer)
    {
        if (buffer.empty())
        {
            return;
        }

        int fd = open_device();
        if (fd < 0)
        {
            throw std::runtime_error("无法打开 /dev/urandom");
        }

        read_bytes(fd, buffer);
    }
}
