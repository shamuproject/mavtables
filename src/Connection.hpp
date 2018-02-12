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

#include <chrono>
#include <map>
#include <memory>
#include <queue>

// #include "ConnectionPool.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "QueuedPacket.hpp"


// Template class only for testing.
template <class TC = std::chrono::steady_clock>
class Connection
{
    private:
        bool mirror_;
        std::shared_ptr<Filter> filter_;
        std::chrono::seconds address_timeout_;
        std::map<MAVAddress, std::chrono::time_point<TC>> addresses_;
        std::priority_queue<std::unique_ptr<QueuedPacket>> packet_queue_;

        void cleanup_addresses_()
        {
        }

    public:
        Connection(bool mirror = false, int address_timeout = 120);
        virtual ~Connection();
        // virtual for testing
        virtual void send(std::shared_ptr<const Packet> packet);
        std::shared_ptr<const Packet> next_packet();
        void add_address(MAVAddress address);
};


template <class TC>
inline Connection<TC>::~Connection()
{
}


template <class TC>
inline Connection<TC>::Connection(bool mirror, int address_timeout)
    : mirror_(mirror), address_timeout_(address_timeout)
{
}

template <class TC>
inline void Connection<TC>::send(std::shared_ptr<const Packet> packet)
{
    (void)packet;
}


template <class TC>
std::shared_ptr<const Packet> Connection<TC>::next_packet()
{
    return packet_queue_.top();
}


template <class TC>
void Connection<TC>::add_address(MAVAddress address)
{
    (void)address;
}


#endif // CONNECTION_HPP_
