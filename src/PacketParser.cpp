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


#include <memory>
#include <utility>

#include "mavlink.h"
#include "PacketParser.hpp"


/** Construct a \ref PacketParser.
 *
 *  It is constructed with a reference to the connection it is parsing as well
 *  as a default packet priority.
 *
 *  \param connection Connection the parser will receive data from.
 *  \param priority Set the default packet priority (default is 0).
 */
PacketParser::PacketParser(
    std::weak_ptr<Connection> connection, int priority)
    : connection_(std::move(connection)), priority_(priority)
{
    buffer_.reserve(MAVLINK_MAX_PACKET_LEN);
}


/** Parse MAVLink wire protocol bytes, v1.0 or v2.0.
 *
 *  When a packet is completed it will be returned and the parser reset so it
 *  can be used to continue parsing.
 *
 *  \param byte A byte from the MAVLink wire protocol.
 *  \returns A complete v1.0 or v2.0 packet.  If the parser has not yet parsed a
 *      complete packet, nullptr is returned.
 */
std::unique_ptr<Packet>  PacketParser::parse_byte(uint8_t byte)
{
    (void)byte;

    if (priority_)
    {
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}
