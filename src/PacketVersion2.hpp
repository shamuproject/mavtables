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


#ifndef PACKETVERSION2_HPP_
#define PACKETVERSION2_HPP_


#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "mavlink.h"
#include "Connection.hpp"
#include "Packet.hpp"


namespace packet_v2
{

    /** A MAVLink packet with the version 2 wire protocol.
     */
    class Packet : public ::Packet
    {
        private:
            const struct mavlink_packet_version2_header *header_() const;

        public:
            /** Copy constructor.
             *
             *  \param other Packet to copy.
             */
            Packet(const Packet &other) = default;
            /** Move constructor.
             *
             *  \param other Packet to move from.
             */
            Packet(Packet &&other) = default;
            Packet(
                std::vector<uint8_t> data,
                std::weak_ptr<Connection> connection,
                int priority = 0);
            virtual unsigned int version() const;
            virtual unsigned long id() const;
            virtual std::string name() const;
            virtual MAVAddress source() const;
            virtual std::optional<MAVAddress> dest() const;
            /** Assignment operator.
             *
             *  \param other Packet to copy.
             */
            Packet &operator=(const Packet &other) = default;
            /** Assignment operator (by move semantics).
             *
             *  \param other Packet to move from.
             */
            Packet &operator=(Packet &&other) = default;
    };

    bool is_magic(uint8_t byte);
    bool is_signed(const std::vector<uint8_t> &data);
    bool header_complete(const std::vector<uint8_t> &data);
    bool packet_complete(const std::vector<uint8_t> &data);
    const struct mavlink_packet_version2_header *header(
        const std::vector<uint8_t> &data);

}


#endif // PACKETVERSION2_HPP_
