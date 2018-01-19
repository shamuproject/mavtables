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
#include <vector>
#include <memory>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C"
{
#include "mavlink.h"
}
#include "Connection.hpp"
#include "PacketVersion1.hpp"


/** \copydoc Packet::Packet(std::vector<uint8_t>,std::weak_ptr<Connection>,int)
 *
 *  \throws std::invalid_argument If packet data does not start with magic byte
 *      (0xFE).
 *  \throws std::length_error If packet data is not of correct length.
 */
PacketVersion1::PacketVersion1(std::vector<uint8_t> data,
                               std::weak_ptr<Connection> connection, int priority)
    : Packet(std::move(data), std::move(connection), priority)
{
    // Check that a complete header was given (including magic number).
    if (this->data().size() >= (MAVLINK_CORE_HEADER_MAVLINK1_LEN + 1))
    {
        // Verify the magic number.
        if (header_()->magic != MAVLINK_STX_MAVLINK1)
        {
            std::stringstream ss;
            ss << "Invlaid packet starting byte (0x" << std::hex << header_()->magic <<
               ").";
            throw std::invalid_argument(ss.str());
        }

        // Ensure a complete packet was given.
        size_t expected_packet_length = header_()->len +
                                        MAVLINK_CORE_HEADER_MAVLINK1_LEN + MAVLINK_NUM_CHECKSUM_BYTES;

        if (this->data().size() == expected_packet_length)
        {
            return;
        }
    }

    throw std::length_error("Packet data does not have correct length.");
}


// Return pointer to the header structure.
const struct mavlink_packet_version1_header *PacketVersion1::header_() const
{
    return reinterpret_cast<const struct mavlink_packet_version1_header *>(&
            (data()[0]));
}


/** \copydoc Packet::version()
 *
 *  \returns 0x0200 (v2.0)
 *  \complexity \f$O(1)\f$
 */
unsigned int PacketVersion1::version() const
{
    return 0x0100;
}


/** \copydoc Packet::id()
 *
 *  \complexity \f$O(1)\f$
*/
unsigned long PacketVersion1::id() const
{
    return header_()->msgid;
}


/** \copydoc Packet::name()
 *
 *  \complexity \f$O(log(n))\f$ where \f$n\f$ is the total number of MAVLink
 *      messages.
 *  \throws std::runtime_error If the packet data has an invalid ID.
 */
std::string PacketVersion1::name() const
{
    if (const mavlink_message_info_t *msg_info = mavlink_get_message_info_by_id(
                header_()->msgid))
    {
        return std::string(msg_info->name);
    }

    throw std::runtime_error("Invalid packet ID.");
}


/** \copydoc Packet::source()
 *
 *  \complexity \f$O(1)\f$
 */
MAVAddress PacketVersion1::source() const
{
    return MAVAddress(header_()->sysid, header_()->compid);
}


/** \copydoc Packet::dest()
 *
 *  \complexity \f$O(1)\f$
 *  \thanks The [mavlink-router](https://github.com/intel/mavlink-router)
 *      project for an example of how to extract the destination address.
 *
 *  \throws std::runtime_error If the packet data has an invalid ID.
 */
std::optional<MAVAddress> PacketVersion1::dest() const
{
    if (const mavlink_msg_entry_t *msg_entry = mavlink_get_msg_entry(
                header_()->msgid))
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
            return MAVAddress(static_cast<unsigned int>(dest_system),
                              static_cast<unsigned int>(dest_component));
        }

        // No destination address.
        return {};
    }

    throw std::runtime_error("Invalid packet ID.");
}
