// MAVLink router and firewall.
// Copyright (C) 2017-2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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
#include <exception>
#include <stdexcept>

#include <boost/tokenizer.hpp>

#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"


/** Construct MAVLink subnet from a MAVLink address and mask.
 *
 *  \param address MAVLink address of subnet.
 *  \param mask Two byte subnet mask, where the system mask is in the MSB and
 *      the component mask is in the LSB.
 *  \throws std::out_of_range if the mask is not between 0x0000 and 0xFFFF.
 *  \sa Check if a \ref MAVAddress is within the subnet with \ref contains.
 */
MAVSubnet::MAVSubnet(const MAVAddress &address, unsigned int mask)
    : address_(address)
{
    // Ensure mask is withing range.
    if (mask > 0xFFFF)
    {
        std::ostringstream ss;
        ss << "mask (0x"
           << std::uppercase << std::hex << mask << std::nouppercase
           << ") is outside of the allowed range (0x0000 - 0xFFFF).";
        throw std::out_of_range(ss.str());
    }

    mask_ = mask;
}


/** Construct MAVLink subnet from an address, system mask, and component mask.
 *
 *  \param address MAVLink address of subnet.
 *  \param system_mask One byte subnet system mask with the bits set that must
 *      match the subnet address for a MAVLink address to be contained within
 *      the subnet.
 *  \param component_mask One byte subnet component mask with the bits set that
 *      must match the subnet address for a MAVLink address to be contained
 *      within the subnet.
 *  \throws std::out_of_range if the system and component masks are not each
 *      between 0x00 and 0xFF.
 *  \sa Check if a \ref MAVAddress is within the subnet with \ref contains.
 */
MAVSubnet::MAVSubnet(const MAVAddress &address, unsigned int system_mask,
                     unsigned int component_mask)
    : address_(address)
{
    // Ensure system mask is withing range.
    if (system_mask > 0xFF)
    {
        std::ostringstream ss;
        ss << "System mask (0x"
           << std::uppercase << std::hex << system_mask << std::nouppercase
           << ") is outside of the allowed range (0x00 - 0xFF).";
        throw std::out_of_range(ss.str());
    }

    // Ensure component mask is withing range.
    if (component_mask > 0xFF)
    {
        std::ostringstream ss;
        ss << "Component mask (0x" << std::hex << component_mask <<
           ") is outside of the allowed range (0x00 - 0xFF).";
        throw std::out_of_range(ss.str());
    }

    mask_ = ((system_mask << 8) & 0xFF00) | (component_mask & 0x00FF);
}


/** Construct MAVLink subnet from a string.
 *
 *  There are three string forms of of MAVLink subnets.
 *
 *    1. "<System ID>.<Component ID>:<System ID mask>.<Component ID mask>"
 *    2. "<System ID>.<Component ID>/<bits>"
 *    3. "<System ID>.<Component ID>\<bits>"
 *    3. "<System ID>.<Component ID>"
 *
 *  The first form is self explanatory, but the 2nd and 3rd are not as simple.
 *  In the 2nd case the number of bits (0 - 16) is the number of bits from the
 *  left that must match for an address to be in the subnet.  The 3rd form is
 *  like the 2nd, but does not require the system ID (first octet) to match and
 *  stats with the number of bits of the component ID (0 - 16) that must match
 *  from the left for an address to be in the subnet.  The last form is
 *  shorthand for "<System ID>.<Component ID>/16".
 *
 *  Below is a table relating the slash postfix to the subnet mask in \<System
 *  mask\>.\<Component mask\> notation.
 *
 *  | Mask with `/` | Mask with `\` | Postfix (#bits) |
 *  | -------------:| -------------:| ----------------:|
 *  |       255.255 |  out of range |               16 |
 *  |       255.254 |  out of range |               15 |
 *  |       255.252 |  out of range |               14 |
 *  |       255.248 |  out of range |               13 |
 *  |       255.240 |  out of range |               12 |
 *  |       255.224 |  out of range |               11 |
 *  |       255.192 |  out of range |               10 |
 *  |       255.128 |  out of range |                9 |
 *  |         255.0 |         0.255 |                8 |
 *  |         254.0 |         0.254 |                7 |
 *  |         252.0 |         0.252 |                6 |
 *  |         248.0 |         0.248 |                5 |
 *  |         240.0 |         0.240 |                4 |
 *  |         224.0 |         0.224 |                3 |
 *  |         192.0 |         0.192 |                2 |
 *  |         128.0 |         0.128 |                1 |
 *  |           0.0 |             0 |                0 |
 *
 *  Some examples are:
 *
 *    - "128.0/8" - Matches addresses with system ID 128 and any component ID.
 *    - "128.0/9" - Matches addresses with system ID 128 and components ID's of
 *      127 or less.
 *    - "128.255/9" - Matches addresses with system ID 128 and components ID's
 *      of 128 or more.
 *    - "128.0\1" - Matches addresses any system ID and components ID's of 127
 *      or less.
 *    - "128.255\1" - Matches addresses any system ID and components ID's of 128
 *      or more.
 *    - "255.0:128.240" - Matches system ID's 128 or greater and component ID's
 *      from 0 to 15.
 *    - "255.16:128.240" - Matches system ID's 128 or greater and component ID's
 *      from 16 to 31.
 *    - "255.16" - Matches only the address with system ID 255 and component ID
 *      16.
 *
 *  \param subnet String representing the MAVLink subnet.
 *  \throws std::out_of_range if either the System ID or the component ID is out
 *      of range.
 *  \throws std::out_of_range if the mask or slash bits are out of range.
 *  \sa Check if a \ref MAVAddress is within the subnet with \ref contains.
 */
MAVSubnet::MAVSubnet(std::string subnet)
    : address_(0)
{
    // If only an address was given (exact match subnet).
    try
    {
        address_ = MAVAddress(subnet);
        mask_ = 0xFFFF;
        return;
    }
    catch (std::exception)
    {
        // Continue on parsing normally.
    }

    // Extract parts of subnet string.
    std::vector<std::string> parts;
    boost::char_separator<char> sep("", ":/\\");
    boost::tokenizer<boost::char_separator<char>> tokens(subnet, sep);

    for (auto i : tokens)
    {
        parts.push_back(i);
    }

    // Ensure proper format.
    if (parts.size() != 3)
    {
        throw std::invalid_argument(
            "Invalid MAVLink subnet: \"" + subnet + "\".");
    }

    address_ = MAVAddress(parts[0]);
    // Determine format of subnet string.
    int slashmask;

    // If a regular subnet was given.
    switch (parts[1].at(0))
    {
        // Mask based subnet.
        case ':':
            try
            {
                mask_ = MAVAddress(parts[2]).address();
            }
            catch (std::invalid_argument)
            {
                throw std::invalid_argument(
                    "Invalid MAVLink subnet: \"" + subnet + "\".");
            }

            break;

        // Forward slash based subnet (bits from left).
        case '/':
            std::istringstream(parts.at(2)) >> slashmask;

            if (slashmask < 0 || slashmask > 16)
            {
                throw std::out_of_range(
                    "Forward slash mask (" + std::to_string(slashmask)
                    + ") is outside of allowed range (0 - 16).");
            }

            mask_ = (0xFFFF << (16 - slashmask)) & 0xFFFF;
            break;

        // Backward slash based subnet (bits from left of component).
        case '\\':
            std::istringstream(parts.at(2)) >> slashmask;

            if (slashmask < 0 || slashmask > 8)
            {
                throw std::out_of_range(
                    "Backslash mask (" + std::to_string(slashmask)
                    + ") is outside of allowed range (0 - 8).");
            }

            mask_ = (0xFFFF << (8 - slashmask)) & 0x00FF;
            break;
    }
}


/** Determine whether or not the subnet contains a given MAVLink address.
 *
 *  \param address The MAVLink address to test.
 *  \retval true if \p address is part of the subnet.
 *  \complexity \f$O(1)\f$
 */
bool MAVSubnet::contains(const MAVAddress &address) const
{
    return (address.address() & mask_) == (address_.address() & mask_);
}


/** Equality comparison.
 *
 *  \relates MAVSubnet
 *  \param lhs The left hand side MAVLink subnet.
 *  \param rhs The right hand side MAVLink subnet.
 *  \retval true if \p lhs and \p rhs are the same.
 *  \retval false if \p lhs and \p rhs are not the same.
 *  \complexity \f$O(1)\f$
 */
bool operator==(const MAVSubnet &lhs, const MAVSubnet &rhs)
{
    return (lhs.address_ == rhs.address_) && (lhs.mask_ == rhs.mask_);
}


/** Inequality comparison.
 *
 *  \relates MAVSubnet
 *  \param lhs The left hand side MAVLink subnet.
 *  \param rhs The right hand side MAVLink subnet.
 *  \retval true if \p lhs and \p rhs are not the same.
 *  \retval false if \p lhs and \p rhs are the same.
 *  \complexity \f$O(1)\f$
 */
bool operator!=(const MAVSubnet &lhs, const MAVSubnet &rhs)
{
    return (lhs.address_ != rhs.address_) || (lhs.mask_ != rhs.mask_);
}


/** Print the MAVLink subnet to the given output stream.
 *
 *  There are three string forms of MAVLink subnets.
 *
 *  1. "<System ID>.<Component ID>:<System ID mask>.<Component ID mask>"
 *  2. "<System ID>.<Component ID>/<bits>"
 *  3. "<System ID>.<Component ID>\<bits>"
 *  4. "<System ID>.<Component ID>"
 *
 *  The slash notation is preferred.  The last form is used when the mask
 *  requires all bits of a subnet to match an address.
 *
 *  See \ref MAVSubnet::MAVSubnet(std::string address) for more information on
 *  the string format.
 *
 *  \relates MAVSubnet
 *  \param os The output stream to print to.
 *  \param mavsubnet The MAVLink subnet to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const MAVSubnet &mavsubnet)
{
    os << mavsubnet.address_;

    switch (mavsubnet.mask_)
    {
        case 0b1111111111111111:
            // return os << "/16"
            // Represent exact masks as an address.
            return os;

        case 0b1111111111111110:
            return os << "/15";

        case 0b1111111111111100:
            return os << "/14";

        case 0b1111111111111000:
            return os << "/13";

        case 0b1111111111110000:
            return os << "/12";

        case 0b1111111111100000:
            return os << "/11";

        case 0b1111111111000000:
            return os << "/10";

        case 0b1111111110000000:
            return os << "/9";

        case 0b1111111100000000:
            return os << "/8";

        case 0b1111111000000000:
            return os << "/7";

        case 0b1111110000000000:
            return os << "/6";

        case 0b1111100000000000:
            return os << "/5";

        case 0b1111000000000000:
            return os << "/4";

        case 0b1110000000000000:
            return os << "/3";

        case 0b1100000000000000:
            return os << "/2";

        case 0b1000000000000000:
            return os << "/1";

        case 0b0000000000000000:
            return os << "/0";

        case 0b0000000011111111:
            return os << "\\8";

        case 0b0000000011111110:
            return os << "\\7";

        case 0b0000000011111100:
            return os << "\\6";

        case 0b0000000011111000:
            return os << "\\5";

        case 0b0000000011110000:
            return os << "\\4";

        case 0b0000000011100000:
            return os << "\\3";

        case 0b0000000011000000:
            return os << "\\2";

        case 0b0000000010000000:
            return os << "\\1";

        default:
            return os << ":" << MAVAddress(mavsubnet.mask_);
    }
}
