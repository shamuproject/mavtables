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
#include <system_error>
#include <utility>
#include <vector>


#include "PartialSendError.hpp"
#include "UnixSerialPort.hpp"
#include "UnixSyscalls.hpp"


/** Construct a serial port.
 *
 *  \param device The string representing the serial port.  For example
 *      "/dev/ttyUSB0".
 *  \param baud_rate Bits per second, the default value is 9600 bps.
 *  \param features A bitflag of the features to enable, default is not
 *      to enable any features.  See \ref SerialPort::Feature for flags.
 *  \param syscalls The object to use for Unix system calls.  It is default
 *      constructed to the production implementation.
 */
UnixSerialPort::UnixSerialPort(
    std::string device,
    unsigned long baud_rate,
    SerialPort::Feature features,
    std::unique_ptr<UnixSyscalls> syscalls)
    : device_(std::move(device)), baud_rate_(baud_rate),
      features_(features), syscalls_(std::move(syscalls)), port_(-1)
{
    open_port_();
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
    std::chrono::milliseconds timeout_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
    struct pollfd fds = {port_, POLLIN, 0};
    auto result = syscalls_->poll(
                      &fds, 1, static_cast<int>(timeout_ms.count()));

    // Poll error
    if (result < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
    // Success
    else if (result > 0)
    {
        // Port error
        if (fds.revents & POLLERR)
        {
            syscalls_->close(port_);
            open_port_();
            return std::vector<uint8_t>();
        }
        // Data available for reading.
        else if (fds.revents & POLLIN)
        {
            return read_();
        }
    }

    // Timed out
    return std::vector<uint8_t>();
}


/** \copydoc SerialPort::write(const std::vector<uint8_t> &)
 */
void UnixSerialPort::write(const std::vector<uint8_t> &data)
{
    // Write the data.
    auto err = syscalls_->write(port_, data.data(), data.size());

    // Handle system call errors.
    if (err < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Could not write all data.
    if (static_cast<size_t>(err) < data.size())
    {
        throw PartialSendError(static_cast<unsigned long>(err), data.size());
    }
}

/** Configure serial port.
 *
 *  \param baud_rate The bitrate to configure for the port.
 *  \param features A bitmask representing the features to use.  See \ref
 *      SerialPort::Feature for documentation.
 *  \throws std::invalid_argument if the baud rate is not supported.
 *  \throws std::system_error if a system call produces an error.
 */
void UnixSerialPort::configure_port_(
    unsigned long baud_rate, SerialPort::Feature features)
{
    // Get attribute structure.
    struct termios tty;

    if (syscalls_->tcgetattr(port_, &tty) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Set baud rate.
    speed_t speed = speed_constant_(baud_rate);

    if (cfsetispeed(&tty, speed) < 0)
    {
        // This is unreachable assuming the speed_constant_ method is properly
        // written.
        // LCOV_EXCL_START
        throw std::system_error(std::error_code(errno, std::system_category()));
        // LCOV_EXCL_STOP
    }

    if (cfsetospeed(&tty, speed) < 0)
    {
        // This is unreachable assuming the speed_constant_ method is properly
        // written.
        // LCOV_EXCL_START
        throw std::system_error(std::error_code(errno, std::system_category()));
        // LCOV_EXCL_STOP
    }

    // Enable receiver and set local mode.
    tty.c_cflag |= static_cast<tcflag_t>(CLOCAL | CREAD);
    // Use 8N1 mode (8 data bits, no parity, 1 stop bit).
    tty.c_cflag &= static_cast<tcflag_t>(~PARENB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSTOPB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSIZE);
    tty.c_cflag |= static_cast<tcflag_t>(CS8);

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
    tty.c_iflag &= static_cast<tcflag_t>(~(IGNBRK | BRKINT | PARMRK | ISTRIP));
    tty.c_iflag &= static_cast<tcflag_t>(~(INLCR | IGNCR | ICRNL));
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


/** Open the serial port.
 */
void UnixSerialPort::open_port_()
{
    port_ = -1;
    // Open serial port.
    port_ = syscalls_->open(device_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    if (port_ < 0)
    {
        throw std::system_error(
            std::error_code(errno, std::system_category()),
            "Failed to open \"" + device_ + "\".");
    }

    // Configure serial port.
    try
    {
        configure_port_(baud_rate_, features_);
    }
    catch (...)
    {
        syscalls_->close(port_);
        throw;
    }
}


/** Read data from serial port.
 *
 *  \note There must be data to read, otherwise calling this method is
 *      undefined.
 *
 *  \returns The data read from the port, up to 1024 bytes at a time.
 */
std::vector<uint8_t> UnixSerialPort::read_()
{
    std::vector<uint8_t> buffer;
    buffer.resize(1024);
    auto size = syscalls_->read(port_, buffer.data(), buffer.size());

    if (size < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    buffer.resize(static_cast<size_t>(size));
    return buffer;
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

        case 115200:
            return B115200;

        case 230400:
            return B230400;

        default:
            throw std::invalid_argument(
                std::to_string(baud_rate) + " bps is not a valid baud rate.");
    }
}

/** \copydoc SerialPort::print_(std::ostream &os)const
 */
std::ostream &UnixSerialPort::print_(std::ostream &os) const
{
    os << "serial {" << std::endl;
    os << "    device " << device_ << ";" << std::endl;
    os << "    baudrate " << std::to_string(baud_rate_) << ";" << std::endl;
    if ((features_ & SerialPort::FLOW_CONTROL) != 0)
    {
        os << "    flow_control yes;" << std::endl;
    }
    else
    {
        os << "    flow_control no;" << std::endl;
    }
    os << "}";
    return os;
}
