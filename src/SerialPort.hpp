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


#ifndef SERIALPORT_HPP_
#define SERIALPORT_HPP_


#include <chrono>
#include <iterator>
#include <string>
#include <vector>


/** A serial port.
 */
class SerialPort
{
    public:
        /** Parity.
         */
        enum Parity
        {
            NONE,   //!< No parity.
            ODD,    //!< Odd parity, must have odd number of set bits.
            EVEN,   //!< Even parity, must have even number of set bits.
            MARK,   //!< Fill parity bit with 1.
            SPACE   //!< Fill parity bit with 0.
        };
        /** Feature bitflags.
         */
        enum Feature
        {
            DEFAULT = 0,        //!< No special features.
            FLOW_CONTROL = 1 << 0 //!< Enable flow control.
        };
        virtual ~SerialPort();
        virtual std::vector<uint8_t> read(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
        virtual void read(
            std::back_insert_iterator<std::vector<uint8_t>> it,
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
        virtual void write(const std::vector<uint8_t> &data);
        virtual void write(
            std::vector<uint8_t>::const_iterator first,
            std::vector<uint8_t>::const_iterator last);
};


#endif // SERIALPORT_HPP_
