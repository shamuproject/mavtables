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


#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#include "Packet.hpp"
#include "PacketQueue.hpp"
#include "QueuedPacket.hpp"


std::shared_ptr<const Packet> PacketQueue::pop(bool blocking)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (blocking)
    {
        // Wait for available packet (or shutdown).
        cv_.wait(lock, [this]
        {
            return !running_ || !queue_.empty();
        });
    }

    // Return the packet if connection is running and queue is not empty.
    if (running_ && !queue_.empty())
    {
        std::shared_ptr<const Packet> packet = queue_.top()->packet();
        queue_.pop();
        return packet;
    }

    return nullptr;
}


void PacketQueue::push(std::shared_ptr<const Packet> packet, int priority)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::make_unique<QueuedPacket>(
                    std::move(packet), priority, ticket_++));
}


void PacketQueue::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}
