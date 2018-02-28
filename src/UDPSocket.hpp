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


#ifndef UDPSOCKET_HPP_
#define UDPSOCKET_HPP_


#include <chrono>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "IPAddress.hpp"


/** A UDP socket, listening on a port/address combination.
 *
 *  One of the \ref send methods and one of the \ref receive methods must be
 *  implemented in derived classes to avoid infinite recursion because this is
 *  technically and abstract class.  The other send and receive methods will be
 *  defaulted.
 */
class UDPSocket
{
    public:
        virtual ~UDPSocket();
        virtual void send(
            const std::vector<uint8_t> &data, const IPAddress &address);
        virtual void send(
            std::vector<uint8_t>::const_iterator first,
            std::vector<uint8_t>::const_iterator last,
            const IPAddress &address);
        virtual std::pair<std::vector<uint8_t>, IPAddress> receive(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
        virtual IPAddress receive(
            std::back_insert_iterator<std::vector<uint8_t>> it,
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
};


#endif // UDPSOCKET_HPP_
