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


#include <utility>

#include "Packet.hpp"


Packet(std::weak_ptr<Connection> connection, int priority = 0)
{
}


std::weak_ptr<Connection> connection() const
{
    return connection_;
}


std::string packet_type() const
{
    
}


int priority() const
{
    return priority_;
}


int priority(int priority)
{
    return priority_ = priority;
}


const std::vector<uint8_t> &data() const
{
    return data_
}


std::ostream &operator<<(std::ostream &os, const Packet &packet)
{

}
