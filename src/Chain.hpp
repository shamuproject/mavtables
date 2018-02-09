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


#ifndef CHAIN_HPP_
#define CHAIN_HPP_


#include <string>

#include "Action.hpp"


class Chain
{
    public:
        const std::string name;
        Chain(std::string name_);
        virtual ~Chain() = default;
        virtual Action::Option action(
            Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const;

};


#endif // CHAIN_HPP_
