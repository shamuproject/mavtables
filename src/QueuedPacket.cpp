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


#include <limits>
#include <memory>
#include <ostream>

#include "Packet.hpp"
#include "QueuedPacket.hpp"


/** Construct a queued packet.
 *
 *  \param packet The packet to store in the queue.
 *  \param priority The priority to send the packet with, higher numbers result
 *      in a higher priority.
 *  \param ticket_number A number that should always be incremented for each
 *      queued packet created per packet queue.
 *  \throws std::invalid_argument if the given packet pointer is nullptr.
 */
QueuedPacket::QueuedPacket(
    std::shared_ptr<const Packet> packet, int priority,
    unsigned long long ticket_number)
    : packet_(std::move(packet)), priority_(priority),
      ticket_number_(ticket_number)
{
    if (packet_ == nullptr)
    {
        throw std::invalid_argument("Given packet pointer is null.");
    }
}


/** Return the contained packet.
 *
 *  \returns The contained MAVLink packet.
 */
std::shared_ptr<const Packet> QueuedPacket::packet() const
{
    return packet_;
}


/** Equality comparison.
 *
 *  \note It should never be the case that two queued packets have the same
 *  ticket number and thus two queued packets should never be equal.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs and \p rhs have the same priority and ticket number.
 *  \retval false if \p lhs and \p rhs have do not have the same priority and
 *      ticket number.
 */
bool operator==(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    return (lhs.priority_ == rhs.priority_) &&
           (lhs.ticket_number_ == rhs.ticket_number_);
}


/** Inequality comparison.
 *
 *  \note It should never be the case that two queued packets have the same
 *  ticket number and thus two queued packets should never be equal.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs and \p rhs have do not have the same priority and
 *      ticket number.
 *  \retval false if \p lhs and \p rhs have the same priority and ticket number.
 */
bool operator!=(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    return (lhs.priority_ != rhs.priority_) ||
           (lhs.ticket_number_ != rhs.ticket_number_);
}


#include <iostream>


/** Less than comparison.
 *
 *  The priority is considered first, followed by the ticket number in reverse
 *  order (lower ticket number is greater).
 *
 *  \note The ticket number is considered to be a wrapping integer and thus
 *      numbers that are within `std::numeric_limits<unsigned long
 *      long>::%max()/2` of each other are considered in the same range.  In
 *      this way 0 is greater than `std::numeric_limits<unsigned long
 *      long>::%max()`.  Because of this it is important that anything relying
 *      on ordering must not contain a range of ticket numbers equal to or
 *      grater than `std::numeric_limits<unsigned long long>::%max()/2`.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs is less than \p rhs.
 *  \retval false if \p lhs is not less than \p rhs.
 */
bool operator<(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    // auto a = lhs.ticket_number_ - rhs.ticket_number_;
    // auto b = std::numeric_limits<unsigned long long>::max() / 2;
    // auto c = lhs.ticket_number_ - rhs.ticket_number_ >
    //          std::numeric_limits<unsigned long long>::max() / 2;
    // std::cout << "a: " << a << std::endl;
    // std::cout << "b: " << b << std::endl;
    // std::cout << "c: " << c << std::endl;
    return (lhs.priority_ < rhs.priority_) || (lhs.priority_ == rhs.priority_ &&
            (lhs.ticket_number_ - rhs.ticket_number_ >
             std::numeric_limits<unsigned long long>::max() / 2));
}


/** Greater than comparison.
 *
 *  The priority is considered first, followed by the ticket number in reverse
 *  order (lower ticket number is greater).
 *
 *  \note The ticket number is considered to be a wrapping integer and thus
 *      numbers that are within `std::numeric_limits<unsigned long
 *      long>::%max()/2` of each other are considered in the same range.  In
 *      this way 0 is greater than `std::numeric_limits<unsigned long
 *      long>::%max()`.  Because of this it is important that anything relying
 *      on ordering must not contain a range of ticket numbers equal to or
 *      grater than `std::numeric_limits<unsigned long long>::%max()/2`.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs is less than \p rhs.
 *  \retval false if \p lhs is not less than \p rhs.
 */
bool operator>(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    // auto a = rhs.ticket_number_ - lhs.ticket_number_;
    // auto b = std::numeric_limits<unsigned long long>::max() / 2;
    // auto c = rhs.ticket_number_ - lhs.ticket_number_ >
    //          std::numeric_limits<unsigned long long>::max() / 2;
    // std::cout << "a: " << a << std::endl;
    // std::cout << "b: " << b << std::endl;
    // std::cout << "c: " << c << std::endl;
    return (lhs.priority_ > rhs.priority_) || (lhs.priority_ == rhs.priority_ &&
            (rhs.ticket_number_ - lhs.ticket_number_ >
             std::numeric_limits<unsigned long long>::max() / 2));
}


/** Less than or equal comparison.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs is less than or eqaul to \p rhs.
 *  \retval false if \p lhs is greater than \p rhs.
 */
bool operator<=(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    return lhs == rhs || lhs < rhs;
}


/** Greater than or equal comparison.
 *
 *  \relates QueuedPacket
 *  \param lhs The left hand side queued packet.
 *  \param rhs The right hand side queued packet.
 *  \retval true if \p lhs is greater than or equal to \p rhs.
 *  \retval false if \p lhs is less than \p rhs.
 */
bool operator>=(const QueuedPacket &lhs, const QueuedPacket &rhs)
{
    return lhs == rhs || lhs > rhs;
}


/** Print the packet to the given output stream.
 *
 *  Some examples are:
 *      - `HEARTBEAT (#1) from 16.8 (v1.0) with priority -3`
 *      - `PING (#4) from 128.4 to 16.8 (v2.0) with priority 0`
 *      - `DATA_TRANSMISSION_HANDSHAKE (#130) from 16.8 (v2.0) with priority 3`
 *      - `ENCAPSULATED_DATA (#131) from 128.4 (v2.0) with priority 1`
 *
 *  \relates QueuedPacket
 *  \param os The output stream to print to.
 *  \param queued_packet The queued packet to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const QueuedPacket &queued_packet)
{
    os << *queued_packet.packet_;
    os << " with priority " << queued_packet.priority_;
    return os;
}
