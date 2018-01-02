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


#ifndef MAVADDRESS_HPP_
#define MAVADDRESS_HPP_


#include <string>


/** A MAVLink address.
 *
 *  MAVLink addresses consist of a system and component and can be represented
 *  as two ocetets in the form:
 *  ```
 *  system.component
 *  ```
 *  Therefore, a system of 16 and a component of 8 can be represented as `16.8`.
 *
 *  `0.0` is reserved as the broadcast address.
 */
class MAVAddress
{
    private:
        unsigned int address_;
        void construct_(unsigned int system, unsigned int component);

    public:
        /** Copy constructor.
         *
         * \param other MAVLink address to copy.
         */
        MAVAddress(const MAVAddress &other) = default;
        MAVAddress(unsigned int address);
        MAVAddress(unsigned int system, unsigned int component);
        MAVAddress(std::string address);
        unsigned int address() const;
        unsigned int system() const;
        unsigned int component() const;
        MAVAddress &operator=(const MAVAddress &other) = default;
};


bool operator==(const MAVAddress &lhs, const MAVAddress &rhs);
bool operator!=(const MAVAddress &lhs, const MAVAddress &rhs);
bool operator<(const MAVAddress &lhs, const MAVAddress &rhs);
bool operator>(const MAVAddress &lhs, const MAVAddress &rhs);
bool operator<=(const MAVAddress &lhs, const MAVAddress &rhs);
bool operator>=(const MAVAddress &lhs, const MAVAddress &rhs);
std::ostream &operator<<(std::ostream &os, const MAVAddress &mavaddress);


#endif // MAVADDRESS_HPP_
