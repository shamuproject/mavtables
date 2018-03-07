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
// along with this program.  %If not, see <http://www.gnu.org/licenses/>.


#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketVersion1.hpp"
#include "mavlink.hpp"


namespace packet_v1
{
    /** \copydoc ::Packet::Packet(std::vector<uint8_t> data)
     *
     *  \throws std::invalid_argument %If packet data does not start with magic
     *      byte (0xFE).
     *  \throws std::length_error %If packet data is not of correct length.
     */
    Packet::Packet(std::vector<uint8_t> data) : ::Packet(std::move(data))
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
            if (START_BYTE != packet_data.front())
            {
                std::stringstream ss;
                ss << "Invalid packet starting byte (0x" << std::uppercase
                   << std::hex << static_cast<unsigned int>(packet_data.front())
                   << std::nouppercase << "), v1.0 packets should start with 0x"
                   << std::uppercase << std::hex
                   << static_cast<unsigned int>(START_BYTE) << std::nouppercase
                   << ".";
                throw std::invalid_argument(ss.str());
            }
            // Otherwise the packet is not long enough.
            else
            {
                throw std::length_error(
                    "Packet (" + std::to_string(packet_data.size()) +
                    " bytes) is shorter than a v1.0 header (" +
                    std::to_string(HEADER_LENGTH) + " bytes).");
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
            size_t expected_length =
                HEADER_LENGTH + header(packet_data)->len + CHECKSUM_LENGTH;
            throw std::length_error(
                "Packet is " + std::to_string(this->data().size()) +
                " bytes, should be " + std::to_string(expected_length) +
                " bytes.");
        }
    }


    /** \copydoc ::Packet::version()
     *
     *  \returns 0x0100 (v1.0) - ::Packet::V1
     *  \complexity \f$O(1)\f$
     */
    ::Packet::Version Packet::version() const { return VERSION; }


    /** \copydoc ::Packet::id()
     *
     *  \complexity \f$O(1)\f$
     */
    unsigned long Packet::id() const { return header(data())->msgid; }


    /** \copydoc ::Packet::name()
     *
     *  \throws std::runtime_error %If the packet data has an invalid ID.
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
        // LCOV_EXCL_START
        throw std::runtime_error(
            "Invalid packet ID (#" + std::to_string(header(data())->msgid) +
            ").");
        // LCOV_EXCL_STOP
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
     *  \complexity \f$O(1)\f$
     *  \thanks The [mavlink-router](https://github.com/intel/mavlink-router)
     *      project for an example of how to extract the destination address.
     *
     *  \throws std::runtime_error %If the packet data has an invalid ID.
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
                // target_system_ofs is offset from start of payload
                size_t offset =
                    msg_entry->target_system_ofs + sizeof(mavlink::v1_header);
                dest_system = data()[offset];
            }

            // Extract destination component.
            if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT)
            {
                // target_compoent_ofs is offset from start of payload
                size_t offset = msg_entry->target_component_ofs +
                                sizeof(mavlink::v1_header);
                dest_component = data()[offset];
            }

            // Construct MAVLink address.
            if (dest_system >= 0 && dest_component >= 0)
            {
                return MAVAddress(
                    static_cast<unsigned int>(dest_system),
                    static_cast<unsigned int>(dest_component));
            }

            // No destination address.
            return {};
        }

        // There should never be any way to reach this point since the message
        // ID was checked in the constructor.  It is here just in case the
        // MAVLink C library has an error in it.
        // LCOV_EXCL_START
        throw std::runtime_error(
            "Invalid packet ID (#" + std::to_string(header(data())->msgid) +
            ").");
        // LCOV_EXCL_STOP
    }


    /** Determine if the given data contains a complete v1.0 header.
     *
     *  \relates Packet
     *  \retval true if \p data contains a complete header (starting with the
     *      magic byte).
     *  \retval false if \p data contains does not contain a complete v1.0
     *      header.
     */
    bool header_complete(const std::vector<uint8_t> &data)
    {
        // We cant use the \ref header function here because that would cause
        // infinite recursion.
        return (data.size() >= HEADER_LENGTH) && (START_BYTE == data.front());
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
            size_t expected_length =
                HEADER_LENGTH + header(data)->len + CHECKSUM_LENGTH;
            return data.size() == expected_length;
        }

        return false;
    }


    /** Cast data as a v1.0 packet header structure pointer.
     *
     *  \return A pointer to the given data, cast to a v1.0 header structure.
     *      %If an incomplete header is given a nullptr will be returned.
     */
    const struct mavlink::v1_header *header(const std::vector<uint8_t> &data)
    {
        if (header_complete(data))
        {
            return reinterpret_cast<const struct mavlink::v1_header *>(
                &(data[0]));
        }

        return nullptr;
    }
}
