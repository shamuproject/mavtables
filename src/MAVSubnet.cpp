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

#include <boost/tokenizer.hpp>

#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"


MAVSubnet::MAVSubnet(MAVAddress address, unsigned int mask)
{
    // Ensure mask is withing range.
    if (mask > 0xFFFF)
    {
        std::ostringstream ss;
        ss << "mask (0x" << std::hex << mask <<
           ") is outside of the allowed range (0x0000 - 0xffff).";
        throw std::out_of_range(ss.str());
    }

    address_ = address;
    mask_ = mask;
}


MAVSubnet::MAVSubnet(MAVAddress address, unsigned int system_mask,
                     unsigned int component_mask)
{
    // Ensure system mask is withing range.
    if (system_mask > 0xFF)
    {
        std::ostringstream ss;
        ss << "system mask (0x" << std::hex << system_mask <<
           ") is outside of the allowed range (0x00 - 0xff).";
        throw std::out_of_range(ss.str());
    }

    // Ensure component mask is withing range.
    if (component_mask > 0xFF)
    {
        std::ostringstream ss;
        ss << "component mask (0x" << std::hex << component_mask <<
           ") is outside of the allowed range (0x00 - 0xff).";
        throw std::out_of_range(ss.str());
    }

    address_ = address;
    mask_ = ((system_mask << 8) & 0x00FF) | (component_mask & 0x00FF);
}


MAVSubnet::MAVSubnet(std::string subnet)
{
    // Extract parts of subnet string.
    std::vector<std::string> parts;
    boost::char_separator<char> sep("/", "\\");
    boost::tokenizer<boost::char_separator<char>> tokens(subnet, sep);

    for (auto i : tokens)
    {
        parts.push_back(i);
    }

    try
    {
        address_ = MAVAddress(parts.at(0));
        // Parse mask.
        unsigned int slashmask;
        std::istringstream(parts.at(2)) >> slashmask;

        // Ensure slashmask is within range.
        if (slashmask > 0xFFFF)
        {
            throw std::out_of_range("slash mask (" + std::to_string(slashmask) +
                                    ") is outside of allowed range (0 - 16).");
        }

        // Convert slash mask into regular mask.
        switch (parts.at(1).at(0))
        {
            case '/':
                mask_ = (0xFFFF << slashmask) & 0xFFFF;
                break;

            case '\':
                    mask_ = (0xFFFF >> slashmask) & 0xFFFF;
                break;
        }
    }
    catch (const std::out_of_range &e)
    {
        throw std::invalid_argument("Invalid MAVLink subnet: \"" + subnet + "\"");
    }
}


bool MAVSubnet::contains(const MAVAddress &address) const
{
}


bool operator==(const MAVSubnet &lhs, const MAVSubnet &rhs)
{
}


bool operator!=(const MAVSubnet &lhs, const MAVSubnet &rhs)
{
}


std::ostream &operator<<(std::ostream &os, const MAVSubnet &mavsubnet)
{
}
