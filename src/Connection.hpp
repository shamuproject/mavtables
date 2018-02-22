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


#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_


#include <memory>

#include "AddressPool.hpp"
#include "config.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketQueue.hpp"


/** Represents a connection that packets can be sent over.
 *
 *  The connection class does not actually send anything.  It filters and sorts
 *  packets in a queue for sending by an \ref Interface.  It also maintains a
 *  list of addresses reachable on this connection.
 */
class Connection
{
    public:
        Connection(
            std::shared_ptr<Filter> filter, std::unique_ptr<AddressPool<>> pool,
            std::unique_ptr<PacketQueue> queue, bool mirror = false);
        // LCOV_EXCL_START
        TEST_VIRTUAL ~Connection() = default;
        // LCOV_EXCL_STOP
        void add_address(MAVAddress address);
        std::shared_ptr<const Packet> next_packet(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds(0));
        TEST_VIRTUAL void send(std::shared_ptr<const Packet> packet);

    private:
        // Variables
        std::shared_ptr<Filter> filter_;
        std::unique_ptr<AddressPool<>> pool_;
        std::unique_ptr<PacketQueue> queue_;
        bool mirror_;
        // Methods
        void send_to_address_(
            std::shared_ptr<const Packet> packet, const MAVAddress &dest);
        void send_to_all_(std::shared_ptr<const Packet> packet);
};


#endif // CONNECTION_HPP_
