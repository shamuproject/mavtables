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
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

#include "config.hpp"
#include "Packet.hpp"
#include "QueuedPacket.hpp"


/** A threadsafe priority queue for MAVLink packets.
 *
 *  This priority queue will order packets based on priority but also maintains
 *  insertion order among packets of the same priority.
 *
 *  This is used to implement the packet priority of the firewall and to provide
 *  a queueing mechanism for packets when consumers are slower than the
 *  producers.
 *
 *  \sa QueuedPacket
 */
class PacketQueue
{
    public:
        PacketQueue(std::optional<std::function<void(void)>> callback = {});
        // LCOV_EXCL_START
        TEST_VIRTUAL ~PacketQueue() = default;
        // LCOV_EXCL_STOP
        TEST_VIRTUAL void close();
        TEST_VIRTUAL bool empty();
        TEST_VIRTUAL std::shared_ptr<const Packet> pop();
        TEST_VIRTUAL std::shared_ptr<const Packet> pop(
            const std::chrono::nanoseconds &timeout);
        TEST_VIRTUAL void push(
            std::shared_ptr<const Packet> packet, int priority = 0);

    private:
        // Variables.
        std::optional<std::function<void(void)>> callback_;
        unsigned long long ticket_;
        bool running_;
        std::priority_queue<QueuedPacket> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
        // Methods
        std::shared_ptr<const Packet> get_packet_();
};


#endif // PACKETQUEUE_HPP_
