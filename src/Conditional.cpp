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
#include <memory>
#include <utility>

#include "MAVSubnet.hpp"
#include "Conditional.hpp"


/** Construct a conditional.
 *
 *  The default is to allow any type of packet from any address to any address.
 *  This is so the \ref type, \ref from, and \ref to methods can be used to
 *  construct a packet with the form shown above.
 *
 *  \param packet_type The type of packet to match, use "*" to match any packet.
 *      The default value is "*".
 *  \param source The subnet a souce address must be in to match.
 *  \param dest The subnet a destination address must be in to match.
 */
Conditional::Conditional(
    std::string packet_type,
    std::unique_ptr<MAVSubnet> source,
    std::unique_ptr<MAVSubnet> dest)
    : packet_type_(std::move(packet_type)),
      source_(std::move(source)), dest_(std::move(dest))
{
}


/** Set the packet type to match by ID.
 *
 *  \param id The packet ID to match.
 *  \returns A reference to itself.
 *  \sa type(const std::string &name)
 */
Conditional &Conditional::type(uint32_t id)
{
}


/** Set the packet type to match by name.
 *
 *  \param name The packet name to match.
 *  \returns A reference to itself.
 *  \sa type(std::string id)
 */
Conditional &Conditional::type(const std::string &name)
{
}


/** Set subnet for source address matching using \ref MAVSubnet.
 *
 *  \param subnet The subnet used for source address matching.
 *  \returns A reference to itself.
 *  \sa from(const std::string &subnet)
 */
Conditional &Conditional::from(std::unique_ptr<MAVSubnet> subnet)
{
}


/** Set subnet for source address matching by string.
 *
 *  See \ref MAVSubnet::MAVSubnet(std::string address) for the acceptable
 *  formats.
 *
 *  \param subnet The subnet used for source address matching.
 *  \returns A reference to itself.
 *  \sa from(std::unique_ptr<MAVSubnet> subnet)
 */
Conditional &Conditional::from(const std::string &subnet)
{
}


/** Set subnet for destination address matching using \ref MAVSubnet.
 *
 *  \param subnet The subnet used for destination address matching.
 *  \returns A reference to itself.
 *  \sa to(const std::string &subnet)
 */
Conditional &Conditional::to(std::unique_ptr<MAVSubnet> subnet)
{
}


/** Set subnet for destination address matching by string.
 *
 *  See \ref MAVSubnet::MAVSubnet(std::string address) for the acceptable
 *  formats.
 *
 *  \param subnet The subnet used for destination address matching.
 *  \returns A reference to itself.
 *  \sa to(std::unique_ptr<MAVSubnet> subnet)
 */
Conditional &Conditional::to(const std::string &subnet)
{
}


/** Check whether a \ref Packet and \ref MAVAddress combination matches.
 *
 *  \param packet The packet to check for a match.
 *  \param address The address the packet is to be sent to.
 *  \retval true If the packet matches the type, source subnet (by \ref
 *      MAVSubnet::contains), and destination subnet (by \ref
 *      MAVSubnet::contains) of the conditional.
 *  \retval false If any of the packet type, source subnet, or destination
 *      subnet does not match.
 */
bool Conditional::check(const Packet &packet, const MAVAddress &address)
{
}


std::ostream &operator<<(std::ostream &os, const Conditional &conditional)
{
}
