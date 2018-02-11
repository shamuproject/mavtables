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

#include "mavlink.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "PacketParser.hpp"


/** Construct a \ref PacketParser.
 */
PacketParser::PacketParser()
    : state_(WAITING_FOR_START_BYTE)
{
    clear();
}


/** Return the number of bytes parsed on the current packet.
 *
 *  \returns The number of bytes parsed on the current packet, 0 if no packet is
 *      currently being parsed.
 */
size_t PacketParser::bytes_parsed() const
{
    return buffer_.size();
}


/** Reset packet parser so it can parse another packet.
 *
 *  %If called while parsing a packet, that packet will be lost.
 */
void PacketParser::clear()
{
    buffer_.clear();
    buffer_.reserve(MAVLINK_MAX_PACKET_LEN);
    state_ = WAITING_FOR_START_BYTE;
    version_ = Packet::V2;
    bytes_remaining_ = 0;
}


/** Parse MAVLink wire protocol bytes, v1.0 or v2.0.
 *
 *  When a packet is completed it will be returned and the parser reset so it
 *  can be used to continue parsing.
 *
 *  \param byte A byte from the MAVLink wire protocol.
 *  \returns A complete v1.0 or v2.0 packet.  %If the parser has not yet parsed
 *      a complete packet, nullptr is returned.
 */
std::unique_ptr<Packet> PacketParser::parse_byte(uint8_t byte)
{
    std::unique_ptr<Packet> packet;

    switch (state_)
    {
        case WAITING_FOR_START_BYTE:
            waiting_for_start_byte_(byte);
            break;

        case WAITING_FOR_HEADER:
            waiting_for_header_(byte);
            break;

        case WAITING_FOR_PACKET:
            packet = waiting_for_packet_(byte);
    }

    return packet;
}


/** Check for start of packet.
 *
 *  Start packet parsing if the given byte is a start byte for either v1.0 or
 *  v2.0 packets.  Next state will be WAITING_FOR_HEADER if this is the case.
 *
 *  \param byte The next byte to attempt parsing.
 */
void PacketParser::waiting_for_start_byte_(uint8_t byte)
{
    if (byte == packet_v1::START_BYTE)
    {
        // Store start byte and begin receiving header.
        buffer_.push_back(byte);
        state_ = WAITING_FOR_HEADER;
        version_ = packet_v1::VERSION;
    }
    else if (byte == packet_v2::START_BYTE)
    {
        // Store start byte and begin receiving header.
        buffer_.push_back(byte);
        state_ = WAITING_FOR_HEADER;
        version_ = packet_v2::VERSION;
    }
}


/** Parser header bytes.
 *
 *  Next state will be WAITING_FOR_PACKET if the given byte complete the header.
 *
 *  \param byte The next byte to attempt parsing.
 */
void PacketParser::waiting_for_header_(uint8_t byte)
{
    buffer_.push_back(byte);

    switch (version_)
    {
        case packet_v1::VERSION:
            if (packet_v1::header_complete(buffer_))
            {
                // Set number of expected bytes and start waiting for
                // remainder of packet.
                bytes_remaining_ = packet_v1::header(buffer_)->len +
                                   packet_v1::CHECKSUM_LENGTH;
                state_ = WAITING_FOR_PACKET;
            }

            break;

        case packet_v2::VERSION:
            if (packet_v2::header_complete(buffer_))
            {
                // Set number of expected bytes and start waiting for
                // remainder of packet.
                bytes_remaining_ = packet_v2::header(buffer_)->len +
                                   packet_v2::CHECKSUM_LENGTH;

                if (packet_v2::is_signed(buffer_))
                {
                    bytes_remaining_ += packet_v2::SIGNATURE_LENGTH;
                }

                state_ = WAITING_FOR_PACKET;
            }

            break;
    }
}


/** Parse packet bytes.
 *
 *  %If this byte completes the packet a \ref std::unique_ptr to the packet is
 *  returned and the parser is reset to begin parsing another packet (next state
 *  WAITING_FOR_START_BYTE).  Otherwise a nullptr is returned.
 *
 *  \param byte The next byte to attempt parsing.
 *  \returns A unique pointer to the complete packet.  %If the packet is not
 *      complete a nullptr is returned.
 */
std::unique_ptr<Packet> PacketParser::waiting_for_packet_(uint8_t byte)
{
    buffer_.push_back(byte);
    --bytes_remaining_;

    if (bytes_remaining_ == 0)
    {
        std::unique_ptr<Packet> packet;

        switch (version_)
        {
            case packet_v1::VERSION:
                packet = std::make_unique<packet_v1::Packet>(
                             std::move(buffer_));
                break;

            case packet_v2::VERSION:
                packet = std::make_unique<packet_v2::Packet>(
                             std::move(buffer_));
                break;
        }

        clear();
        return packet;
    }

    return nullptr;
}
