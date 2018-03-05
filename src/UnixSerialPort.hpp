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


#ifndef UNIXSERIALPORT_HPP_
#define UNIXSERIALPORT_HPP_


#include <chrono>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "SerialPort.hpp"
#include "UnixSyscalls.hpp"


/** A serial port.
 */
class UnixSerialPort : public SerialPort
{
    public:
        UnixSerialPort(
            std::string device,
            unsigned long buad_rate = 9600,
            SerialPort::Feature features = SerialPort::DEFAULT,
            std::unique_ptr<UnixSyscalls> syscalls =
                std::make_unique<UnixSyscalls>());
        virtual ~UnixSerialPort();
        virtual std::vector<uint8_t> read(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero()) final;
        virtual void write(const std::vector<uint8_t> &data) final;

    private:
        // Variables
        std::unique_ptr<UnixSyscalls> syscalls_;
        int port_;
        // Methods
        void configure_port_(
            unsigned long buad_rate, SerialPort::Feature features);
        speed_t speed_constant_(unsigned long buad_rate);
};


#endif // UNIXSERIALPORT_HPP_
