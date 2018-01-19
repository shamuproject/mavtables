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
 *  \complexity \f$O(1)\f$
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
 *  \complexity \f$O(1)\f$
 */
int Packet::priority() const
{
    return priority_;
}


/** Set the priority of the packet.
 *
 *  A higher priority packet will be routed before a lower priority packet.  Any
 *  valid int is a valid priority.  Therefore, the guaranteed range is -32,768
 *  to 32,767 but on some systems priorities outside this range may still be
 *  valid.
 *
 *  \return The new priority of the packet.
 *  \complexity \f$O(1)\f$
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
 *  The format is "<Message Name> (#<Message ID>) from <Source Address> to
 *  <Destination Address> with priority <Priority> (v<Packet Version>)".
 *  However, both "to <Destination Address>" and "with priority <Priority>" are
 *  optional.  The former is not printed if it is a broadcast packet while the
 *  latter is not printed if the priority is the default 0.
 *
 *  Some examples are:
 *      - `"HEARTBEAT (#1) from 16.8 with priority 4 (v1.0)"`
 *      - `"PING (#4) from 128.4 to 16.8 (v2.0)"`
 *      - `"DATA_TRANSMISSION_HANDSHAKE (#130) from 16.8 (v2.0)"`
 *      - `"ENCAPSULATED_DATA (#131) from 128.4 with priority -3 (v2.0)"`
 *
 *  \relates Packet
 *  \param os The output stream to print to.
 *  \param packet The MAVLink packet to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const Packet &packet)
{
    // ID, name, and source.
    os << packet.name() << " (#" << packet.id() << ")";
    os << " from " << packet.source();

    // Destination.
    if (auto dest = packet.dest())
    {
        os << " to " << dest.value();
    }

    // Priority.
    if (packet.priority() != 0)
    {
        os << " with priority " << packet.priority();
    }

    // Version.
    os << " (v" << ((packet.version() & 0xFF00) >> 8) << "." <<
       (packet.version() & 0x00FF) << ")";
    return os;
}
