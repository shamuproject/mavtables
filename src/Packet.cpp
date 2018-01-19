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


#include <string>
#include <memory>
#include <utility>
#include <cstdint>
#include <ostream>
#include <optional>

#include "Connection.hpp"
#include "Packet.hpp"


/** Construct a packet.
 *
 *  \param data Raw packet data.
 *  \param connection Connection packet was received on.
 *  \param priority Set the priority (default is 0).
 */
Packet::Packet(std::vector<uint8_t> data, std::weak_ptr<Connection> connection,
               int priority)
    : data_(std::move(data)), connection_(std::move(connection)),
      priority_(priority)
{
}


Packet::~Packet()
{
}


/** Return reference to receiving connection.
 *
 *  \return The connection the packet was received on.
 */
std::weak_ptr<Connection> Packet::connection() const
{
    return connection_;
}


/** Return the priority of the packet.
 *
 *  The default priority is 0.  A higher priority packet will be routed before a
 *  lower priority packet.
 *
 *  \return The priority of the packet.
 */
int Packet::priority() const
{
    return priority_;
}


/** Set the priority of the packet.
 *
 *  A higher priority packet will be routed before a lower priority packet.
 *
 *  \return The new priority of the packet.
 */
int Packet::priority(int priority)
{
    return priority_ = priority;
}


/** Return the packet data.
 *
 *  \return The packet data as a vector of bytes.
 */
const std::vector<uint8_t> &Packet::data() const
{
    return data_;
}


/** Print the packet to the given output stream.
 *
 *  The format is "<Message Name> (#<Message ID>) from <Source Address> to <Dest
 *  Address> (v<packet version>)" or "<Message Name> (#<Message ID>) from
 *  <Source Address> (v<packet version>)" if no distination address is
 *  specified.
 *
 *  Some examples are:
 *      - "HEARTBEAT (#1) from 16.8 (v1.0)"
 *      - "PING (#4) from 128.4 to 16.8 (v2.0)"
 *      - "DATA_TRANSMISSION_HANDSHAKE (#130) from 16.8 (v2.0)"
 *      - "ENCAPSULATED_DATA (#131) from 128.4 (v2.0)"
 *
 *  \relates Packet
 *  \param os The output stream to print to.
 *  \param packet The MAVLink packet to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const Packet &packet)
{
    os << packet.name() << "(#" << packet.id() << ")";
    os << " from " << packet.source();

    if (auto dest = packet.dest())
    {
        os << " to " << dest.value();
    }

    os << " (" << ((packet.version() & 0xFF00) >> 8) << "." <<
       (packet.version() & 0x00FF) << ")";
    return os;
}
