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


#ifndef PACKETQUEUE_HPP_
#define PACKETQUEUE_HPP_


#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "config.hpp"
#include "Packet.hpp"
#include "QueuedPacket.hpp"


/** A threadsafe priority queue for MAVLink packets.
 */
class PacketQueue
{
    private:
        std::priority_queue<QueuedPacket> queue_;
        unsigned long long ticket_;
        bool running_;
        std::mutex mutex_;
        std::condition_variable cv_;

    public:
        PacketQueue();
        TEST_VIRTUAL ~PacketQueue() = default;
        TEST_VIRTUAL std::shared_ptr<const Packet> pop(bool blocking = false);
        TEST_VIRTUAL void push(
                std::shared_ptr<const Packet> packet, int priority = 0);
        TEST_VIRTUAL void shutdown();
};


#endif // PACKETQUEUE_HPP_
