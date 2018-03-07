// MAVLink router and firewall.
// Copyright (C) 2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <system_error>

#include <catch.hpp>
#include <errno.h>
#include <fakeit.hpp>

#include "UnixSyscalls.hpp"
#include "UnixUDPSocket.hpp"

#include "common.hpp"


using namespace std::chrono_literals;


TEST_CASE("UnixUDPSocket's create and bind a UDP socket on construction and "
          "closes the socket on destruction.", "[UnixUDPSocket]")
{
    SECTION("Without a specific IP address (no errors).")
    {
        // Mock system calls.
        fakeit::Mock<UnixSyscalls> mock_sys;
        // Mock 'socket'.
        fakeit::When(Method(mock_sys, socket)).Return(3);
        // Mock 'bind'.
        struct sockaddr_in address;
        fakeit::When(Method(mock_sys, bind)).Do(
            [&](auto fd, auto addr, auto addrlen)
        {
            (void)fd;
            std::memcpy(&address, addr, addrlen);
            return 0;
        });
        // Mock 'close'.
        fakeit::When(Method(mock_sys, close)).Return(0);
        {
            // Construct socket.
            UnixUDPSocket socket(14050, {}, mock_unique(mock_sys));
            fakeit::Verify(Method(mock_sys, socket).Matching(
                               [&](auto family, auto type, auto protocol)
            {
                return family == AF_INET && type == SOCK_DGRAM && protocol == 0;
            })).Once();
            fakeit::Verify(Method(mock_sys, bind).Matching(
                               [&](auto fd, auto addr, auto addrlen)
            {
                (void)addr;
                return fd == 3 && addrlen == sizeof(address);
            })).Once();
            REQUIRE(address.sin_family == AF_INET);
            REQUIRE(ntohs(address.sin_port) == 14050);
            REQUIRE(ntohl(address.sin_addr.s_addr) == INADDR_ANY);

            for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
            {
                REQUIRE(address.sin_zero[i] == '\0');
            }

            fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(0);
        }
        fakeit::Verify(Method(mock_sys, close).Using(3)).Once();
    }
    SECTION("With a specific IP address (no errors).")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).Return(3);
        struct sockaddr_in address;
        fakeit::When(Method(mock_sys, bind)).Do(
            [&](auto fd, auto addr, auto addrlen)
        {
            (void)fd;
            std::memcpy(&address, addr, addrlen);
            return 0;
        });
        fakeit::When(Method(mock_sys, close)).Return(0);
        {
            UnixUDPSocket socket(
                14050, IPAddress(1234567890), mock_unique(mock_sys));
            fakeit::Verify(Method(mock_sys, socket).Matching(
                               [&](auto family, auto type, auto protocol)
            {
                return family == AF_INET && type == SOCK_DGRAM && protocol == 0;
            })).Once();
            fakeit::Verify(Method(mock_sys, bind).Matching(
                               [](auto fd, auto addr, auto addrlen)
            {
                (void)addr;
                return fd == 3 && addrlen == sizeof(address);
            })).Once();
            REQUIRE(address.sin_family == AF_INET);
            REQUIRE(ntohs(address.sin_port) == 14050);
            REQUIRE(ntohl(address.sin_addr.s_addr) == 1234567890);

            for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
            {
                REQUIRE(address.sin_zero[i] == '\0');
            }

            fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(0);
        }
        fakeit::Verify(Method(mock_sys, close).Using(3)).Once();
    }
    SECTION("Emmits errors from 'socket' system call.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).AlwaysReturn(-1);
        std::array<int, 7> errors{{
                EACCES,
                EAFNOSUPPORT,
                EINVAL,
                EMFILE,
                ENOBUFS,
                ENOMEM,
                EPROTONOSUPPORT
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(
                UnixUDPSocket(14050, {}, mock_unique(mock_sys)),
                std::system_error);
        }
    }
    SECTION("Emmits errors from 'bind' system call.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).AlwaysReturn(4);
        fakeit::When(Method(mock_sys, bind)).AlwaysReturn(-1);
        std::array<int, 13> errors{{
                EACCES,
                EADDRINUSE,
                EBADF,
                EINVAL,
                ENOTSOCK,
                EADDRNOTAVAIL,
                EFAULT,
                ELOOP,
                ENAMETOOLONG,
                ENOENT,
                ENOMEM,
                ENOTDIR,
                EROFS
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(
                UnixUDPSocket(14050, {}, mock_unique(mock_sys)),
                std::system_error);
        }
    }
}


TEST_CASE("UnixUDPSocket's 'send' method sends data on the socket.",
          "[UnixUDPSocket]")
{
    // Mock system calls.
    fakeit::Mock<UnixSyscalls> mock_sys;
    // Mock 'socket'.
    fakeit::When(Method(mock_sys, socket)).Return(3);
    // Mock 'bind'.
    fakeit::When(Method(mock_sys, bind)).Return(0);
    // Mock 'close'.
    fakeit::When(Method(mock_sys, close)).Return(0);
    UnixUDPSocket socket(14050, {}, mock_unique(mock_sys));
    SECTION("Without error.")
    {
        // Mock 'sendto'
        std::vector<uint8_t> sent;
        struct sockaddr_in address;
        fakeit::When(Method(mock_sys, sendto)).Do(
            [&](auto fd, auto buf, auto len, auto flags,
                auto addr, auto addrlen)
        {
            (void)fd;
            (void)flags;
            sent.resize(len);
            std::memcpy(sent.data(), buf, len);
            std::memcpy(&address, addr, addrlen);
            return len;
        });
        // Test
        std::vector<uint8_t> vec = {1, 3, 3, 7};
        socket.send(vec, IPAddress(1234567890, 14050));
        // Verify 'sendto'.
        fakeit::Verify(Method(mock_sys, sendto).Matching(
                           [](auto fd, auto buf, auto len, auto flags,
                              auto addr, auto addrlen)
        {
            (void)buf;
            (void)addr;
            return fd == 3 && len == 4 && flags == 0 &&
                   addrlen == sizeof(address);
        })).Once();
        REQUIRE(sent == vec);
        REQUIRE(address.sin_family == AF_INET);
        REQUIRE(ntohs(address.sin_port) == 14050);
        REQUIRE(ntohl(address.sin_addr.s_addr) == 1234567890);

        for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
        {
            REQUIRE(address.sin_zero[i] == '\0');
        }
    }
    SECTION("Emmits errors from 'sendto' system call.")
    {
        fakeit::When(Method(mock_sys, sendto)).AlwaysReturn(-1);
        std::array<int, 18> errors{{
                EACCES,
                EAGAIN,
                EWOULDBLOCK,
                EALREADY,
                EBADF,
                ECONNRESET,
                EDESTADDRREQ,
                EFAULT,
                EINTR,
                EINVAL,
                EISCONN,
                EMSGSIZE,
                ENOBUFS,
                ENOMEM,
                ENOTCONN,
                ENOTSOCK,
                EOPNOTSUPP,
                EPIPE
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(
                socket.send({1, 3, 3, 7}, IPAddress(1234567890, 14050)),
                std::system_error);
        }
    }
}


TEST_CASE("UnixUDPSocket's 'receive' method receives data on the socket.",
          "[UnixUDPSocket]")
{
    fakeit::Mock<UnixSyscalls> mock_sys;
    fakeit::When(Method(mock_sys, socket)).AlwaysReturn(3);
    fakeit::When(Method(mock_sys, bind)).AlwaysReturn(0);
    fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
    UnixUDPSocket socket(14050, {}, mock_unique(mock_sys));
    SECTION("Timeout, no packet (no errors).")
    {
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            return 0;
        });
        REQUIRE(
            socket.receive(250ms) ==
            std::pair<std::vector<uint8_t>, IPAddress>({}, IPAddress(0)));
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
    }
    SECTION("Poll error, close and restart the socket (no other errors).")
    {
        struct pollfd fds;
        struct sockaddr_in address;
        fakeit::When(Method(mock_sys, bind)).Do(
            [&](auto fd, auto addr, auto addrlen)
        {
            (void)fd;
            std::memcpy(&address, addr, addrlen);
            return 0;
        });
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLERR;
            return 1;
        });
        socket.receive(250ms);
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
        fakeit::Verify(Method(mock_sys, socket).Matching(
                           [&](auto family, auto type, auto protocol)
        {
            return family == AF_INET && type == SOCK_DGRAM && protocol == 0;
        })).Exactly(2);
        fakeit::Verify(Method(mock_sys, bind).Matching(
                           [&](auto fd, auto addr, auto addrlen)
        {
            (void)addr;
            return fd == 3 && addrlen == sizeof(address);
        })).Exactly(2);
        REQUIRE(address.sin_family == AF_INET);
        REQUIRE(ntohs(address.sin_port) == 14050);
        REQUIRE(ntohl(address.sin_addr.s_addr) == INADDR_ANY);

        for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
        {
            REQUIRE(address.sin_zero[i] == '\0');
        }

        fakeit::Verify(Method(mock_sys, close).Using(3)).Once();
    }
    SECTION("Packet available (no errors).")
    {
        // Mock poll system call.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock ioctl system call.
        fakeit::When(Method(mock_sys, ioctl)).Do(
            [](auto fd, auto request, auto size)
        {
            (void)fd;
            (void)request;
            *reinterpret_cast<int *>(size) = 4;
            return 0;
        });
        // Mock recvfrom.
        socklen_t address_length = 0;
        fakeit::When(Method(mock_sys, recvfrom)).Do(
            [&](auto fd, auto buf, auto len, auto flags,
                auto addr, auto addrlen)
        {
            (void)fd;
            (void)flags;
            // Write to buffer.
            std::vector<uint8_t> vec = {1, 3, 3, 7};
            std::memcpy(buf, vec.data(), std::min(vec.size(), len));
            // Set address
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(static_cast<uint16_t>(5000));
            address.sin_addr.s_addr = htonl(static_cast<uint32_t>(1234567890));
            memset(address.sin_zero, '\0', sizeof(address.sin_zero));
            std::memcpy(
                addr, &address,
                std::min(static_cast<size_t>(*addrlen), sizeof(address)));
            address_length = *addrlen;
            *addrlen = static_cast<socklen_t>(
                           std::min(static_cast<size_t>(*addrlen),
                                    sizeof(address)));
            // Return number of received bytes.
            return std::min(vec.size(), len);
        });
        // Test
        auto [data, ip] = socket.receive(250ms);
        REQUIRE(data == std::vector<uint8_t>({1, 3, 3, 7}));
        REQUIRE(ip == IPAddress(1234567890, 5000));
        // Verify poll system call.
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
        // Verify ioctl system call.
        fakeit::Verify(Method(mock_sys, ioctl).Matching(
                           [](auto fd, auto request, auto size)
        {
            (void)size;
            return fd == 3 && request == FIONREAD;
        })).Once();
        // Verify recvfrom.
        fakeit::Verify(Method(mock_sys, recvfrom).Matching(
                           [](auto fd, auto buf, auto len, auto flags,
                              auto addr, auto addrlen)
        {
            (void)buf;
            (void)addr;
            (void)addrlen;
            return fd == 3 && len >= 4 && flags == 0;
        })).Once();
        REQUIRE(address_length >= sizeof(sockaddr_in));
    }
    SECTION("Packet available (not IPv4).")
    {
        // Mock poll system call.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock ioctl system call.
        fakeit::When(Method(mock_sys, ioctl)).Do(
            [](auto fd, auto request, auto size)
        {
            (void)fd;
            (void)request;
            *reinterpret_cast<int *>(size) = 4;
            return 0;
        });
        // Mock recvfrom.
        socklen_t address_length = 0;
        fakeit::When(Method(mock_sys, recvfrom)).Do(
            [&](auto fd, auto buf, auto len, auto flags,
                auto addr, auto addrlen)
        {
            (void)fd;
            (void)flags;
            // Write to buffer.
            std::vector<uint8_t> vec = {1, 3, 3, 7};
            std::memcpy(buf, vec.data(), std::min(vec.size(), len));
            // Set address
            struct sockaddr_in address;
            address.sin_family = AF_INET6;
            address.sin_port = htons(static_cast<uint16_t>(5000));
            address.sin_addr.s_addr = htonl(static_cast<uint32_t>(1234567890));
            memset(address.sin_zero, '\0', sizeof(address.sin_zero));
            std::memcpy(
                addr, &address,
                std::min(static_cast<size_t>(*addrlen), sizeof(address)));
            address_length = *addrlen;
            *addrlen = static_cast<socklen_t>(
                           std::min(static_cast<size_t>(*addrlen),
                                    sizeof(address)));
            // Return number of received bytes.
            return std::min(vec.size(), len);
        });
        // Test
        auto [data, ip] = socket.receive(250ms);
        REQUIRE(data == std::vector<uint8_t>());
        REQUIRE(ip == IPAddress(0));
        // Verify poll system call.
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
        // Verify ioctl system call.
        fakeit::Verify(Method(mock_sys, ioctl).Matching(
                           [](auto fd, auto request, auto size)
        {
            (void)size;
            return fd == 3 && request == FIONREAD;
        })).Once();
        // Verify recvfrom.
        fakeit::Verify(Method(mock_sys, recvfrom).Matching(
                           [](auto fd, auto buf, auto len, auto flags,
                              auto addr, auto addrlen)
        {
            (void)buf;
            (void)addr;
            (void)addrlen;
            return fd == 3 && len >= 4 && flags == 0;
        })).Once();
        REQUIRE(address_length >= sizeof(sockaddr_in));
    }
    SECTION("Packet available (IP address truncated).")
    {
        // Mock poll system call.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock ioctl system call.
        fakeit::When(Method(mock_sys, ioctl)).Do(
            [](auto fd, auto request, auto size)
        {
            (void)fd;
            (void)request;
            *reinterpret_cast<int *>(size) = 4;
            return 0;
        });
        // Mock recvfrom.
        socklen_t address_length = 0;
        fakeit::When(Method(mock_sys, recvfrom)).Do(
            [&](auto fd, auto buf, auto len, auto flags,
                auto addr, auto addrlen)
        {
            (void)fd;
            (void)flags;
            // Write to buffer.
            std::vector<uint8_t> vec = {1, 3, 3, 7};
            std::memcpy(buf, vec.data(), std::min(vec.size(), len));
            // Set address
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(static_cast<uint16_t>(5000));
            address.sin_addr.s_addr = htonl(static_cast<uint32_t>(1234567890));
            memset(address.sin_zero, '\0', sizeof(address.sin_zero));
            std::memcpy(
                addr, &address,
                std::min(static_cast<size_t>(*addrlen), sizeof(address)));
            address_length = *addrlen;
            *addrlen = static_cast<socklen_t>(
                           std::min(static_cast<size_t>(*addrlen),
                                    sizeof(address))) + 1;
            // Return number of received bytes.
            return std::min(vec.size(), len);
        });
        // Test
        auto [data, ip] = socket.receive(250ms);
        REQUIRE(data == std::vector<uint8_t>());
        REQUIRE(ip == IPAddress(0));
        // Verify poll system call.
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
        // Verify ioctl system call.
        fakeit::Verify(Method(mock_sys, ioctl).Matching(
                           [](auto fd, auto request, auto size)
        {
            (void)size;
            return fd == 3 && request == FIONREAD;
        })).Once();
        // Verify recvfrom.
        fakeit::Verify(Method(mock_sys, recvfrom).Matching(
                           [](auto fd, auto buf, auto len, auto flags,
                              auto addr, auto addrlen)
        {
            (void)buf;
            (void)addr;
            (void)addrlen;
            return fd == 3 && len >= 4 && flags == 0;
        })).Once();
        REQUIRE(address_length >= sizeof(sockaddr_in));
    }
    SECTION("Emmits errors from 'recvfrom' system call.")
    {
        // Mock poll system call.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).AlwaysDo(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock ioctl system call.
        fakeit::When(Method(mock_sys, ioctl)).AlwaysDo(
            [](auto fd, auto request, auto size)
        {
            (void)fd;
            (void)request;
            *reinterpret_cast<int *>(size) = 4;
            return 0;
        });
        // Mock recvfrom.
        fakeit::When(Method(mock_sys, recvfrom)).AlwaysReturn(-1);
        // Test
        std::array<int, 13> errors{{
                EACCES,
                EWOULDBLOCK,
                EBADF,
                ECONNRESET,
                EINTR,
                EINVAL,
                ENOTCONN,
                ENOTSOCK,
                EOPNOTSUPP,
                ETIMEDOUT,
                EIO,
                ENOBUFS,
                ENOMEM
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(socket.receive(250ms), std::system_error);
        }
    }
    SECTION("Emmits errors from 'ioctl' system call.")
    {
        // Mock poll system call.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).AlwaysDo(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock ioctl system call.
        fakeit::When(Method(mock_sys, ioctl)).AlwaysReturn(-1);
        // Test
        std::array<int, 4> errors{{
                EBADF,
                EFAULT,
                EINVAL,
                ENOTTY
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(socket.receive(250ms), std::system_error);
        }
    }
    SECTION("Emmits errors from 'poll' system call.")
    {
        // Mock poll system call.
        fakeit::When(Method(mock_sys, poll)).AlwaysReturn(-1);
        // Test
        std::array<int, 4> errors{{
                EFAULT,
                EINTR,
                EINVAL,
                ENOMEM
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(socket.receive(250ms), std::system_error);
        }
    }
}
