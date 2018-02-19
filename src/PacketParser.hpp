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


#ifndef PACKETPARSER_HPP_
#define PACKETPARSER_HPP_

#include <memory>

#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"


/** A MAVLink packet parser.
 *
 *  Parses wire protocol bytes into a MAVLink \ref Packet.
 */
class PacketParser
{
    private:
        // Types
        /** Packet parser states.
         */
        enum State
        {
            WAITING_FOR_START_BYTE, //!< Waiting for a magic start byte.
            WAITING_FOR_HEADER,     //!< Waiting for complete header.
            WAITING_FOR_PACKET      //!< Waitinf for complete packet.
        };
        // Variables
        std::vector<uint8_t> buffer_;
        PacketParser::State state_;
        Packet::Version version_;
        size_t bytes_remaining_;
        // Methods
        void waiting_for_start_byte_(uint8_t byte);
        void waiting_for_header_(uint8_t byte);
        std::unique_ptr<Packet> waiting_for_packet_(uint8_t byte);

    public:
        PacketParser();
        PacketParser(const PacketParser &other) = delete;
        PacketParser(PacketParser &&other) = delete;
        size_t bytes_parsed() const;
        void clear();
        std::unique_ptr<Packet> parse_byte(uint8_t byte);
        PacketParser &operator=(const PacketParser &other) = delete;
        PacketParser &operator=(PacketParser &&other) = delete;
};


#endif // PACKETPARSER_HPP_
