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


#ifndef SERIALINTERFACE_HPP_
#define SERIALINTERFACE_HPP_


#include <chrono>
#include <memory>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "PacketParser.hpp"
#include "SerialPort.hpp"


class SerialInterface : public Interface
{
    public:
        SerialInterface(
            std::unique_ptr<SerialPort> port,
            std::shared_ptr<ConnectionPool> connection_pool,
            std::unique_ptr<Connection> connection);
        // LCOV_EXCL_START
        ~SerialInterface() = default;
        // LCOV_EXCL_STOP
        void send_packet(const std::chrono::nanoseconds &timeout) final;
        void receive_packet(const std::chrono::nanoseconds &timeout) final;

    protected:
        std::ostream &print_(std::ostream &os) const final;

    private:
        // Variables.
        std::unique_ptr<SerialPort> port_;
        std::shared_ptr<ConnectionPool> connection_pool_;
        std::shared_ptr<Connection> connection_;
        PacketParser parser_;
};


#endif // SERIALINTERFACE_HPP_
