// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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
#include <vector>
#include <memory>
#include <cstdint>

extern "C"
{
#include "mavlink.h"
}
#include "Connection.hpp"
#include "PacketVersion1.hpp"


// const mavlink_message_info_t *mavlink_get_message_info_by_id(uint32_t msgid);
// #include "mavlink/v2.0/mavlink_get_info.h"

/** Construct a packet.
 *
 *  \param data Raw packet data.
 *  \param connection Connection packet was received on.
 *  \param priority Set the priority (default is 0).
 */
PacketVersion1::PacketVersion1(std::vector<uint8_t> data,
                               std::weak_ptr<Connection> connection, int priority)
    : Packet(std::move(data), std::move(connection), priority)
{
    if (this->data().size() >= sizeof(mavlink_packet_version1_header))
    {
        if (this->data().size() == sizeof(mavlink_packet_version1_header) +
                header_()->len + 2)
        {
            return;
        }
    }

    throw std::length_error("Packet data does not have correct length.");
}


// Return pointer to the header structure.
const struct mavlink_packet_version1_header *PacketVersion1::header_() const
{
    return reinterpret_cast<const struct mavlink_packet_version1_header *>(&(data()[0]));
}


/** \copydoc Packet::version()
 *
 *  \returns 0x0100 (v1.0)
 */
unsigned int PacketVersion1::version() const
{
    return 0x0100;
}


//! \copydoc Packet::id()
unsigned long PacketVersion1::id() const
{
    return header_()->msgid;
}


//! \copydoc Packet::name()
std::string PacketVersion1::name() const
{
    if (const mavlink_message_info_t *msg_info = mavlink_get_message_info_by_id(header_()->msgid))
    {
        return std::string(msg_info->name);
    }

    throw std::runtime_error("Invalid packet ID.");
}


//! \copydoc Packet::source()
MAVAddress PacketVersion1::source() const
{
    return MAVAddress(header_()->sysid, header_()->compid);
}


/** \copydoc Packet::dest()
 *
 *  Thanks to the
 *  [mavlink-router](https://github.com/intel/mavlink-router) project
 *  for an example of how to extract the destination address.
 */
std::optional<MAVAddress> PacketVersion1::dest() const
{
    if (const mavlink_msg_entry_t *msg_entry = mavlink_get_msg_entry(header_()->msgid))
    {
        int dest_system = -1;
        int dest_component = -1;

        // Extract destination system.
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM)
        {
            dest_system = data()[msg_entry->target_system_ofs];
        }

        // Extract destination component.
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT)
        {
            dest_component = data()[msg_entry->target_component_ofs];
        }

        // Construct MAVLink address.
        if (dest_system >= 0 && dest_component >= 0)
        {
            return MAVAddress(static_cast<unsigned int>(dest_system), static_cast<unsigned int>(dest_component));
        }

        // No destination address.
        return {};
    }

    throw std::runtime_error("Invalid packet ID.");
}
