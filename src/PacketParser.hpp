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


#ifndef PACKETPARSER_HPP_
#define PACKETPARSER_HPP_


#include <memory>
#include <cstdint>

#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"


/** Parse bytes into a MAVLink packet.
 */
class PacketParser
{
    private:
        std::unique_ptr<PacketVersion1>;
        std::unique_ptr<PacketVersion2>;

    public:
        bool complete();
        bool parse_byte(uint8_t byte);
        std::vector<uint8_t> parse_bytes(const std::vector<uint8_t> bytes);
        std::unique_ptr<Packet> packet();
}


#endif // PACKETPARSER_HPP_
