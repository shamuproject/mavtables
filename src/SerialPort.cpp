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
#include <ostream>
#include <utility>
#include <vector>

#include "SerialPort.hpp"


// Placed here to avoid weak-vtables error.
// LCOV_EXCL_START
SerialPort::~SerialPort()
{
}
// LCOV_EXCL_STOP


/** Read data from the serial port.
 *
 *  \note The \p timeout is not guaranteed to be up to nanosecond precision, the
 *      actual precision is up to the operating system's implementation but is
 *      guaranteed to have at least millisecond precision.
 *
 *  \param timeout How long to wait for data to arrive on the serial port if
 *      there is not already data to read.  The default is to not wait.
 *  \returns The data read from the serial port.
 */
std::vector<uint8_t> SerialPort::read(const std::chrono::nanoseconds &timeout)
{
    std::vector<uint8_t> vec;
    read(std::back_inserter(vec), timeout);
    return vec;
}


/** Read data from the serial port.
 *
 *  \note The \p timeout is not guaranteed to be up to nanosecond precision, the
 *      actual precision is up to the operating system's implementation but is
 *      guaranteed to have at least millisecond precision.
 *
 *  \param it A back insert iterator to read bytes into.
 *  \param timeout How long to wait for data to arrive on the serial port if
 *      there is not already data to read.  The default is to not wait.
 */
void SerialPort::read(
    std::back_insert_iterator<std::vector<uint8_t>> it,
    const std::chrono::nanoseconds &timeout)
{
    auto vec = read(timeout);
    std::copy(vec.begin(), vec.end(), it);
}


/** Write data to the serial port (blocking write).
 *
 *  \param data The bytes to send.
 */
void SerialPort::write(const std::vector<uint8_t> &data)
{
    write(data.begin(), data.end());
}


/** Write data to the serial port (blocking write).
 *
 *  \param first Iterator to first byte in range to send.
 *  \param last Iterator to one past the last byte to send.
 */
void SerialPort::write(
    std::vector<uint8_t>::const_iterator first,
    std::vector<uint8_t>::const_iterator last)
{
    std::vector<uint8_t> vec;
    std::copy(first, last, std::back_inserter(vec));
    write(vec);
}


/** Print the serial port to the given output stream.
 *
 *  \param os The output stream to print to.
 *  \returns The output stream.
 */
std::ostream &SerialPort::print_(std::ostream &os) const
{
    os << "unknown serial port";
    return os;
}


/** Print the given serial port to the given output stream.
 *
 *  \note This is a polymorphic print, it will work on any child of \ref
 *      SerialPort even if the pointer/reference is to the base class \ref
 *      SerialPort.
 *
 *  An example:
 *  ```
 *  serial {
 *      device /dev/ttyUSB0;
 *      baudrate 115200;
 *      flow_control yes;
 *  }
 *  ```
 *
 *  The base \ref SerialPort class will print:
 *  ```
 *  unknown serial port
 *  ```

 *  \relates SerialPort
 *  \param os The output stream to print to.
 *  \param serial_port The serial port (or any child of \ref SerialPort) to
 *      print.
 *  \returns The output stream.
 */
std::ostream &operator<<(std::ostream &os, const SerialPort &serial_port)
{
    return serial_port.print_(os);
}
