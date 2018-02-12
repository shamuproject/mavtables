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


class QueuedPacket
{
    private:
        std::shared_ptr<Packet> packet_;
        int priority_;
        unsigned long long ticket_number_;

    public:
        /** Copy constructor.
         *
         *  \param other QueuedPacket to copy.
         */
        QueuedPacket(const QueuedPacket &other) = default;
        /** Move constructor.
         *
         *  \param other QueuedPacket to move from.
         */
        QueuedPacket(QueuedPacket &&other) = default;
        QueuedPacket(
            std::shared_ptr<Packet> packet, int priority,
            unsigned long long ticket_number);
        std::shared_ptr<Packet> packet() const;
        /** Assignment operator.
         *
         * \param other QueuedPacket to copy.
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

};

bool operator==(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator!=(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator<(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator>(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator<=(const QueuedPacket &lhs, const QueuedPacket &rhs);
bool operator>=(const QueuedPacket &lhs, const QueuedPacket &rhs);
std::ostream &operator<<(std::ostream &os, const QueuedPacket &queued_packet);


#endif // QUEUEDPACKET_HPP_
