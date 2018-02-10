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


#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "If.hpp"
#include "mavlink.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"


/** Construct an If statement.
 *
 *  The default is to allow any type of packet from any address to any address.
 *
 *  The \ref type, \ref to, and \ref from methods can be used to apply
 *  conditions after construction.  Some examples are:
 *      - `If().type("PING").from("1.0/8").to("255.0");`
 *      - `If().type("HEARTBEAT").from("255.0/8");`
 *      - `If().type("SET_MODE").to("255.0/8");`
 *      - `If().from("255.0/8");`
 *
 *  \param id The packet ID to match.  %If {} or std::nullopt then any packet ID
 *      will match.  The defaut is {}.
 *  \param source The subnet a source address must be in to match.  %If {} or
 *      nullopt then any source address will match.  The default is {}.
 *  \param dest The subnet a destination address must be in to match.  %If {} or
 *      std::nullopt then any destination address will match.  The default is
 *      {}.
 *  \throws std::invalid_argument if the given \p id is not valid.
 */
If::If(
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
If &If::type(unsigned long id)
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
If &If::type(const std::string &name)
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
If &If::from(MAVSubnet subnet)
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
If &If::from(const std::string &subnet)
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
If &If::to(MAVSubnet subnet)
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
If &If::to(const std::string &subnet)
{
    dest_ = MAVSubnet(subnet);
    return *this;
}


/** Check whether a \ref Packet and \ref MAVAddress combination matches.
 *
 *  \param packet The packet to check for a match.
 *  \param address The address the packet is to be sent to.
 *  \retval true %If the packet matches the type, source subnet (by \ref
 *      MAVSubnet::contains), and destination subnet (by \ref
 *      MAVSubnet::contains) of the if statement.
 *  \retval false %If any of the packet type, source subnet, or destination
 *      subnet does not match.
 */
bool If::check(const Packet &packet, const MAVAddress &address) const
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


/** Equality comparison.
 *
 *  \relates If
 *  \param lhs The left hand side if statement.
 *  \param rhs The right hand side if statement.
 *  \retval true if \p lhs and \p rhs are the same.
 *  \retval false if \p lhs and \p rhs are not the same.
 *  \complexity \f$O(1)\f$
 */
bool operator==(const If &lhs, const If &rhs)
{
    return (lhs.id_ == rhs.id_) && (lhs.source_ == rhs.source_) &&
           (lhs.dest_ == rhs.dest_);
}


/** Inequality comparison.
 *
 *  \relates If
 *  \param lhs The left hand side if statement.
 *  \param rhs The right hand side if statement.
 *  \retval true if \p lhs and \p rhs are not the same.
 *  \retval false if \p lhs and \p rhs are the same.
 *  \complexity \f$O(1)\f$
 */
bool operator!=(const If &lhs, const If &rhs)
{
    return (lhs.id_ != rhs.id_) || (lhs.source_ != rhs.source_) ||
           (lhs.dest_ != rhs.dest_);
}


/** Print the if statement to the given output stream.
 *
 *  Some examples are:
 *  - `if PING from 1.0/8 to 255.0`
 *  - `if HEARTBEAT from 255.0/8`
 *  - `if SET_MODE to 255.0`
 *  - `if from 255.0/8`
 */
std::ostream &operator<<(std::ostream &os, const If &if_)
{
    // Handle the match any if_.
    os << "if";

    if (!if_.id_ && !if_.source_ && !if_.dest_)
    {
        os << " any";
        return os;
    }

    // Print packet name.
    if (if_.id_)
    {
        os << " " << mavlink::name(if_.id_.value());
    }

    // Print source subnet.
    if (if_.source_)
    {
        os << " from " << if_.source_.value();
    }

    // Print destination subnet.
    if (if_.dest_)
    {
        os << " to " << if_.dest_.value();
    }

    return os;
}
