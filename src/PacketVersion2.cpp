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

#include "mavlink.h"
#include "Connection.hpp"
#include "PacketVersion2.hpp"


namespace packet_v2
{

    /** Construct a packet.
     *
     *  \param data Raw packet data.
     *  \param connection Connection packet was received on.
     *  \param priority Set the priority (default is 0).
     *
     *  \throws std::invalid_argument If packet data does not start with magic
     *      byte (0xFD).
     *  \throws std::length_error If packet data is not of correct length.
     */
    Packet::Packet(
        std::vector<uint8_t> data,
        std::weak_ptr<Connection> connection, int priority)
        : ::Packet(std::move(data), std::move(connection), priority)
    {
        const std::vector<uint8_t> &packet_data = this->data();

        // Check that data was given.
        if (packet_data.empty())
        {
            throw std::length_error("Packet is empty.");
        }

        // Check that a complete header was given (including magic number).
        if (!header_complete(packet_data))
        {
            // Could be the magic number.
            if (!is_magic(packet_data.front()))
            {
                std::stringstream ss;
                ss << "Invalid packet starting byte (0x"
                   << std::uppercase << std::hex
                   << static_cast<unsigned int>(packet_data.front())
                   << std::nouppercase << "), v2.0 packets should start with 0x"
                   << std::uppercase << std::hex << MAVLINK_STX
                   << std::nouppercase << ".";
                throw std::invalid_argument(ss.str());
            }
            // Otherwise the packet is not long enough.
            else
            {
                throw std::length_error(
                    "Packet (" + std::to_string(packet_data.size()) +
                    " bytes) is shorter than a v2.0 header (" +
                    std::to_string(MAVLINK_NUM_HEADER_BYTES) +
                    " bytes).");
            }
        }

        // Verify the message ID.
        if (mavlink_get_message_info_by_id(header(packet_data)->msgid) ==
                nullptr)
        {
            throw std::runtime_error(
                "Invalid packet ID (#" +
                std::to_string(header(packet_data)->msgid) + ").");
        }

        // Ensure a complete packet was given.
        if (!packet_complete(packet_data))
        {
            std::string prefix = "Packet";
            size_t expected_length =
                MAVLINK_NUM_NON_PAYLOAD_BYTES + header(packet_data)->len;
            if (is_signed(packet_data))
            {
                expected_length += MAVLINK_SIGNATURE_BLOCK_LEN;
                prefix = "Signed packet";
            }
            throw std::length_error(
                prefix + " is " + std::to_string(packet_data.size()) +
                " bytes, should be " +
                std::to_string(expected_length) + " bytes.");
        }
    }


    /** \copydoc ::Packet::version()
     *
     *  \returns 0x0200 (v2.0)
     *  \complexity \f$O(1)\f$
     */
    unsigned int Packet::version() const
    {
        return 0x0200;
    }


    /** \copydoc ::Packet::id()
     *
     *  \complexity \f$O(1)\f$
    */
    unsigned long Packet::id() const
    {
        return header(data())->msgid;
    }


    /** \copydoc ::Packet::name()
     *
     *  \throws std::runtime_error If the packet data has an invalid ID.
     *  \complexity \f$O(log(n))\f$ where \f$n\f$ is the total number of MAVLink
     *      messages.
     */
    std::string Packet::name() const
    {
        if (const mavlink_message_info_t *msg_info =
                    mavlink_get_message_info_by_id(header(data())->msgid))
        {
            return std::string(msg_info->name);
        }

        // There should never be any way to reach this point since the message
        // ID was checked in the constructor.  It is here just in case the
        // MAVLink C library has an error in it.
        throw std::runtime_error(
            "Invalid packet ID (#" +
            std::to_string(header(data())->msgid) + ").");
    }


    /** \copydoc ::Packet::source()
     *
     *  \complexity \f$O(1)\f$
     */
    MAVAddress Packet::source() const
    {
        return MAVAddress(header(data())->sysid, header(data())->compid);
    }


    /** \copydoc ::Packet::dest()
     *
     *  \throws std::runtime_error If the packet data has an invalid ID.
     *  \complexity \f$O(1)\f$
     *  \thanks The [mavlink-router](https://github.com/intel/mavlink-router)
     *      project for an example of how to extract the destination address.
     *
     */
    std::optional<MAVAddress> Packet::dest() const
    {
        if (const mavlink_msg_entry_t *msg_entry =
                    mavlink_get_msg_entry(header(data())->msgid))
        {
            int dest_system = -1;
            int dest_component = 0;

            // Extract destination system.
            if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM)
            {
                // Must check to make sure the target system offset is within
                // the packet payload because it can be striped out in v2.0
                // packets if it is 0.
                if (msg_entry->target_system_ofs < header(data())->len)
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
                // Must check to make sure the target component offset is within
                // the packet payload because it can be striped out in v2.0
                // packets if it is 0.
                if (msg_entry->target_component_ofs < header(data())->len)
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

        // There should never be any way to reach this point since the message
        // ID was checked in the constructor.  It is here just in case the
        // MAVLink C library has an error in it.
        throw std::runtime_error(
            "Invalid packet ID (#" +
            std::to_string(header(data())->msgid) + ").");
    }


    /** Determine if a byte is a MAVLink v2.0 packet starting byte.
     *
     *  \relates Packet
     *  \retval true if \p byte is the MAVLink v2.0 packet starting byte (0xFD).
     *  \retval false if \p byte is not the v2.0 starting byte.
     */
    bool is_magic(uint8_t byte)
    {
        return byte == MAVLINK_STX;
    }


    /** Determine if a MAVLink v2.0 packet is signed or not.
     *
     *  \relates
     *  \throws std::invalid_argument Header is not complete or is invalid.
     *  \throws std::length_error If packet data is not of correct length.
     */
    bool is_signed(const std::vector<uint8_t> &data)
    {
        // Check that a complete header was given (including magic number).
        if (!header_complete(data))
        {
            throw std::invalid_argument("Header is incomplete or invalid.");
        }

        return (header(data)->incompat_flags & MAVLINK_IFLAG_SIGNED);
    }


    /** Determine if the given data contains a complete v2.0 header.
     *
     *  \relates Packet
     *  \retval true if \p data contains a complete header (starting with the
     *      magic byte).
     *  \retval false if \p data contains does not contain a complete v2.0
     *      header.
     */
    bool header_complete(const std::vector<uint8_t> &data)
    {
        return (data.size() >= MAVLINK_NUM_HEADER_BYTES) &&
               (is_magic(data.front()));
    }


    /** Determine if the given data contains a complete v1.0 packet.
     *
     *  \relates Packet
     *  \retval true if \p data contains a complete packet (starting with the
     *      magic byte).
     *  \retval false if \p data contains does not contain a complete v1.0
     *      packet, or if there is extra bytes in \p data beyond the packet.
     */
    bool packet_complete(const std::vector<uint8_t> &data)
    {
        if (header_complete(data))
        {
            size_t expected_length = MAVLINK_NUM_NON_PAYLOAD_BYTES +
                header(data)->len;
            if (header(data)->incompat_flags & MAVLINK_IFLAG_SIGNED)
            {
                expected_length += MAVLINK_SIGNATURE_BLOCK_LEN;
            }
            return data.size() == expected_length;
        }

        return false;
    }


    /** Cast data as a v2.0 packet header structure pointer.
     *
     *  \return A pointer to the given data, cast to a v2.0 header structure.
     *      If an incomplete header is given a nullptr will be returned.
     */
    const struct mavlink_packet_version2_header *header(
        const std::vector<uint8_t> &data)
    {
        if (header_complete(data))
        {
            return reinterpret_cast<
                const struct mavlink_packet_version2_header *>(&(data[0]));
        }
        return nullptr;
    }

}
