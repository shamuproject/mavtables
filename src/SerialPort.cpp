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
#include <iterator>
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
 *  \param timeout How long to wait for data to arrive on the serial
 *      port.  The default is to not wait.
 *  \returns The data read from the serial port.
 */
std::vector<uint8_t> SerialPort::read(const std::chrono::nanoseconds &timeout)
{
    std::vector<uint8_t> vec;
    read(std::back_inserter(vec), timeout);
    return vec;
}


/** Write data to the serial port.
 *
 *  \param data The bytes to send.
 */
void SerialPort::write(const std::vector<uint8_t> &data)
{
    write(data.begin(), data.end());
}