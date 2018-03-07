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


#ifndef PACKET_HPP_
#define PACKET_HPP_


#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "MAVAddress.hpp"


/** A MAVLink packet.
 *
 *  This is an abstract class, it is meant to be overridden by classes
 *  implementing either version 1 or version 2 of the MAVLink packet wire
 *  protocol.
 */
class Packet
{
  public:
    enum Version
    {
        V1 = 0x0100,
        V2 = 0x0200
    };

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
    Packet(std::vector<uint8_t> data);
    virtual ~Packet();  // Clang does not like pure virtual destructors.
    /** Return packet version.
     *
     *  \returns Two byte Packet version with major version in MSB and minor
     *      version in LSB.
     */
    virtual Version version() const = 0;
    /** Return MAVLink message ID.
     *
     *  \returns The numeric message ID of the MAVLink packet (0 to 255).
     */
    virtual unsigned long id() const = 0;
    /** Return MAVLink message name.
     *
     *  \returns The message name of the MAVLink packet.
     */
    virtual std::string name() const = 0;
    /** Return source address.
     *
     *  Where the packet came from.
     *
     *  \returns The source MAVLink address of the packet.
     */
    virtual MAVAddress source() const = 0;
    /** Return destination address.
     *
     *  Where the packet is sent to.  This is optional because not all
     *  packets have a destination.   %If a system is specified but not a
     *  component a component ID of 0 will be used (the broadcast ID).
     *
     *  \returns The destination MAVLink address of the packet if not a
     *      broadcast packet.
     */
    virtual std::optional<MAVAddress> dest() const = 0;
    const std::vector<uint8_t> &data() const;
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

  private:
    std::vector<uint8_t> data_;
};


bool operator==(const Packet &lhs, const Packet &rhs);
bool operator!=(const Packet &lhs, const Packet &rhs);
std::ostream &operator<<(std::ostream &os, const Packet &packet);


#endif  // PACKET_HPP_
