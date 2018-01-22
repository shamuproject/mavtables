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
#include "PacketVersion2.hpp"


/** \copydoc Packet::Packet(std::vector<uint8_t>,std::weak_ptr<Connection>,int)
 *
 *  \throws std::invalid_argument If packet data does not start with magic byte
 *      (0xFD).
 *  \throws std::length_error If packet data is not of correct length.
 */
PacketVersion2::PacketVersion2(
    std::vector<uint8_t> data,
    std::weak_ptr<Connection> connection, int priority)
    : Packet(std::move(data), std::move(connection), priority)
{
    // Check that a complete header was given (including magic number).
    if (this->data().size() < MAVLINK_NUM_HEADER_BYTES)
    {
        throw std::length_error(
            "Packet (" + std::to_string(this->data().size()) +
            " bytes) is shorter than a v2.0 header (" +
            std::to_string(MAVLINK_NUM_HEADER_BYTES) + " bytes).");
    }

    // Verify the magic number.
    if (header_()->magic != MAVLINK_STX)
    {
        std::stringstream ss;
        ss << "Invalid packet starting byte (0x"
           << std::uppercase << std::hex
           << static_cast<unsigned int>(header_()->magic)
           << std::nouppercase << "), v2.0 packets should start with 0x"
           << std::uppercase << std::hex << MAVLINK_STX << std::nouppercase
           << ".";
        throw std::invalid_argument(ss.str());
    }

    // Verify the message ID.
    if (mavlink_get_message_info_by_id(header_()->msgid) == nullptr)
    {
        throw std::runtime_error(
            "Invalid packet ID (#" + std::to_string(header_()->msgid) + ").");
    }

    // Ensure a complete packet was given.
    size_t expected_length = MAVLINK_NUM_NON_PAYLOAD_BYTES + header_()->len;

    if (header_()->incompat_flags & MAVLINK_IFLAG_SIGNED)
    {
        expected_length += MAVLINK_SIGNATURE_BLOCK_LEN;
    }

    if (this->data().size() != expected_length)
    {
        throw std::length_error(
            "Packet is " + std::to_string(this->data().size()) +
            " bytes, should be " + std::to_string(expected_length) + " bytes.");
    }
}


// Return pointer to the header structure.
const struct mavlink_packet_version2_header *PacketVersion2::header_() const
{
    return reinterpret_cast<const struct mavlink_packet_version2_header *>
           (&(data()[0]));
}


/** \copydoc Packet::version()
 *
 *  \returns 0x0200 (v2.0)
 *  \complexity \f$O(1)\f$
 */
unsigned int PacketVersion2::version() const
{
    return 0x0200;
}


/** \copydoc Packet::id()
 *
 *  \complexity \f$O(1)\f$
*/
unsigned long PacketVersion2::id() const
{
    return header_()->msgid;
}


/** \copydoc Packet::name()
 *
 *  \throws std::runtime_error If the packet data has an invalid ID.
 *  \complexity \f$O(log(n))\f$ where \f$n\f$ is the total number of MAVLink
 *      messages.
 */
std::string PacketVersion2::name() const
{
    if (const mavlink_message_info_t *msg_info = mavlink_get_message_info_by_id(
                header_()->msgid))
    {
        return std::string(msg_info->name);
    }

    // There should never be any way to reach this point since the message ID
    // was checked in the constructor.  It is here just in case the MAVLink C
    // library has an error in it.
    throw std::runtime_error(
        "Invalid packet ID (#" + std::to_string(header_()->msgid) + ").");
}


/** \copydoc Packet::source()
 *
 *  \complexity \f$O(1)\f$
 */
MAVAddress PacketVersion2::source() const
{
    return MAVAddress(header_()->sysid, header_()->compid);
}


/** \copydoc Packet::dest()
 *
 *  \throws std::runtime_error If the packet data has an invalid ID.
 *  \complexity \f$O(1)\f$
 *  \thanks The [mavlink-router](https://github.com/intel/mavlink-router)
 *      project for an example of how to extract the destination address.
 *
 */
std::optional<MAVAddress> PacketVersion2::dest() const
{
    if (const mavlink_msg_entry_t *msg_entry =
                mavlink_get_msg_entry(header_()->msgid))
    {
        int dest_system = -1;
        int dest_component = 0;

        // Extract destination system.
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM)
        {
            // Must check to make sure the target system offset is within the
            // packet payload because it can be striped out in v2.0 packets if
            // it is 0.
            if (msg_entry->target_system_ofs < header_()->len)
            {
                // target_system_ofs is offset from start of payload
                size_t offset = msg_entry->target_system_ofs +
                                sizeof(mavlink_packet_version2_header);
                dest_system = data()[offset];
            }
            else
            {
                dest_system = 0;
            }
        }

        // Extract destination component.
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT)
        {
            // Must check to make sure the target component offset is within the
            // packet payload because it can be striped out in v2.0 packets if
            // it is 0.
            if (msg_entry->target_component_ofs < header_()->len)
            {
                // target_compoent_ofs is offset from start of payload
                size_t offset = msg_entry->target_component_ofs +
                                sizeof(mavlink_packet_version2_header);
                dest_component = data()[offset];
            }
            else
            {
                dest_component = 0;
            }
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

    // There should never be any way to reach this point since the message ID
    // was checked in the constructor.  It is here just in case the MAVLink C
    // library has an error in it.
    throw std::runtime_error(
        "Invalid packet ID (#" + std::to_string(header_()->msgid) + ").");
}
