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


#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "MAVAddress.hpp"
#include "Packet.hpp"


/** Construct a packet.
 *
 *  \param data Raw packet data.
 */
Packet::Packet(std::vector<uint8_t> data)
    : data_(std::move(data))
{
}


// GCC generates a seemingly uncallable destructor for pure virtual classes.
// Therefore, it must be excluded from test coverage.
// LCOV_EXCL_START
Packet::~Packet()
{
}
// LCOV_EXCL_STOP


/** Return the packet data.
 *
 *  \returns The packet data as a vector of bytes.
 */
const std::vector<uint8_t> &Packet::data() const
{
    return data_;
}


/** Set the source connection of the packet.
 *
 *  \param connection The source connection.
 *  \sa connection()
 */
void Packet::connection(std::weak_ptr<Connection> connection)
{
    connection_ = connection;
}


/** Get the source connection of the packet.
 *
 *  \returns The source connection if set and it still exists, otherwise
 *      nullptr.
 *  \sa connection(std::weak_ptr<Connection>)
 */
const std::shared_ptr<Connection> Packet::connection() const
{
    return connection_.lock();
}


/** Equality comparison.
 *
 *  Compares the raw packet data.
 *
 *  \relates Packet
 *  \param lhs The left hand side packet.
 *  \param rhs The right hand side packet.
 *  \retval true if \p lhs and \p rhs have the same packet data.
 *  \retval false if \p lhs and \p rhs do not have the same packet data.
 */
bool operator==(const Packet &lhs, const Packet &rhs)
{
    return lhs.data() == rhs.data();
}


/** Inequality comparison.
 *
 *  Compares the raw packet data.
 *
 *  \relates Packet
 *  \param lhs The left hand side packet.
 *  \param rhs The right hand side packet.
 *  \retval true if \p lhs and \p rhs do not have the same packet data.
 *  \retval false if \p lhs and \p rhs have the same packet data.
 */
bool operator!=(const Packet &lhs, const Packet &rhs)
{
    return lhs.data() != rhs.data();
}


/** Print the packet to the given output stream.
 *
 *  The format is "<Message Name> (#<Message ID>) from <Source Address> to
 *  <Destination Address> (v<Packet Version>)".  However, "to <Destination
 *  Address>" is optional and will not be printed if the destination is the
 *  broadcast address 0.0.
 *
 *  Some examples are:
 *      - `HEARTBEAT (#1) from 16.8 (v1.0)`
 *      - `PING (#4) from 128.4 to 16.8 (v2.0)`
 *      - `DATA_TRANSMISSION_HANDSHAKE (#130) from 16.8 (v2.0)`
 *      - `ENCAPSULATED_DATA (#131) from 128.4 (v2.0)`
 *
 *  \relates Packet
 *  \param os The output stream to print to.
 *  \param packet The MAVLink packet to print.
 *  \returns The output stream.
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

    // Version.
    os << " (v" << ((packet.version() & 0xFF00) >> 8) << "." <<
       (packet.version() & 0x00FF) << ")";
    return os;
}
