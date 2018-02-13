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


/** Add a connection to the pool.
 *
 *  \param connection The connection to add.
 *  \throws std::invalid_argument if the given connection pointer is nullptr.
 */
void ConnectionPool::add(std::shared_ptr<Connection<>> connection)
{
    if (connection == nullptr)
    {
        throw std::invalid_argument("Given Connection pointer is null.");
    }

    connections_.insert(std::move(connection));
}


/** Remove a connection from the pool.
 *
 *  \param connection The connection to remove.
 */
void ConnectionPool::remove(const std::shared_ptr<Connection<>> &connection)
{
    connections_.erase(connection);
}


/** Send a packet to every connection.
 *
 *  \note Each connection may decide to ignore the packet based on the filter
 *      rules.
 *
 *  \param packet Send a packet to every connection.
 */
void ConnectionPool::send(std::shared_ptr<const Packet> packet)
{
    for (auto i : connections_)
    {
        i->send(packet);
    }
}