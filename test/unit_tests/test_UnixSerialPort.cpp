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
#include <string>
#include <stdexcept>
#include <system_error>

#include <catch.hpp>
#include <errno.h>
#include <fakeit.hpp>

#include "PartialSendError.hpp"
#include "UnixSyscalls.hpp"
#include "UnixSerialPort.hpp"
#include "utility.hpp"

#include "common.hpp"


using namespace std::chrono_literals;


TEST_CASE("UnixSerialPort's open and configure a serial port on construction"
          "and closes the port on destruction.", "[UnixSerialPort]")
{
    SECTION("Without hardware flow control (no errors).")
    {
        // Mock system calls.
        fakeit::Mock<UnixSyscalls> mock_sys;
        // Mock 'open'.
        fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
        // Mock 'tcgetattr'.
        fakeit::When(Method(mock_sys, tcgetattr)).AlwaysDo(
            [&](auto fd, auto termios_p)
        {
            (void)fd;
            std::memset(termios_p, '\0', sizeof(struct termios));
            return 0;
        });
        // Mock 'tcsetattr'.
        struct termios tty;
        fakeit::When(Method(mock_sys, tcsetattr)).AlwaysDo(
            [&](auto fd, auto action, auto termios_p)
        {
            (void)fd;
            (void)action;
            std::memcpy(&tty, termios_p, sizeof(struct termios));
            return 0;
        });
        // Mock 'close'.
        fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
        {
            // Construct port.
            UnixSerialPort port(
                "/dev/ttyUSB0", 9600,
                SerialPort::DEFAULT,
                mock_unique(mock_sys));
            // Verify 'open' system call.
            fakeit::Verify(Method(mock_sys, open).Matching(
                               [](auto path, auto flags)
            {
                return std::string(path) == "/dev/ttyUSB0" &&
                       flags == (O_RDWR | O_NOCTTY | O_SYNC);
            })).Once();
            // Verify 'tcgetattr'.
            fakeit::Verify(Method(mock_sys, tcgetattr).Matching(
                               [&](auto fd, auto termios_p)
            {
                (void)termios_p;
                return fd == 3;
            })).Once();
            // Verify 'tcsetattr'.
            fakeit::Verify(Method(mock_sys, tcsetattr).Matching(
                               [](auto fd, auto action, auto termios_p)
            {
                (void)termios_p;
                return fd == 3 && action == TCSANOW;
            })).Once();
            REQUIRE(cfgetispeed(&tty) == B9600);
            REQUIRE(cfgetospeed(&tty) == B9600);
            REQUIRE((tty.c_cflag & CLOCAL) != 0);
            REQUIRE((tty.c_cflag & CREAD) != 0);
            REQUIRE((tty.c_cflag & PARENB) == 0);
            REQUIRE((tty.c_cflag & CSTOPB) == 0);
            REQUIRE((tty.c_cflag & CS8) != 0);
            REQUIRE((tty.c_cflag & CRTSCTS) == 0);
            REQUIRE((tty.c_lflag & ICANON) == 0);
            REQUIRE((tty.c_lflag & ECHO) == 0);
            REQUIRE((tty.c_lflag & ECHOE) == 0);
            REQUIRE((tty.c_lflag & ISIG) == 0);
            REQUIRE((tty.c_iflag & IGNBRK) == 0);
            REQUIRE((tty.c_iflag & BRKINT) == 0);
            REQUIRE((tty.c_iflag & PARMRK) == 0);
            REQUIRE((tty.c_iflag & ISTRIP) == 0);
            REQUIRE((tty.c_iflag & INLCR) == 0);
            REQUIRE((tty.c_iflag & IGNCR) == 0);
            REQUIRE((tty.c_iflag & ICRNL) == 0);
            REQUIRE((tty.c_iflag & IXON) == 0);
            REQUIRE((tty.c_iflag & IXOFF) == 0);
            REQUIRE((tty.c_iflag & IXANY) == 0);
            REQUIRE((tty.c_oflag & OPOST) == 0);
            REQUIRE(tty.c_cc[VMIN] == 0);
            REQUIRE(tty.c_cc[VTIME] == 0);
            // Verify 'close'.
            fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(0);
        }
        // Verify 'close'.
        fakeit::Verify(Method(mock_sys, close).Using(3)).Once();
    }
    SECTION("With hardware flow control (no errors).")
    {
        // Mock system calls.
        fakeit::Mock<UnixSyscalls> mock_sys;
        // Mock 'open'.
        fakeit::When(Method(mock_sys, open)).Return(3);
        // Mock 'tcgetattr'.
        fakeit::When(Method(mock_sys, tcgetattr)).Do(
            [&](auto fd, auto termios_p)
        {
            (void)fd;
            std::memset(termios_p, '\0', sizeof(struct termios));
            return 0;
        });
        // Mock 'tcsetattr'.
        struct termios tty;
        fakeit::When(Method(mock_sys, tcsetattr)).Do(
            [&](auto fd, auto action, auto termios_p)
        {
            (void)fd;
            (void)action;
            std::memcpy(&tty, termios_p, sizeof(struct termios));
            return 0;
        });
        // Mock 'close'.
        fakeit::When(Method(mock_sys, close)).Return(0);
        {
            // Construct port.
            UnixSerialPort port(
                "/dev/ttyUSB0", 9600,
                SerialPort::FLOW_CONTROL,
                mock_unique(mock_sys));
            // Verify 'open' system call.
            fakeit::Verify(Method(mock_sys, open).Matching(
                               [](auto path, auto flags)
            {
                return std::string(path) == "/dev/ttyUSB0" &&
                       flags == (O_RDWR | O_NOCTTY | O_SYNC);
            })).Once();
            // Verify 'tcgetattr'.
            fakeit::Verify(Method(mock_sys, tcgetattr).Matching(
                               [&](auto fd, auto termios_p)
            {
                (void)termios_p;
                return fd == 3;
            })).Once();
            // Verify 'tcsetattr'.
            fakeit::Verify(Method(mock_sys, tcsetattr).Matching(
                               [](auto fd, auto action, auto termios_p)
            {
                (void)termios_p;
                return fd == 3 && action == TCSANOW;
            })).Once();
            REQUIRE(cfgetispeed(&tty) == B9600);
            REQUIRE(cfgetospeed(&tty) == B9600);
            REQUIRE((tty.c_cflag & CLOCAL) != 0);
            REQUIRE((tty.c_cflag & CREAD) != 0);
            REQUIRE((tty.c_cflag & PARENB) == 0);
            REQUIRE((tty.c_cflag & CSTOPB) == 0);
            REQUIRE((tty.c_cflag & CS8) != 0);
            REQUIRE((tty.c_cflag & CRTSCTS) != 0);
            REQUIRE((tty.c_lflag & ICANON) == 0);
            REQUIRE((tty.c_lflag & ECHO) == 0);
            REQUIRE((tty.c_lflag & ECHOE) == 0);
            REQUIRE((tty.c_lflag & ISIG) == 0);
            REQUIRE((tty.c_iflag & IGNBRK) == 0);
            REQUIRE((tty.c_iflag & BRKINT) == 0);
            REQUIRE((tty.c_iflag & PARMRK) == 0);
            REQUIRE((tty.c_iflag & ISTRIP) == 0);
            REQUIRE((tty.c_iflag & INLCR) == 0);
            REQUIRE((tty.c_iflag & IGNCR) == 0);
            REQUIRE((tty.c_iflag & ICRNL) == 0);
            REQUIRE((tty.c_iflag & IXON) == 0);
            REQUIRE((tty.c_iflag & IXOFF) == 0);
            REQUIRE((tty.c_iflag & IXANY) == 0);
            REQUIRE((tty.c_oflag & OPOST) == 0);
            REQUIRE(tty.c_cc[VMIN] == 0);
            REQUIRE(tty.c_cc[VTIME] == 0);
            // Verify 'close'.
            fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(0);
        }
        // Verify 'close'.
        fakeit::Verify(Method(mock_sys, close).Using(3)).Once();
    }
    SECTION("Emmits errors from 'open' system call.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, open)).AlwaysReturn(-1);
        std::array<int, 26> errors{{
                EACCES,
                EDQUOT,
                EEXIST,
                EFAULT,
                EFBIG,
                EINTR,
                EINVAL,
                EISDIR,
                ELOOP,
                EMFILE,
                ENAMETOOLONG,
                ENFILE,
                ENODEV,
                ENOENT,
                ENOMEM,
                ENOSPC,
                ENOTDIR,
                ENXIO,
                EOPNOTSUPP,
                EOVERFLOW,
                EPERM,
                EROFS,
                ETXTBSY,
                EWOULDBLOCK,
                EBADF,
                ENOTDIR
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(UnixSerialPort(
                                  "/dev/ttyUSB0", 9600, SerialPort::DEFAULT,
                                  mock_unique(mock_sys)), std::system_error);
        }

        fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(0);
    }
    SECTION("Emmits errors from 'tcgetattr' system call, and closes the port.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
        fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
        fakeit::When(Method(mock_sys, tcgetattr)).AlwaysReturn(-1);
        std::array<int, 2> errors{{
                EBADF,
                ENOTTY
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(UnixSerialPort(
                                  "/dev/ttyUSB0", 9600, SerialPort::DEFAULT,
                                  mock_unique(mock_sys)), std::system_error);
        }

        fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(2);
    }
    SECTION("Emmits errors from 'tcsetattr' system call, and closes the port.")
    {
        fakeit::Mock<UnixSyscalls> mock_sys;
        fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
        fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
        fakeit::When(Method(mock_sys, tcgetattr)).AlwaysReturn(0);
        fakeit::When(Method(mock_sys, tcsetattr)).AlwaysReturn(-1);
        std::array<int, 5> errors{{
                EBADF,
                EINTR,
                EINVAL,
                ENOTTY,
                EIO
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(UnixSerialPort(
                                  "/dev/ttyUSB0", 9600, SerialPort::DEFAULT,
                                  mock_unique(mock_sys)), std::system_error);
        }

        fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(5);
    }
}


TEST_CASE("UnixSerialPort's open method configures the baud rate.",
          "[UnixSerialPort]")
{
    // Mock system calls.
    fakeit::Mock<UnixSyscalls> mock_sys;
    // Mock 'open'.
    fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
    // Mock 'tcgetattr'.
    fakeit::When(Method(mock_sys, tcgetattr)).AlwaysDo(
        [&](auto fd, auto termios_p)
    {
        (void)fd;
        std::memset(termios_p, '\0', sizeof(struct termios));
        return 0;
    });
    // Mock 'tcsetattr'.
    struct termios tty;
    fakeit::When(Method(mock_sys, tcsetattr)).AlwaysDo(
        [&](auto fd, auto action, auto termios_p)
    {
        (void)fd;
        (void)action;
        std::memcpy(&tty, termios_p, sizeof(struct termios));
        return 0;
    });
    // Mock 'close'.
    fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
    SECTION("0 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 0,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B0);
        REQUIRE(cfgetospeed(&tty) == B0);
    }
    SECTION("50 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 50,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B50);
        REQUIRE(cfgetospeed(&tty) == B50);
    }
    SECTION("75 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 75,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B75);
        REQUIRE(cfgetospeed(&tty) == B75);
    }
    SECTION("110 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 110,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B110);
        REQUIRE(cfgetospeed(&tty) == B110);
    }
    SECTION("134 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 134,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B134);
        REQUIRE(cfgetospeed(&tty) == B134);
    }
    SECTION("135 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 135,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B134);
        REQUIRE(cfgetospeed(&tty) == B134);
    }
    SECTION("150 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 150,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B150);
        REQUIRE(cfgetospeed(&tty) == B150);
    }
    SECTION("200 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 200,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B200);
        REQUIRE(cfgetospeed(&tty) == B200);
    }
    SECTION("300 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 300,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B300);
        REQUIRE(cfgetospeed(&tty) == B300);
    }
    SECTION("600 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 600,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B600);
        REQUIRE(cfgetospeed(&tty) == B600);
    }
    SECTION("1200 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 1200,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B1200);
        REQUIRE(cfgetospeed(&tty) == B1200);
    }
    SECTION("1800 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 1800,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B1800);
        REQUIRE(cfgetospeed(&tty) == B1800);
    }
    SECTION("2400 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 2400,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B2400);
        REQUIRE(cfgetospeed(&tty) == B2400);
    }
    SECTION("4800 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 4800,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B4800);
        REQUIRE(cfgetospeed(&tty) == B4800);
    }
    SECTION("9600 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 9600,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B9600);
        REQUIRE(cfgetospeed(&tty) == B9600);
    }
    SECTION("19200 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 19200,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B19200);
        REQUIRE(cfgetospeed(&tty) == B19200);
    }
    SECTION("38400 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 38400,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B38400);
        REQUIRE(cfgetospeed(&tty) == B38400);
    }
    SECTION("57600 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 57600,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B57600);
        REQUIRE(cfgetospeed(&tty) == B57600);
    }
    SECTION("115200 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 115200,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B115200);
        REQUIRE(cfgetospeed(&tty) == B115200);
    }
    SECTION("230400 bps")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 230400,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(cfgetispeed(&tty) == B230400);
        REQUIRE(cfgetospeed(&tty) == B230400);
    }
    SECTION("Throws error when given unsupported baud rate.")
    {
        REQUIRE_THROWS_AS(
            UnixSerialPort(
                "/dev/ttyUSB0", 9601, SerialPort::DEFAULT,
                mock_unique(mock_sys)), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            UnixSerialPort(
                "/dev/ttyUSB0", 9601, SerialPort::DEFAULT,
                mock_unique(mock_sys)), "9601 bps is not a valid baud rate.");
    }
}


TEST_CASE("UnixSerialPort's 'read' method receives data on the socket.",
          "[UnixSerialPort]")
{
    // Mock system calls.
    fakeit::Mock<UnixSyscalls> mock_sys;
    // Mock 'open'.
    fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
    // Mock 'tcgetattr'.
    fakeit::When(Method(mock_sys, tcgetattr)).AlwaysReturn(0);
    // Mock 'tcsetattr'.
    fakeit::When(Method(mock_sys, tcsetattr)).AlwaysReturn(0);
    // Mock 'close'.
    fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
    // Construct port.
    UnixSerialPort port(
        "/dev/ttyUSB0", 9600,
        SerialPort::DEFAULT,
        mock_unique(mock_sys));
    SECTION("Timeout, no data (no errors).")
    {
        // Mock 'poll'.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            return 0;
        });
        // Test.
        REQUIRE(port.read(250ms) == std::vector<uint8_t>());
        // Verify 'poll'.
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
    SECTION("Poll error, close and reopen the serial port (no other errors).")
    {
        // Mock 'tcgetattr'.
        fakeit::When(Method(mock_sys, tcgetattr)).AlwaysDo(
            [&](auto fd, auto termios_p)
        {
            (void)fd;
            std::memset(termios_p, '\0', sizeof(struct termios));
            return 0;
        });
        // Mock 'tcsetattr'.
        struct termios tty;
        fakeit::When(Method(mock_sys, tcsetattr)).AlwaysDo(
            [&](auto fd, auto action, auto termios_p)
        {
            (void)fd;
            (void)action;
            std::memcpy(&tty, termios_p, sizeof(struct termios));
            return 0;
        });
        // Mock 'poll'.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).AlwaysDo(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLERR;
            return 1;
        });
        // Test
        REQUIRE(port.read(250ms) == std::vector<uint8_t>());
        // Verify 'open' system call.
        fakeit::Verify(Method(mock_sys, open).Matching(
                           [](auto path, auto flags)
        {
            return std::string(path) == "/dev/ttyUSB0" &&
                   flags == (O_RDWR | O_NOCTTY | O_SYNC);
        })).Exactly(2);
        // Verify 'tcgetattr'.
        fakeit::Verify(Method(mock_sys, tcgetattr).Matching(
                           [&](auto fd, auto termios_p)
        {
            (void)termios_p;
            return fd == 3;
        })).Exactly(2);
        // Verify 'tcsetattr'.
        fakeit::Verify(Method(mock_sys, tcsetattr).Matching(
                           [](auto fd, auto action, auto termios_p)
        {
            (void)termios_p;
            return fd == 3 && action == TCSANOW;
        })).Exactly(2);
        REQUIRE(cfgetispeed(&tty) == B9600);
        REQUIRE(cfgetospeed(&tty) == B9600);
        REQUIRE((tty.c_cflag & CLOCAL) != 0);
        REQUIRE((tty.c_cflag & CREAD) != 0);
        REQUIRE((tty.c_cflag & PARENB) == 0);
        REQUIRE((tty.c_cflag & CSTOPB) == 0);
        REQUIRE((tty.c_cflag & CS8) != 0);
        REQUIRE((tty.c_cflag & CRTSCTS) == 0);
        REQUIRE((tty.c_lflag & ICANON) == 0);
        REQUIRE((tty.c_lflag & ECHO) == 0);
        REQUIRE((tty.c_lflag & ECHOE) == 0);
        REQUIRE((tty.c_lflag & ISIG) == 0);
        REQUIRE((tty.c_iflag & IGNBRK) == 0);
        REQUIRE((tty.c_iflag & BRKINT) == 0);
        REQUIRE((tty.c_iflag & PARMRK) == 0);
        REQUIRE((tty.c_iflag & ISTRIP) == 0);
        REQUIRE((tty.c_iflag & INLCR) == 0);
        REQUIRE((tty.c_iflag & IGNCR) == 0);
        REQUIRE((tty.c_iflag & ICRNL) == 0);
        REQUIRE((tty.c_iflag & IXON) == 0);
        REQUIRE((tty.c_iflag & IXOFF) == 0);
        REQUIRE((tty.c_iflag & IXANY) == 0);
        REQUIRE((tty.c_oflag & OPOST) == 0);
        REQUIRE(tty.c_cc[VMIN] == 0);
        REQUIRE(tty.c_cc[VTIME] == 0);
        // Verify 'close'.
        fakeit::Verify(Method(mock_sys, close).Using(3)).Exactly(1);
    }
    SECTION("Data available (no errors).")
    {
        // Mock 'poll'.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).Do(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock 'read'.
        fakeit::When(Method(mock_sys, read)).Do(
            [](auto fd, auto buf, auto count)
        {
            (void)fd;
            // Write to buffer.
            std::vector<uint8_t> vec = {1, 3, 3, 7};
            std::memcpy(buf, vec.data(), std::min(vec.size(), count));
            // Return number of received bytes.
            return std::min(vec.size(), count);
        });
        // Test.
        REQUIRE(port.read(250ms) == std::vector<uint8_t>({1, 3, 3, 7}));
        // Verify 'poll'.
        fakeit::Verify(Method(mock_sys, poll).Matching(
                           [](auto fds_, auto nfds, auto timeout)
        {
            (void)fds_;
            return nfds == 1 && timeout == 250;
        })).Once();
        REQUIRE(fds.fd == 3);
        REQUIRE(fds.events == POLLIN);
        REQUIRE(fds.revents == 0);
        // Verify 'read'.
        fakeit::Verify(Method(mock_sys, read).Matching(
                           [](auto fd, auto buf, auto count)
        {
            (void)buf;
            return fd == 3 && count >= 1024;
        }));
    }
    SECTION("Emmits errors from 'read' system call.")
    {
        // Mock 'poll'.
        struct pollfd fds;
        fakeit::When(Method(mock_sys, poll)).AlwaysDo(
            [&](auto fds_, auto nfds, auto timeout)
        {
            (void)timeout;
            std::memcpy(&fds, fds_, nfds * sizeof(fds));
            fds_->revents = POLLIN;
            return 1;
        });
        // Mock 'read'.
        fakeit::When(Method(mock_sys, read)).AlwaysReturn(-1);
        // Test
        std::array<int, 7> errors{{
                EAGAIN,
                EBADF,
                EFAULT,
                EINTR,
                EINVAL,
                EIO,
                EISDIR
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(port.read(250ms), std::system_error);
        }
    }
    SECTION("Emmits errors from 'poll' system call.")
    {
        // Mock 'poll'.
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
            REQUIRE_THROWS_AS(port.read(250ms), std::system_error);
        }
    }
}


TEST_CASE("UnixSerialPort's 'write' method sends data over the serial port.",
          "[UnixSerialPort]")
{
    // Mock system calls.
    fakeit::Mock<UnixSyscalls> mock_sys;
    // Mock 'open'.
    fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
    // Mock 'tcgetattr'.
    fakeit::When(Method(mock_sys, tcgetattr)).AlwaysReturn(0);
    // Mock 'tcsetattr'.
    fakeit::When(Method(mock_sys, tcsetattr)).AlwaysReturn(0);
    // Mock 'close'.
    fakeit::When(Method(mock_sys, close)).Return(0);
    // Construct port.
    UnixSerialPort port(
        "/dev/ttyUSB0", 9600,
        SerialPort::DEFAULT,
        mock_unique(mock_sys));
    SECTION("Without error.")
    {
        // Mock 'write'.
        std::vector<uint8_t> written;
        fakeit::When(Method(mock_sys, write)).Do(
            [&](auto fd, auto buf, auto count)
        {
            (void)fd;
            written.resize(count);
            std::memcpy(written.data(), buf, count);
            return count;
        });
        // Test
        std::vector<uint8_t> vec = {1, 3, 3, 7};
        port.write(vec);
        // Verify 'write'.
        fakeit::Verify(Method(mock_sys, write).Matching(
                           [](auto fd, auto buf, auto count)
        {
            (void)buf;
            return fd == 3 && count == 4;
        })).Once();
        REQUIRE(written == vec);
    }
    SECTION("Could not write all data.")
    {
        // Mock 'write'.
        fakeit::When(Method(mock_sys, write)).AlwaysReturn(3);
        // Test
        std::vector<uint8_t> vec = {1, 3, 3, 7};
        REQUIRE_THROWS_AS(port.write(vec), PartialSendError);
        REQUIRE_THROWS_WITH(port.write(vec), "Could only write 3 of 4 bytes.");
    }
    SECTION("Emits errors from 'write' system call.")
    {
        fakeit::When(Method(mock_sys, write)).AlwaysReturn(-1);
        std::array<int, 11> errors{{
                EAGAIN,
                EBADF,
                EDESTADDRREQ,
                EDQUOT,
                EFAULT,
                EFBIG,
                EINTR,
                EINVAL,
                EIO,
                ENOSPC,
                EPIPE
            }};

        for (auto error : errors)
        {
            errno = error;
            REQUIRE_THROWS_AS(port.write({1, 3, 3, 7}), std::system_error);
        }
    }
}


TEST_CASE("UnixSerialPort's are printable.", "[UnixSerialPort]")
{
    // Mock system calls.
    fakeit::Mock<UnixSyscalls> mock_sys;
    // Mock 'open'.
    fakeit::When(Method(mock_sys, open)).AlwaysReturn(3);
    // Mock 'tcgetattr'.
    fakeit::When(Method(mock_sys, tcgetattr)).AlwaysDo(
        [&](auto fd, auto termios_p)
    {
        (void)fd;
        std::memset(termios_p, '\0', sizeof(struct termios));
        return 0;
    });
    // Mock 'tcsetattr'.
    struct termios tty;
    fakeit::When(Method(mock_sys, tcsetattr)).AlwaysDo(
        [&](auto fd, auto action, auto termios_p)
    {
        (void)fd;
        (void)action;
        std::memcpy(&tty, termios_p, sizeof(struct termios));
        return 0;
    });
    // Mock 'close'.
    fakeit::When(Method(mock_sys, close)).AlwaysReturn(0);
    SECTION("Without flow control.")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 9600,
            SerialPort::DEFAULT,
            mock_unique(mock_sys));
        REQUIRE(
            str(port) ==
            "serial {\n"
            "    device /dev/ttyUSB0;\n"
            "    baudrate 9600;\n"
            "    flow_control no;\n"
            "}");
    }
    SECTION("With flow control.")
    {
        UnixSerialPort port(
            "/dev/ttyUSB0", 9600,
            SerialPort::FLOW_CONTROL,
            mock_unique(mock_sys));
        REQUIRE(
            str(port) ==
            "serial {\n"
            "    device /dev/ttyUSB0;\n"
            "    baudrate 9600;\n"
            "    flow_control yes;\n"
            "}");
    }
}
