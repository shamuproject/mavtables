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


#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_


#include <chrono>
#include <memory>

#include "ConnectionPool.hpp"


/** The base class for all interfaces.
 *
 *  Derived classes should add one or more connections to queue packets for
 *  sending in.
 */
class Interface
{
  public:
    virtual ~Interface();
    /** Send a packet from one of the interface's connections.
     *
     *  \param timeout The maximum amount of time to wait for a packet to be
     *      available for sending.
     */
    virtual void send_packet(const std::chrono::nanoseconds &timeout) = 0;
    /** Receive a packet on the interface.
     *
     *  \param timeout The maximum amount of time to wait for incoming data.
     */
    virtual void receive_packet(const std::chrono::nanoseconds &timeout) = 0;
};


#endif  // INTERFACE_HPP_
