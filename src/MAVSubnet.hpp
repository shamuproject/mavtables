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


#ifndef MAVSUBNET_HPP_
#define MAVSUBNET_HPP_


#include <ostream>
#include <string>

#include "MAVAddress.hpp"


/** A MAVLink subnet.
 *
 *  Mavlink subnets work the same as IP subnets and allow the definition of a
 *  range of addresses.  This is used to allow a single firewall rule to match
 *  multiple addresses.
 *
 *  \sa MAVAddress
 */
class MAVSubnet
{
    public:
        /** Copy constructor.
         *
         * \param other MAVLink subnet to copy from.
         */
        MAVSubnet(const MAVSubnet &other) = default;
        /** Move constructor.
         *
         * \param other MAVLink subnet to move from.
         */
        MAVSubnet(MAVSubnet &&other) = default;
        MAVSubnet(const MAVAddress &address, unsigned int mask = 0xFFFF);
        MAVSubnet(const MAVAddress &address, unsigned int system_mask,
                  unsigned int component_mask);
        MAVSubnet(std::string address);
        bool contains(const MAVAddress &address) const;
        /** Assignment operator.
         *
         * \param other MAVLink subnet to copy from.
         */
        MAVSubnet &operator=(const MAVSubnet &other) = default;
        /** Assignment operator (by move semantics).
         *
         * \param other MAVLink subnet to move from.
         */
        MAVSubnet &operator=(MAVSubnet &&other) = default;

        friend bool operator==(const MAVSubnet &lhs, const MAVSubnet &rhs);
        friend bool operator!=(const MAVSubnet &lhs, const MAVSubnet &rhs);
        friend std::ostream &operator<<(std::ostream &os,
                                        const MAVSubnet &mavsubnet);

    private:
        MAVAddress address_;
        unsigned int mask_;
};


bool operator==(const MAVSubnet &lhs, const MAVSubnet &rhs);
bool operator!=(const MAVSubnet &lhs, const MAVSubnet &rhs);
std::ostream &operator<<(std::ostream &os, const MAVSubnet &mavsubnet);


#endif // MAVSUBNET_HPP_
