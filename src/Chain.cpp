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


#include <string>
#include <stdexcept>

#include "Action.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"


Chain::Chain(std::string name_)
    : name(std::move(name_))
{
    if (name.find_first_of("\t\n ") != std::string::npos)
    {
        throw std::invalid_argument("Chain names cannot contain whitespace.");
    }
}


Action Chain::action(
    const Packet &packet, const MAVAddress &address) const
{
    (void)packet;
    (void)address;
    return Action::make_continue();
}
