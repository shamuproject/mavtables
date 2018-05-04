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
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "Logger.hpp"
#include "Packet.hpp"
#include "utility.hpp"


/** Add a connection to the pool.
 *
 *  \param connection The connection to add to the pool.
 */
void ConnectionPool::add(std::weak_ptr<Connection> connection)
{
    std::lock_guard<std::shared_mutex> lock(mutex_);
    connections_.insert(std::move(connection));
}


/** Remove a connection from the pool.
 *
 *  \param connection The connection to remove from the pool.
 */
void ConnectionPool::remove(const std::weak_ptr<Connection> &connection)
{
    std::lock_guard<std::shared_mutex> lock(mutex_);
    connections_.erase(connection);
}


/** Send a packet to every connection.
 *
 *  \note Each connection may decide to ignore the packet based on it's filter
 *      rules.
 *
 *  \param packet The packet to send to every connection, must not be nullptr.
 *  \throws std::invalid_argument if the \p packet pointer is null.
 */
void ConnectionPool::send(std::unique_ptr<const Packet> packet)
{
    if (packet == nullptr)
    {
        throw std::invalid_argument("Given packet pointer is null.");
    }

    if (Logger::level() >= 2)
    {
        std::stringstream ss;
        ss << "received " << str(*packet) << " source ";
        auto connection = packet->connection();

        if (connection == nullptr)
        {
            ss << "unknown";
        }
        else
        {
            ss << *connection;
        }

        Logger::log(2, ss.str());
    }

    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::shared_ptr<const Packet> shared = std::move(packet);

    for (auto it = connections_.begin(); it != connections_.end();)
    {
        // Send packet on connection.
        if (auto connection = it->lock())
        {
            connection->send(shared);
            ++it;
        }
        // Remove connection.
        else
        {
            it = connections_.erase(it);
        }
    }
}
