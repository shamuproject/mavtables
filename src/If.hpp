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


#ifndef IF_HPP_
#define IF_HPP_


#include <optional>
#include <string>

#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"


/** An if statement used to determine if a packet matches a rule.
 *
 *  This uses the type, source, and destination of a packet to determine if it
 *  matches a \ref Rule.
 *
 */
class If
{
    public:
        If(std::optional<unsigned long> id = {},
           std::optional<MAVSubnet> source = {},
           std::optional<MAVSubnet> dest = {});
        /** Copy constructor.
         *
         * \param other If to copy.
         */
        If(const If &other) = default;
        /** Move constructor.
         *
         * \param other If to move from.
         */
        If(If &&other) = default;
        If &type(unsigned long id);
        If &type(const std::string &name);
        If &from(MAVSubnet subnet);
        If &from(const std::string &subnet);
        If &to(MAVSubnet subnet);
        If &to(const std::string &subnet);
        bool check(const Packet &packet, const MAVAddress &address) const;
        /** Assignment operator.
         *
         * \param other If to copy.
         */
        If &operator=(const If &other) = default;
        /** Assignment operator (by move semantics).
         *
         * \param other If to move from.
         */
        If &operator=(If &&other) = default;

        friend bool operator==(const If &lhs, const If &rhs);
        friend bool operator!=(const If &lhs, const If &rhs);
        friend std::ostream &operator<<(
            std::ostream &os, const If &if_);

    private:
        std::optional<unsigned long> id_;
        std::optional<MAVSubnet> source_;
        std::optional<MAVSubnet> dest_;
};


bool operator==(const If &lhs, const If &rhs);
bool operator!=(const If &lhs, const If &rhs);
std::ostream &operator<<(std::ostream &os, const If &if_);


#endif // IF_HPP_
