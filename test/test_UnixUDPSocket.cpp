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
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).Return(4);
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
            UnixUDPSocket socket(14050, {}, mock_unique(mock_sys));
            fakeit::Verify(Method(mock_sys, socket).Matching(
                    [&](auto family, auto type, auto protocol)
            {
                return family == AF_INET && type == SOCK_DGRAM && protocol == 0;
            })).Once();
            fakeit::Verify(Method(mock_sys, bind).Matching(
                        [](auto fd, auto addr, auto addrlen)
            {
                (void)addr;
                return fd == 4 && addrlen == sizeof(address);
            })).Once();
            REQUIRE(address.sin_family == AF_INET);
            REQUIRE(ntohs(address.sin_port) == 14050);
            REQUIRE(ntohl(address.sin_addr.s_addr) == INADDR_ANY);
            for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
            {
                REQUIRE(address.sin_zero[i] == '\0');
            }
            fakeit::Verify(Method(mock_sys, close).Using(4)).Exactly(0);
        }
        fakeit::Verify(Method(mock_sys, close).Using(4)).Once();
    }
    SECTION("With a specific IP address (no errors).")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).Return(4);
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
                return fd == 4 && addrlen == sizeof(address);
            })).Once();
            REQUIRE(address.sin_family == AF_INET);
            REQUIRE(ntohs(address.sin_port) == 14050);
            REQUIRE(ntohl(address.sin_addr.s_addr) == 1234567890);
            for (size_t i = 0; i < sizeof(address.sin_zero); ++i)
            {
                REQUIRE(address.sin_zero[i] == '\0');
            }
            fakeit::Verify(Method(mock_sys, close).Using(4)).Exactly(0);
        }
        fakeit::Verify(Method(mock_sys, close).Using(4)).Once();
    }
    SECTION("Emmits errors from 'socket' system call.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, socket)).AlwaysReturn(-1);
        std::array<int, 13> errors{{
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
