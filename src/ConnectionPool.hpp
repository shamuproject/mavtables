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


#ifndef CONNECTIONPOOL_HPP_
#define CONNECTIONPOOL_HPP_


#include <memory>
#include <set>
#include <shared_mutex>

#include "Connection.hpp"
#include "Packet.hpp"
#include "config.hpp"


/** A pool of connections to send packets out on.
 *
 *  A connection pool stores a reference to all connections that packets can be
 *  sent out over.
 */
class ConnectionPool
{
  public:
    // LCOV_EXCL_START
    TEST_VIRTUAL ~ConnectionPool() = default;
    // LCOV_EXCL_STOP
    TEST_VIRTUAL void add(std::weak_ptr<Connection> connection);
    TEST_VIRTUAL void remove(const std::weak_ptr<Connection> &connection);
    TEST_VIRTUAL void send(std::unique_ptr<const Packet> packet);

  private:
    std::set<
        std::weak_ptr<Connection>, std::owner_less<std::weak_ptr<Connection>>>
        connections_;
    std::shared_mutex mutex_;
};


#endif  // CONNECTIONPOOL_HPP_
