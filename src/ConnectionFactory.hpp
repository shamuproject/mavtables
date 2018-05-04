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


#ifndef CONNECTIONFACTORY_HPP_
#define CONNECTIONFACTORY_HPP_


#include <chrono>
#include <memory>
#include <string>

#include "config.hpp"
#include "Connection.hpp"
#include "semaphore.hpp"


/** A factory for making related connections that use a common semaphore.
 */
template <class C = Connection,
          class AP = AddressPool<>,
          class PQ = PacketQueue>
class ConnectionFactory
{
    public:
        ConnectionFactory(std::shared_ptr<Filter> filter, bool mirror = false);
        TEST_VIRTUAL ~ConnectionFactory() = default;
        TEST_VIRTUAL std::unique_ptr<C> get(std::string name = "unknown");
        TEST_VIRTUAL bool wait_for_packet(
            const std::chrono::nanoseconds &timeout);

    private:
        std::shared_ptr<Filter> filter_;
        bool mirror_;
        semaphore semaphore_;
};


/** Construct a connection factory.
 *
 *  \tparam C The Connection class (or derived class) to use.
 *  \tparam AP The AddressPool class (or derived class) to use.
 *  \tparam PQ The PacketQueue class (or derived class) to use, must accept a
 *      callback function in it's constructor.
 *  \param filter The packet filter to use for determining whether and with what
 *      priority to add a packet to the queue for transmission.  This will be
 *      given to each constructed \ref Connection.  Cannot be nullptr.
 *  \param mirror Set to true if all \ref Connection's made by this factory are
 *      to be mirror connections.  A mirror connection is one that will receive
 *      all packets, regardless of destination address.  The default is false.
 *  \throws std::invalid_argument if the given \p filter pointer is null.
 */
template <class C, class AP, class PQ>
ConnectionFactory<C, AP, PQ>::ConnectionFactory(
    std::shared_ptr<Filter> filter, bool mirror)
    : filter_(std::move(filter)), mirror_(mirror)
{
    if (filter_ == nullptr)
    {
        throw std::invalid_argument("Given filter pointer is null.");
    }
}


/** Construct and return a new connection.
 *
 *  This connection will share a common semaphore with all other connections
 *  made by this factory instance.
 *
 *  \param name The name of the new connection.
 *  \returns The new connection.
 */
template <class C, class AP, class PQ>
std::unique_ptr<C> ConnectionFactory<C, AP, PQ>::get(std::string name)
{
    return std::make_unique<C>(
               name, filter_, mirror_,
               std::make_unique<AP>(),
               std::make_unique<PQ>([this]()
    {
        semaphore_.notify();
    }));
}


/** Wait for a packet to be available on any connection made by this factory.
 *
 *  \param timeout How long to block waiting for a packet.  Set to 0s for non
 *      blocking.
 *  \retval true There is a packet on at lest one of the connections created by
 *      this factory instance.
 *  \retval false The wait timed out, there is no packet available on any of the
 *      connections created by this factory instance.
 */
template <class C, class AP, class PQ>
bool ConnectionFactory<C, AP, PQ>::wait_for_packet(
    const std::chrono::nanoseconds &timeout)
{
    return semaphore_.wait_for(timeout);
}


#endif // CONNECTIONFACTORY_HPP_
