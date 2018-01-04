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


#include "Packet.hpp"


Packet::Packet(std::vector<uint8_t>,
               std::weak_ptr<Connection> connection = std::weak_ptr<Connection>(),
               int priority = 0)
{

}


std::weak_ptr<Connection> connection() const
{
}


std::string packet_type() const
{
}


unsigned int source_address() const
{
}


unsigned int dest_address() const
{
}


int priority() const
{
}


int priority(int priority)
{
}


const std::vector<uint8_t> &data() const
{
}


std::ostream &operator<<(std::ostream &os, const Packet &packet)
{
}
