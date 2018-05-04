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


#ifndef QUEUEDPACKET_HPP_
#define QUEUEDPACKET_HPP_


#include <memory>
#include <ostream>

#include "Packet.hpp"


/** A packet in the queue to be sent out.
 *
 *  Forms a node in the \ref PacketQueue class.
 *
 *  This is the data structure used in the priority queues used by the \ref
 *  Connection class.  It stores a MAVLink packet as well as a priority and
 *  ticket number used to maintain packet order in the priority queue when
 *  packets have the same priority.
 *
 *  \sa PacketQueue
 */
class QueuedPacket
{
    public:
        /** Copy constructor.
         *
         *  \param other QueuedPacket to copy from.
         */
        QueuedPacket(const QueuedPacket &other) = default;
        /** Move constructor.
         *
         *  \param other QueuedPacket to move from.
         */
        QueuedPacket(QueuedPacket &&other) = default;
        QueuedPacket(
            std::shared_ptr<const Packet> packet, int priority,
            unsigned long long ticket_number);
        std::shared_ptr<const Packet> packet() const;
        /** Assignment operator.
         *
         * \param other QueuedPacket to copy from.
         */
        QueuedPacket &operator=(const QueuedPacket &other) = default;
        /** Assignment operator (by move semantics).
         *
         * \param other QueuedPacket to move from.
         */
        QueuedPacket &operator=(QueuedPacket &&other) = default;

        friend bool operator==(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend bool operator!=(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend bool operator<(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend bool operator>(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend bool operator<=(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend bool operator>=(
            const QueuedPacket &lhs, const QueuedPacket &rhs);
        friend std::ostream &operator<<(
            std::ostream &os, const QueuedPacket &queued_packet);

    private:
        std::shared_ptr<const Packet> packet_;
        int priority_;
        unsigned long long ticket_number_;
};

bool operator==(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator!=(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator<(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator>(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator<=(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator>=(const QueuedPacket &lhs, const QueuedPacket &rhs);
std::ostream &operator<<(std::ostream &os, const QueuedPacket &queued_packet);


#endif // QUEUEDPACKET_HPP_
