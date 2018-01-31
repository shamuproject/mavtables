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
#include <utility>
#include <optional>
#include <stdexcept>

#include "mavlink.hpp"
#include "Packet.hpp"
#include "MAVSubnet.hpp"
#include "Conditional.hpp"


/** Construct a default condition.
 *
 *  The default conditional is initialized to match any packet type with and
 *  source and destination addresses.  Use \ref type, \ref to, and \ref from to
 *  restrict the matching.  Some examples are:
 *      - `auto cond = Conditional().type("PING").from("1.0/8").to("255.0");`
 *      - `auto cond = Conditional().type("HEARTBEAT").from("255.0/8");`
 *      - `auto cond = Conditional().type("SET_MODE").to("255.0/8");`
 *      - `auto cond = Conditional().from("255.0/8");`
 */
Conditional::Conditional()
{
    // The default constructors for all data members give the desired empty
    // option.
}



/** Construct a conditional.
 *
 *  The default is to allow any type of packet from any address to any address.
 *  This is so the \ref type, \ref from, and \ref to methods can be used to
 *  construct a packet with the form shown above.
 *
 *  \param id The packet ID to match.  If {} or std::nullopt then any packet ID
 *      will match.
 *  \param source The subnet a source address must be in to match.  If {} or
 *      nullopt then any source address will match.  The default is {}.
 *  \param dest The subnet a destination address must be in to match.  If {} or
 *      std::nullopt then any destination address will match.  The default is
 *      {}.
 *  \throws std::invalid_argument if the given \p id is not valid.
 */
Conditional::Conditional(
    std::optional<unsigned long> id,
    std::optional<MAVSubnet> source,
    std::optional<MAVSubnet> dest)
    : source_(std::move(source)), dest_(std::move(dest))
{
    if (id)
    {
        type(id.value());
    }
}


/** Set the packet type to match, by ID.
 *
 *  \param id The packet ID to match.
 *  \returns A reference to itself.
 *  \throws std::invalid_argument if the given \p id is not valid.
 *  \sa type(const std::string &name)
 */
Conditional &Conditional::type(unsigned long id)
{
    // Check packet ID, throws error if invalid.
    mavlink::name(id);
    id_ = id;
    return *this;
}


/** Set the packet type to match, by name.
 *
 *  \param name The packet name to match.
 *  \returns A reference to itself.
 *  \throws std::invalid_argument if the given message \p name is not valid.
 *  \sa type(unsigned long id)
 */
Conditional &Conditional::type(const std::string &name)
{
    id_ = mavlink::id(name);
    return *this;
}


/** Set subnet for source address matching using \ref MAVSubnet.
 *
 *  \param subnet The subnet used for source address matching.
 *  \returns A reference to itself.
 *  \sa from(const std::string &subnet)
 */
Conditional &Conditional::from(MAVSubnet subnet)
{
    source_ = std::move(subnet);
    return *this;
}

/** Set subnet for source address matching by string.
 *
 *  See \ref MAVSubnet::MAVSubnet(std::string address) for the acceptable
 *  formats and passible errors.
 *
 *  \param subnet The subnet used for source address matching.
 *  \returns A reference to itself.
 *  \sa from(MAVSubnet subnet)
 */
Conditional &Conditional::from(const std::string &subnet)
{
    source_ = MAVSubnet(subnet);
    return *this;
}


/** Set subnet for destination address matching using \ref MAVSubnet.
 *
 *  \param subnet The subnet used for destination address matching.
 *  \returns A reference to itself.
 *  \sa to(const std::string &subnet)
 */
Conditional &Conditional::to(MAVSubnet subnet)
{
    dest_ = std::move(subnet);
    return *this;
}


/** Set subnet for destination address matching by string.
 *
 *  See \ref MAVSubnet::MAVSubnet(std::string address) for the acceptable
 *  formats and possible errors.
 *
 *  \param subnet The subnet used for destination address matching.
 *  \returns A reference to itself.
 *  \sa to(MAVSubnet subnet)
 */
Conditional &Conditional::to(const std::string &subnet)
{
    dest_ = MAVSubnet(subnet);
    return *this;
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
bool Conditional::check(const Packet &packet, const MAVAddress &address) const
{
    bool result = true;

    // Check packet ID.
    if (id_)
    {
        result &= packet.id() == id_;
    }

    // Check source address.
    if (source_)
    {
        result &= source_->contains(packet.source());
    }

    // Check destination address.
    if (dest_)
    {
        result &= dest_->contains(address);
    }

    return result;
}


/** Print the conditional to the given output stream.
 *
 *  Some examples are:
 *  - `if PING from 1.0/8 to 255.0`
 *  - `if HEARTBEAT from 255.0/8`
 *  - `if SET_MODE to 255.0`
 *  - `if from 255.0/8`
 */
std::ostream &operator<<(std::ostream &os, const Conditional &conditional)
{
    // Handle the match any conditional.
    os << "if";

    if (!conditional.id_ && !conditional.source_ && !conditional.dest_)
    {
        os << " any";
        return os;
    }

    // Print packet name.
    if (conditional.id_)
    {
        os << " " << mavlink::name(conditional.id_.value());
    }

    // Print source subnet.
    if (conditional.source_)
    {
        os << " from " << conditional.source_.value();
    }

    // Print destination subnet.
    if (conditional.dest_)
    {
        os << " to " << conditional.dest_.value();
    }

    return os;
}
