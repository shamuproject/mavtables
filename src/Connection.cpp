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


#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

#include "AddressPool.hpp"
#include "Connection.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketQueue.hpp"


/** Send a packet to a particular address.
 *
 *  Packets are ran through the contained \ref Filter before being placed into
 *  the \ref PacketQueue given in the constructor.  Packets are read from the
 *  queue (for sending) by using the \ref next_packet method.
 *
 *  \note This disregards the destination address of the packet.
 *
 *  \param packet The packet to send.
 *  \param dest The address to send the packet to, if this address is not
 *      handled by this connection the packet will be silently dropped.
 */
void Connection::send_to_address_(
    std::shared_ptr<const Packet> packet, const MAVAddress &dest)
{
    // Address reachable on this connection.
    if (pool_->contains(dest))
    {
        // Run packet/address combination through the filter.
        auto [accept, priority] = filter_->will_accept(*packet, dest);

        // Add packet to the queue.
        if (accept)
        {
            queue_->push(std::move(packet), priority);
        }
    }
}


/** Send a packet to every address reachable on the connection.
 *
 *  Packets are ran through the contained \ref Filter before being placed into
 *  the \ref PacketQueue given in the constructor.  Packets are read from the
 *  queue (for sending) by using the \ref next_packet method.
 *
 *  \note This disregards the destination address of the packet.
 *
 *  \param packet The packet to send.
 */
void Connection::send_to_all_(std::shared_ptr<const Packet> packet)
{
    bool accept = false;
    int priority = std::numeric_limits<int>::min();

    // Loop over addresses.
    for (const auto &source : pool_->addresses())
    {
        if (packet->source() != source)
        {
            // Filter packet/address combination.
            auto [accept_, priority_] = filter_->will_accept(*packet, source);

            // Update accept/priority.
            if (accept_)
            {
                accept = accept_;
                priority = std::max(priority, priority_);
            }
        }
    }

    // Add packet to the queue.
    if (accept)
    {
        queue_->push(std::move(packet), priority);
    }
}


/** Construct a connection.
 *
 *  \param filter The packet filter to use for determining whether and with what
 *      priority to add a packet to the queue for transmission.
 *  \param pool The AddressPool to use for keeping track of the addresses
 *      reachable by the connection.
 *  \param queue The PacketQueue to used to hold packets awaiting transmission.
 *  \param mirror Set to true if this is to be a mirror connection.  A mirror
 *      connection is one that will receive all packets, regardless of
 *      destination address.  The default is false.
 *  \throws std::invalid_argument if the given any of the \p filter, \p pool, or
 *      \p queue pointers are null.
 *  \remarks If the given \ref AddressPool and \ref PacketQueue are threadsafe
 *      then the connection will also be threadsafe.
 */
Connection::Connection(
    std::shared_ptr<Filter> filter, bool mirror,
    std::unique_ptr<AddressPool<>> pool,
    std::unique_ptr<PacketQueue> queue)
    : filter_(std::move(filter)), pool_(std::move(pool)),
      queue_(std::move(queue)), mirror_(mirror)
{
    if (filter_ == nullptr)
    {
        throw std::invalid_argument("Given filter pointer is null.");
    }

    if (pool_ == nullptr)
    {
        throw std::invalid_argument("Given pool pointer is null.");
    }

    if (queue_ == nullptr)
    {
        throw std::invalid_argument("Given queue pointer is null.");
    }
}


/** Add a MAVLink address to the connection.
 *
 *  This adds an address to the list of components that can be reached on this
 *  connection.
 *
 *  \note Addresses will be removed after the timeout set in the AddressPool
 *      given in the constructor.  Readding the address (even before this time
 *      runs out) will reset the timeout.
 *
 *  \param address The MAVLink address to add or update the timeout for.
 */
void Connection::add_address(MAVAddress address)
{
    pool_->add(std::move(address));
}



/** Get next packet to send.
 *
 *  Blocks until a packet is ready to be sent or the \p timeout expires.
 *  Returns nullptr in the later case.
 *
 *  \param timeout How long to block waiting for a packet.  Set to 0s for non
 *      blocking.
 *  \returns The next packet to send.  Or nullptr if the call times out waiting
 *      on a packet.
 */
std::shared_ptr<const Packet> Connection::next_packet(
    const std::chrono::nanoseconds &timeout)
{
    return queue_->pop(timeout);
}


/** Send a packet out on the connection.
 *
 *  Packets are ran through the contained \ref Filter before being placed into
 *  the \ref PacketQueue given in the constructor.  Packets are read from the
 *  queue (for sending) by using the \ref next_packet method.
 *
 *  \note If the packet has a destination address that is not 0.0 (the broadcast
 *      address) it will only be sent if that address is reachable on this
 *      connection.
 *
 *  \note If this is a mirror connection then the destination address of the
 *      packet is ignored.
 *
 *  \param packet The packet to send.
 *  \throws std::invalid_argument if the \p packet pointer is null.
 */
void Connection::send(std::shared_ptr<const Packet> packet)
{
    if (packet == nullptr)
    {
        throw std::invalid_argument("Given packet pointer is null.");
    }

    auto dest = packet->dest();

    if (dest && dest.value() != MAVAddress(0, 0) && !mirror_)
    {
        send_to_address_(std::move(packet), dest.value());
    }
    else
    {
        send_to_all_(std::move(packet));
    }
}
