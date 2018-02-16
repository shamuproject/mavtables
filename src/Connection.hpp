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


#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_


#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <vector>

#include "Action.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "QueuedPacket.hpp"


/** A connection that packets can be sent out over.
 *
 *  \note The only reason this is a template class is for unit testing purposes.
 */
template <class TC = std::chrono::steady_clock>
class Connection
{
    private:
        // Variables
        std::shared_ptr<Filter> filter_;
        bool mirror_;
        std::map<MAVAddress, std::chrono::time_point<TC>> addresses_;
        std::chrono::milliseconds address_timeout_;
        std::priority_queue<std::unique_ptr<QueuedPacket>> packet_queue_;
        unsigned long long ticket_number_;
        bool running_;
        std::mutex mutex_;
        std::condition_variable cv_;
        // Methods
        std::vector<MAVAddress> get_addresses_();
        void send_to_address_(
            std::shared_ptr<const Packet> packet, const MAVAddress &dest);
        void send_to_all_(std::shared_ptr<const Packet> packet);

    public:
        Connection(
            std::shared_ptr<Filter> filter, bool mirror = false,
            std::chrono::milliseconds address_timeout =
                std::chrono::milliseconds(120000));
        TEST_VIRTUAL ~Connection() = default;
        void add_address(MAVAddress address);
        std::shared_ptr<const Packet> next_packet(bool blocking = false);
        // virtual for testing
        virtual void send(std::shared_ptr<const Packet> packet);
        void shutdown();
};


/** Get a vector of the addresses reachable on this connection.
 *
 *  \returns A vector of the addresses that are handled by this connection.
 */
template <class TC>
std::vector<MAVAddress> Connection<TC>::get_addresses_()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MAVAddress> addresses;
    addresses.reserve(addresses_.size());
    auto current_time = TC::now();

    // Loop over addresses.
    for (auto it = addresses_.cbegin(); it != addresses_.cend();)
    {
        // Remove the address if it has expired.
        if ((current_time - it->second) > address_timeout_)
        {
            it = addresses_.erase(it);
        }
        // Store the address.
        else
        {
            addresses.push_back((it++)->first);
        }
    }

    return addresses;
}


/** Send a packet to a particular address.
 *
 *  Packets are ran through the contained filter before being placed into a
 *  priority queue based on priority followed by insertion time.  Packets are
 *  read from the queue (for sending) by the \ref next_packet method.
 *
 *  \note This disregards the destination address of the packet.
 *
 *  \param packet The packet to send.
 *  \param dest The address to send the packet to, if this address is not
 *      handled by this connection the packet will be silently dropped.
 */
template <class TC>
void Connection<TC>::send_to_address_(
    std::shared_ptr<const Packet> packet, const MAVAddress &dest)
{
    auto current_time = TC::now();
    std::lock_guard<std::mutex> lock(mutex_);

    // THIS WILL THROW UNCAUGHT ERROR
    // Remove address if expired.
    if (current_time - addresses_.at(dest) < address_timeout_)
    {
        addresses_.erase(dest);
        return;
    }

    // Run packet/address combination through the filter.
    auto [accept, priority] = filter_->will_accept(*packet, dest);

    // Add packet to the queue.
    if (accept)
    {
        packet_queue_.push(
            std::make_unique<QueuedPacket>(
                std::move(packet), priority, ticket_number_++));
    }
}


/** Send a packet to every address reachable on the connection.
 *
 *  Packets are ran through the contained filter before being placed into a
 *  priority queue based on priority followed by insertion time.  Packets are
 *  read from the queue (for sending) by the \ref next_packet method.
 *
 *  \note This disregards the destination address of the packet.
 *
 *  \param packet The packet to send.
 */
template <class TC>
void Connection<TC>::send_to_all_(std::shared_ptr<const Packet> packet)
{
    bool accept = false;
    int priority = std::numeric_limits<int>::min();

    // Loop over addresses.
    for (const auto &i : get_addresses_())
    {
        // Filter packet/address combination.
        auto [accept_, priority_] = filter_->will_accept(*packet, i);

        // Update accept/priority.
        if (accept_)
        {
            accept = accept_;
            priority = std::max(priority, priority_);
        }
    }

    // Add packet to the queue.
    if (accept)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        packet_queue_.push(
            std::make_unique<QueuedPacket>(
                std::move(packet), priority, ticket_number_++));
    }
}


/** Construct a connection.
 *
 *  \param filter The packet filter to use for determining whether and with what
 *      priority to add a packet to the queue for transmission.
 *  \param mirror Set to true if this is to be a mirror connection.  A mirror
 *      connection is one that will receive all packets, regardless of
 *      destination address.  The default is false.
 *  \param address_timeout The amount of time (in milliseconds) allowed between
 *      HEARTBEAT packets before considering the component offline.  The default
 *      is 120000 milliseconds (120 seconds).
 *  \throws std::invalid_argument if the given \p filter pointer is null.
 */
template <class TC>
Connection<TC>::Connection(
    std::shared_ptr<Filter> filter, bool mirror,
    std::chrono::milliseconds address_timeout)
    : filter_(std::move(filter)), mirror_(mirror),
      address_timeout_(address_timeout), ticket_number_(0), running_(true)
{
    if (filter_ == nullptr)
    {
        throw std::invalid_argument("Given filter pointer is null.");
    }
}


/** Add a MAVLink address to the connection.
 *
 *  This adds an address to the list of components that can be reached on this
 *  connection.
 *
 *  \note Addresses will be removed after the address_timeout (set in the
 *      constructor) has run out.  Readding the address (even before this time
 *      runs out) will reset the timeout.
 *
 *  \param address The MAVLink address to add or update the timeout for.
 */
template <class TC>
void Connection<TC>::add_address(MAVAddress address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    addresses_.insert_or_assign(std::move(address), TC::now());
}


/** Get next packet to send.
 *
 *  Blocks until a packet is ready to be sent or the connection is shut down.
 *  Returns nullptr in the later case.
 *
 *  \note If the connection is \ref shutdown then a nullptr is returned
 *      immediately.
 *
 *  \param blocking If true, this function will block if there are no packets.
 *      If false, a nullptr will be returned if there are no packets in the
 *      queue.  The default value is false.
 */
template <class TC>
std::shared_ptr<const Packet> Connection<TC>::next_packet(bool blocking)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (blocking)
    {
        // Wait for available packet (or shutdown).
        cv_.wait(lock, [this]
        {
            return !running_ || !packet_queue_.empty();
        });
    }

    // Return the packet if connection is running and queue is not empty.
    if (running_ && !packet_queue_.empty())
    {
        std::shared_ptr<const Packet>packet = packet_queue_.top();
        packet_queue_.pop();
        return packet;
    }

    return nullptr;
}


/** Send a packet out on the connection.
 *
 *  Packets are ran through the contained filter before being placed into a
 *  priority queue based on priority followed by insertion time.  Packets are
 *  read from the queue (for sending) by the \ref next_packet method.
 *
 *  \note If the packet has a destination address has a destination address that
 *      is not 0.0 it will only be sent if that address is reachable on this
 *      connection.
 *
 *  \note If this is a mirror connection then the destination address of the
 *      packet is ignored.
 *
 *  \param packet The packet to send.
 *  \throws std::invalid_argument if the \p packet pointer is null.
 */
template <class TC>
void Connection<TC>::send(std::shared_ptr<const Packet> packet)
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


/** Shutdown the connection.
 *
 *  This releases any blocking calls.  In particular the \ref next_packet
 *  function.
 */
template <class TC>
void Connection<TC>::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}


#endif // CONNECTION_HPP_
