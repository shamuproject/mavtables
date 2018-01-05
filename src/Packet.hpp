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


#ifndef PACKET_HPP_
#define PACKET_HPP_


#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <ostream>
#include <optional>

#include "Connection.hpp"


/** A MAVLink packet with a reference to the connection it arrived on.
 *
 *  This is an abstract class, it is meant to be overridden by classes
 *  implementing either version 1 or version 2 of the MAVLink packet wire
 *  protocol.
 */
class Packet
{
    private:
        std::weak_ptr<Connection> connection_;
        int priority_;

    protected:
        std::vector<uint8_t> data_;

    public:
        /** Copy constructor.
         *
         * \param other Packet to copy.
         */
        Packet(const Packet &other) = default;
        Packet(std::weak_ptr<Connection> connection, int priority = 0);
        virtual ~Packet() = default;
        std::weak_ptr<Connection> connection() const;
        virtual unsigned int version() const = 0;
        virtual std::string packet_type() const = 0;
        virtual MAVAddress source_address() const = 0;
        virtual std::optional<MAVAddress> dest_address() const = 0;
        int priority() const;
        int priority(int priority);
        const std::vector<uint8_t> &data() const;
        /** Assignment operator.
         *
         * \param other Packet to copy.
         */
        Packet &operator=(const Packet &other) = default;

};


std::ostream &operator<<(std::ostream &os, const Packet &packet);


#endif // PACKET_HPP_