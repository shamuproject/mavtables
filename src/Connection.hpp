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

#include "Action.hpp"
// #include "ConnectionPool.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "QueuedPacket.hpp"


// Template class only for testing.
template <class TC = std::chrono::steady_clock>
class Connection
{
    private:
        bool mirror_;
        std::shared_ptr<Filter> filter_;
        std::map<MAVAddress, std::chrono::time_point<TC>> addresses_;
        std::chrono::milliseconds address_timeout_;
        std::priority_queue<std::unique_ptr<QueuedPacket>> packet_queue_;
        bool running_;
        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned long long ticket_number_;

        void purge_addresses_();
        void send_to_address_(
            std::shared_ptr<const Packet> packet, const MAVAddress &dest);
        void send_to_all_(std::shared_ptr<const Packet> packet);

    public:
        Connection(
            bool mirror = false,
            std::chrono::milliseconds address_timeout =
                std::chrono::milliseconds(120000));
        virtual ~Connection() = default;
        void add_address(MAVAddress address);
        std::shared_ptr<const Packet> next_packet();
        // virtual for testing
        virtual void send(std::shared_ptr<const Packet> packet);
        void shutdown();
};

#include <iostream>

template <class TC>
void Connection<TC>::purge_addresses_()
{
    auto current_time = TC::now();

    for (auto it = addresses_.cbegin(); it != addresses_.cend();)
    {
        if ((current_time - it->second) > address_timeout_)
        {
            it = addresses_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


template <class TC>
Connection<TC>::Connection(
    bool mirror, std::chrono::milliseconds address_timeout)
    : mirror_(mirror), address_timeout_(address_timeout),
      running_(true), ticket_number_(0)
{
}


template <class TC>
void Connection<TC>::add_address(MAVAddress address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    addresses_.insert_or_assign(std::move(address), TC::now());
}


/** Get next packet to send on connection.
 *
 *  Blocks until a packet is ready to be sent or the connection is shut down.
 *  Returns nullptr in the later case.
 */
template <class TC>
std::shared_ptr<const Packet> Connection<TC>::next_packet()
{
    // Wait for available packet.
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]
    {
        return !running_ || !packet_queue_.empty();
    });

    // Return the packet if connection is still running.
    if (running_)
    {
        std::shared_ptr<const Packet>packet = packet_queue_.top();
        packet_queue_.pop();
        return packet;
    }

    return nullptr;
}


template <class TC>
void Connection<TC>::send_to_address_(
    std::shared_ptr<const Packet> packet, const MAVAddress &dest)
{
    auto current_time = TC::now();

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


template <class TC>
void Connection<TC>::send_to_all_(std::shared_ptr<const Packet> packet)
{
    auto current_time = TC::now();
    bool accept = false;
    int priority = std::numeric_limits<int>::min();

    // Loop over addresses.
    for (auto it = addresses_.cbegin(); it != addresses_.cend();)
    {
        // Remove the address if it has expired.
        if ((current_time - it->second) > address_timeout_)
        {
            it = addresses_.erase(it);
        }
        // Run packet/address combination through the filter.
        else
        {
            auto [accept_, priority_] =
                filter_->will_accept(*packet, it->first);

            if (accept_)
            {
                accept = accept_;
                priority = std::max(priority, priority_);
            }

            ++it;
        }
    }

    // Add packet to the queue.
    if (accept)
    {
        packet_queue_.push(
            std::make_unique<QueuedPacket>(
                std::move(packet), priority, ticket_number_++));
    }
}


template <class TC>
void Connection<TC>::send(std::shared_ptr<const Packet> packet)
{
    // TODO: Check for null packet.
    auto dest = packet->dest();

    if (dest && !mirror_)
    {
        send_to_address_(std::move(packet), dest.value());
    }
    else
    {
        send_to_all_(std::move(packet));
    }
}


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
