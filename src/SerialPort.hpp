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


#include <string>



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
        /** Construct a serial port.
         *
         *  \param device The string representing the serial port.  This is
         *      operating system specific.
         *  \param buad_rate Bits per second, the default value is 9600 bps.
         *  \param data_bits Length (in bits) of the data portion of each
         *      character.
         *  \param parity The parity to use, the default is no parity.
         *  \param stop_bits The number of stop bits, the default is 1 stop bit.
         *  \param features A bitflag of the features to enable, default is not
         *      to enable any features.
         */
        SerialPort(
            std::string device,
            unsigned long buad_rate = 9600,
            unsigned int data_bits = 8,
            SerialPort::Parity parity = SerialPort::NONE,
            float stop_bits = 1,
            SerialPort::Feature features = SerialPort::DEFAULT);
        virtual ~SerialPort();
        std::vector<uint8_t> read(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
        /** Read data from the serial port.
         *
         *  \param it A back insert iterator to read bytes into.
         *  \param timeout How long to wait for data to arrive on the serial
         *      port.  The default is to not wait.
         */
        virtual void read(
            std::back_insert_iterator<std::vector<uint8_t>> it,
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero()) = 0;
        void write(const std::vector<uint8_t> &data);
        /** Write data to the serial port.
         *
         *  \param first Iterator to first byte in range to send.
         *  \param last Iterator to one past the last byte to send.
         */
        virtual void write(
            std::vector<uint8_t>::const_iterator first,
            std::vector<uint8_t>::const_iterator last) = 0;
};


#endif // SERIALPORT_HPP_
