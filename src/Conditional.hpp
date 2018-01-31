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


#ifndef CONDITIONAL_HPP_
#define CONDITIONAL_HPP_


#include <string>
#include <memory>
#include <optional>

#include "Packet.hpp"
#include "MAVSubnet.hpp"
#include "MAVAddress.hpp"


/** A conditional used to determine if a packet matches a rule.
 *
 *  This uses the type, source, and destination of a packet to determine if it
 *  matches a \ref Rule.
 *
 */
class Conditional
{
    private:
        std::optional<unsigned long> id_;
        std::optional<MAVSubnet> source_;
        std::optional<MAVSubnet> dest_;

    public:
        Conditional();
        Conditional(
            std::optional<unsigned long> id,
            std::optional<MAVSubnet> source,
            std::optional<MAVSubnet> dest);
        Conditional &type(unsigned long id);
        Conditional &type(const std::string &name);
        Conditional &from(MAVSubnet subnet);
        Conditional &from(const std::string &subnet);
        Conditional &to(MAVSubnet subnet);
        Conditional &to(const std::string &subnet);
        bool check(const Packet &packet, const MAVAddress &address);

        friend std::ostream &operator<<(
            std::ostream &os, const Conditional &conditional);
};


std::ostream &operator<<(std::ostream &os, const Conditional &conditional);


#endif // CONDITIONAL_HPP_
