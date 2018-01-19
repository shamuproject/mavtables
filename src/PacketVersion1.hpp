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


#ifndef PACKETVERSION1_HPP_
#define PACKETVERSION1_HPP_


#include <string>
#include <vector>
#include <memory>
#include <cstdint>

extern "C"
{
#include "mavlink.h"
}
#include "Connection.hpp"
#include "Packet.hpp"


/** A MAVLink packet with the version 2 wire protocol.
 */
class PacketVersion1 : public Packet
{
    private:
        const struct mavlink_packet_version1_header *header_() const;

    public:
        /** Copy constructor.
         *
         * \param other Packet to copy.
         */
        PacketVersion1(const PacketVersion1 &other) = default;
        PacketVersion1(std::vector<uint8_t> data, std::weak_ptr<Connection> connection,
                       int priority = 0);
        virtual unsigned int version() const;
        virtual unsigned long id() const;
        virtual std::string name() const;
        virtual MAVAddress source() const;
        virtual std::optional<MAVAddress> dest() const = 0;
        /** Assignment operator.
         *
         * \param other Packet to copy.
         */
        PacketVersion1 &operator=(const PacketVersion1 &other) = default;
};


#endif // PACKETVERSION1_HPP_
