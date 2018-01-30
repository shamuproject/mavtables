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

#include "MAVSubnet.hpp"
#include "MAVAddress.hpp"


/** A conditional used to determine if a packet matches a rule.
 *
 *  This uses the type, source, and destination of a packet to determine if it
 *  matches a \ref Rule.
 *
 *  While a typical constructor is provided a shorthand method is provided which
 *  allows construction of a Conditional much as it is represented in the
 *  configuration file.  Some examples are:
 *
 *  - `Conditional cond.type("PING").from("1.0/8").to("255.0");`
 *  - `Conditional cond.type("HEARTBEAT").from("255.0/8");`
 *  - `Conditional cond.type("SET_MODE").to("255.0/8");`
 *  - `Conditional cond.from("255.0/8");`
 */
class Conditional
{
    private:
        std::string packet_type_;
        std::unique_ptr<MAVSubnet> source_;
        std::unique_ptr<MAVSubnet> dest_;

    public:
        // Conditional(
        //     std::string packet_type = "*",
        //     std::unique_ptr<MAVSubnet> source =
        //         std::make_unique<MAVSubnet>(MAVAddress(0), 0),
        //     std::unique_ptr<MAVSubnet> dest =
        //         std::make_unique<MAVSubnet>(MAVAddress(0), 0));
        Conditional(
            std::string packet_type,
            std::unique_ptr<MAVSubnet> source,
            std::unique_ptr<MAVSubnet> dest);
        Conditional &type(uint32_t id);
        Conditional &type(const std::string &name);
        Conditional &from(std::unique_ptr<MAVSubnet> subnet);
        Conditional &from(const std::string &subnet);
        Conditional &to(std::unique_ptr<MAVSubnet> subnet);
        Conditional &to(const std::string &subnet);
        bool check(const Packet &packet, const MAVAddress &address);

        friend std::ostream &operator<<(
            std::ostream &os, const Conditional &conditional);
};


std::ostream &operator<<(std::ostream &os, const Conditional &conditional);


#endif // CONDITIONAL_HPP_
