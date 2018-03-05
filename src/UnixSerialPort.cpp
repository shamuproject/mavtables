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
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


#include "UnixSerialPort.hpp"
#include "UnixSyscalls.hpp"


/** Construct a serial port.
 *
 *  \param device The string representing the serial port.  For example
 *      "/dev/ttyUSB0".
 *  \param buad_rate Bits per second, the default value is 9600 bps.
 *  \param features A bitflag of the features to enable, default is not
 *      to enable any features.  See \ref SerialPort::Feature for flags.
 */
UnixSerialPort::UnixSerialPort(
    std::string device,
    unsigned long buad_rate,
    SerialPort::Feature features,
    std::unique_ptr<UnixSyscalls> syscalls)
    : syscalls_(std::move(syscalls)), port_(-1)
{
    port_ = -1;
    // Open serial port.
    port_ = syscalls_->open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    if (port_ < 0)
    {
        throw std::system_error(
            std::error_code(errno, std::system_category()),
            "Failed to open \"" + device + "\".");
    }

    // Configure serial port.
    try
    {
        configure_port_(buad_rate, features);
    }
    catch (...)
    {
        syscalls_->close(port_);
        throw;
    }
}


/** The port destructor.
 *
 *  This closes the underlying file descriptor.
 */
// LCOV_EXCL_START
UnixSerialPort::~UnixSerialPort()
{
    syscalls_->close(port_);
}
// LCOV_EXCL_STOP


/** \copydoc SerialPort::read(const std::chrono::nanoseconds &)
 *
 *  \note The timeout precision of this implementation is 1 millisecond.
 */
std::vector<uint8_t> UnixSerialPort::read(
    const std::chrono::nanoseconds &timeout)
{
    (void)timeout;
    return {};
}


/** \copydoc SerialPort::write(const std::vector<uint8_t> &)
 */
void UnixSerialPort::write(const std::vector<uint8_t> &data)
{
    // Write the data.
    auto err = syscalls_->write(port_, data.data(), data.size());

    if (err < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    if (static_cast<size_t>(err) < data.size())
    {
        throw std::runtime_error(
            "Could only write " + std::to_string(err) + " of " +
            std::to_string(data.size()) + " bytes.");
    }
}


/** Configure serial port.
 *
 *  \param buad_rate The bitrate to configure for the port.
 *  \param features A bitmask representing the features to use.  See \ref
 *      SerialPort::Feature for documentation.
 *  \throws std::invalid_argument if the baud rate is not supported.
 *  \throws std::system_error if a system call produces an error.
 */
void UnixSerialPort::configure_port_(
    unsigned long buad_rate, SerialPort::Feature features)
{
    // Get attribute structure.
    struct termios tty;

    if (syscalls_->tcgetattr(port_, &tty) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Set baud rate.
    speed_t speed = speed_constant_(buad_rate);

    if (cfsetispeed(&tty, speed) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    if (cfsetospeed(&tty, speed) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Enable receiver and set local mode.
    tty.c_cflag |= static_cast<tcflag_t>(CLOCAL | CREAD);
    // Use 8N1 mode (8 data bits, no parity, 1 stop bit).
    tty.c_cflag &= static_cast<tcflag_t>(~PARENB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSTOPB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSIZE);

    // Enable/disable hardware flow control.
    if (features & SerialPort::FLOW_CONTROL)
    {
        tty.c_cflag |= static_cast<tcflag_t>(CRTSCTS);
    }
    else
    {
        tty.c_cflag &= static_cast<tcflag_t>(~CRTSCTS);
    }

    // Use raw input.
    tty.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO | ECHOE | ISIG));
    // Disable software flow control.
    tty.c_iflag &= static_cast<tcflag_t>(~(IXON | IXOFF | IXANY));
    // Use raw output.
    tty.c_oflag &= static_cast<tcflag_t>(~OPOST);
    // Non blocking mode, using poll.
    // See: http://unixwiz.net/techtips/termios-vmin-vtime.html
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    // Apply settings to serial port.
    if (syscalls_->tcsetattr(port_, TCSANOW, &tty) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
}


/** Convert integer to termios baud rate constant.
 *
 *  See \ref UnixSerialPort::UnixSerialPort for valid baud rates.
 *
 *  \param baud_rate The baud rate to convert.
 *  \returns The baud rate constant.
 *  \throws std::invalid_argument if the given buad rate is not supported.
 */
speed_t UnixSerialPort::speed_constant_(unsigned long baud_rate)
{
    switch (baud_rate)
    {
        case 0:
            return B0;

        case 50:
            return B50;

        case 75:
            return B75;

        case 110:
            return B110;

        case 134: // actually 134.5
        case 135: // actually 134.5
            return B134;

        case 150:
            return B150;

        case 200:
            return B200;

        case 300:
            return B300;

        case 600:
            return B600;

        case 1200:
            return B1200;

        case 1800:
            return B1800;

        case 2400:
            return B2400;

        case 4800:
            return B4800;

        case 9600:
            return B9600;

        case 19200:
            return B19200;

        case 38400:
            return B38400;

        case 57600:
            return B57600;

        case 76800:
            return B57600;

        case 115200:
            return B115200;

        default:
            throw std::invalid_argument(
                std::to_string(baud_rate) + "bps is not a valid baud rate.");
    }
}
