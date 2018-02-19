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
#include <stdexcept>
#include <utility>

#include "Packet.hpp"
#include "PacketQueue.hpp"
#include "QueuedPacket.hpp"


/** Construct a packet queue.
 */
PacketQueue::PacketQueue()
    : ticket_(0), running_(true)
{
}


/** Remove and return the packet at the front of the queue.
 *
 *  \param blocking
 *      * **true** %If the queue is empty the call will block until a packet is
 *          available or the queue is shutdown.  In the latter case a null
 *          pointer is returned.
 *      * **false** %If the queue is empty (or shutdown) a null pointer is
 *          returned.  This is the default.
 *   \returns The packet that was at the front of the queue.
 *  \remarks
 *      Threadsafe (locking).
 */
std::shared_ptr<const Packet> PacketQueue::pop(bool blocking)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (blocking)
    {
        // Wait for available packet (or shutdown).
        cv_.wait(lock, [this]()
        {
            return !running_ || !queue_.empty();
        });
    }

    // Return the packet if connection is running and queue is not empty.
    if (running_ && !queue_.empty())
    {
        std::shared_ptr<const Packet> packet = queue_.top().packet();
        queue_.pop();
        return packet;
    }

    return nullptr;
}


/** Add a new packet to the queue, with a priority.
 *
 *  A higher \p priority will result in the packet being pushed to the front of
 *  the queue.  When priorities are equal the order in which the packets were
 *  added to the queue is maintained.
 *
 *  \param packet The packet to add to the queue.  It must not be nullptr.
 *  \param priority The priority to use when adding it to the queue.  The
 *      default is 0.
 *  \throws std::invalid_argument if the packet pointer is null.
 *  \remarks
 *      Threadsafe (locking).
 */
void PacketQueue::push(std::shared_ptr<const Packet> packet, int priority)
{
    if (packet == nullptr)
    {
        throw std::invalid_argument("Given packet pointer is null.");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(std::move(packet), priority, ticket_++);
    cv_.notify_one();
}


/** Shutdown the queue.
 *
 *  This will release any blocking calls to \ref pop.
 *  \remarks
 *      Threadsafe (locking).
 */
void PacketQueue::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}
