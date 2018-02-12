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


#include <memory>
#include <set>
#include <stdexcept>
#include <utility>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "Packet.hpp"


void ConnectionPool::add(std::shared_ptr<Connection> connection)
{
    if (connection == nullptr)
    {
        throw std::invalid_argument("Given Connection pointer is null.");
    }
    connections_.insert(std::move(connection));
}


void ConnectionPool::remove(const std::shared_ptr<Connection> &connection)
{
    connections_.erase(connection);
}


void ConnectionPool::send(std::shared_ptr<const Packet> packet)
{
    for (auto i : connections_)
    {
        i->send(packet);
    }
}
