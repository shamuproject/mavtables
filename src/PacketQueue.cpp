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
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>

#include "Packet.hpp"
#include "PacketQueue.hpp"
#include "QueuedPacket.hpp"


/** Get packet from queue.
 *
 *  \note The mutex must be locked.
 *
 *  \returns The next packet or nullptr if the queue has been closed or is
 *      empty.
 */
std::shared_ptr<const Packet> PacketQueue::get_packet_()
{
    if (running_ && !queue_.empty())
    {
        std::shared_ptr<const Packet> packet = queue_.top().packet();
        queue_.pop();
        return packet;
    }

    return nullptr;
}


/** Construct a packet queue.
 *
 *  \param callback A function to call whenever a new packet is added to the
 *      queue.  This allows the queue to signal when it has become non empty.
 *      The default is no callback {}.
 */
PacketQueue::PacketQueue(std::optional<std::function<void(void)>> callback)
    : callback_(std::move(callback)), ticket_(0), running_(true)
{
}


/** Close the queue.
 *
 *  This will release any blocking calls to \ref pop.
 *  \remarks
 *      Threadsafe (locking).
 */
void PacketQueue::close()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
}


/** Determine if the packet queue is empty or not.
 *
 *  retval true There are no packets in the queue.
 *  retval false There is at least one packet in the queue.
 */
bool PacketQueue::empty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}


/** Remove and return the packet at the front of the queue.
 *
 *  This version will block on an empty queue and will not return until the
 *  queue becomes non empty or is closed with \ref close.
 *
 *  \returns The packet that was at the front of the queue, or nullptr if the
 *      queue was closed.
 *  \remarks
 *      Threadsafe (locking).
 *  \sa pop(const std::chrono::nanoseconds &)
 */
std::shared_ptr<const Packet> PacketQueue::pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    // Wait for available packet.
    cv_.wait(lock, [this]() { return !running_ || !queue_.empty(); });
    // Return the packet if the queue is running and is not empty.
    return get_packet_();
}


/** Remove and return the packet at the front of the queue.
 *
 *  This version will block on an empty queue and will not return until the
 *  queue becomes non empty, is closed with \ref close, or the timeout has
 *  expired.
 *
 *  \param timeout How long to block waiting for an empty queue.  Set to 0s for
 *      non blocking.
 *  \returns The packet that was at the front of the queue, or nullptr if the
 *      queue was closed or the timeout expired.
 *  \remarks
 *      Threadsafe (locking).
 */
std::shared_ptr<const Packet>
PacketQueue::pop(const std::chrono::nanoseconds &timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout > std::chrono::nanoseconds::zero())
    {
        // Wait for available packet (or the queue to be closed).
        cv_.wait_for(
            lock, timeout, [this]() { return !running_ || !queue_.empty(); });
    }

    // Return the packet if the queue is running and is not empty.
    return get_packet_();
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

    // Add the packet to the queue.
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace(std::move(packet), priority, ticket_++);
    }
    // Notify a waiting pop.
    cv_.notify_one();

    // Trigger the callback.
    if (callback_)
    {
        (*callback_)();
    }
}
