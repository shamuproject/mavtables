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


#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "util.hpp"
#include "MAVAddress.hpp"


/** Construct MAVLink address from a string.
 *
 *  Parse a string of the form "<System ID>.<Component ID>".
 *
 *  Some examples are:
 *      - "0.0"
 *      - "16.8"
 *      - "128.4"
 *
 *  \param address String representing the MAVLink address.
 */
MAVAddress::MAVAddress(std::string address)
{
    std::replace(address.begin(), address.end(), '.', ' ');
    std::vector<unsigned long> octets;
    std::istringstream ss(address);
    unsigned long i;

    while (ss >> i)
    {
        octets.push_back(i);
    }

    if (octets.size() != 2)
    {
        throw std::invalid_argument("Invalid MAVLink address string.");
    }

    address_ = (octets[0] << 8) | octets[1];
}


/** Construct MAVLink address from an address in numeric representation.
 *
 *  The numeric representation of a MAVLink address is two bytes, the MSB
 *  contains the System ID and the LSB contains the Component ID.
 *
 *  \param address Address (0 - 65535) with System encoded in MSB and Component
 *      encoded in LSB.
 *  \throws std::out_of_range if the address is not between 0 and 65535.
 */
MAVAddress::MAVAddress(unsigned int address)
{
    if (address > 65535)
    {
        throw std::out_of_range("address (" + std::str(address) +
                                ") is outside of the allowed range (0 - 65535).");
    }

    address_ = address;
}


/** Construct MAVLink address from the System ID and Component ID.
 *
 *  \note component=0 and system=0 is the broadcast address.
 *
 *  \param system System ID (0 - 255).
 *  \param system Component ID (0 - 255).
 *  \throws std::out_of_range if either the System ID or the component ID is out
 *      of range.
 */
MAVAddress::MAVAddress(unsigned int system, unsigned int component)
{
    if (system > 255)
    {
        throw std::out_of_range("system id (" + std::str(system) +
                                ") is outside of the allowed range (0 - 255).");
    }

    if (component > 255)
    {
        throw std::out_of_range("component id (" + std::str(system) +
                                ") is outside of the allowed range (0 - 255).");
    }

    address_ = ((system << 8) & 0xFF00) | (component & 0x00FF);
}


/** Return the MAVLink address in numeric form.
 *
 *  \return The MAVLink address as a two byte number with the System ID encoded
 *      in the MSB and the Component ID in the LSB.
 */
unsigned int MAVAddress::address() const
{
    return address_;
}


/** Return the System ID.
 *
 *  \return The System ID (0 - 255).
 */
unsigned int MAVAddress::system() const
{
    return (address_ >> 8) & 0x00FF;
}


/** Return the Component ID.
 *
 *  \return The Component ID (0 - 255).
 */
unsigned int MAVAddress::component() const
{
    return address & 0x00FF;
}


/** Equality comparison.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side MAVLink address.
 *  \param rhs The right hand side MAVLink address.
 *  \retval true if \p lhs and \p rhs have the same system and component ID's.
 *  \retval true if \p lhs and \p rhs do not have the same system and component
 *      ID's.
 */
bool operator==(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() == rhs.address();
}


/** Equality comparison.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side MAVLink address.
 *  \param rhs The right hand side MAVLink address.
 *  \retval true if \p lhs and \p rhs do not have the same system and component
 *      ID's
 *  \retval true if \p lhs and \p rhs have the same system and component ID's.
 */
bool operator!=(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() != rhs.address();
}


/** Less than comparison.
 *
 *  \note The System ID is considered first followed by the Component ID.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side MAVLink address.
 *  \param rhs The right hand side MAVLink address.
 *  \retval true if \p lhs is less than \p rhs.
 *  \retval false if \p lhs is not less than \p rhs.
 */
bool operator<(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() < rhs.address();
}


/** Greater than comparison.
 *
 *  \note The System ID is considered first followed by the Component ID.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side MAVLink address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is greater than \p rhs.
 *  \retval false if \p lhs is not greater than \p rhs.
 */
bool operator>(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() > rhs.address();
}


/** Less than or equal comparison.
 *
 *  \note The System ID is considered first followed by the Component ID.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is less than or eqaul to \p rhs.
 *  \retval false if \p lhs is greater than \p rhs.
 */
bool operator<=(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() <= rhs.address();
}


/** Greater than comparison.
 *
 *  \note The System ID is considered first followed by the Component ID.
 *
 *  \relates MAVAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is greater than or equal to \p rhs.
 *  \retval false if \p lhs is less than \p rhs.
 */
bool operator>=(const MAVAddress &lhs, const MAVAddress &rhs)
{
    return lhs.address() >= rhs.address();
}


/** Print the MAVLink address to the given output stream.
 *
 *  The format is "<System ID>.<Component ID>".
 *
 *  Some examples are:
 *      - "0.0"
 *      - "16.8"
 *      - "128.4"
 *
 *  \relates MAVAddress
 *  \param os The output stream to print to.
 *  \param ipaddress The MAVLink address to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const MAVAddress &mavaddress)
{
    os << mavaddress.system() << "." << mavaddress.component();
    return os;
}
