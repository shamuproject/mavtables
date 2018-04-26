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


#include <chrono>
#include <memory>
#include <stdexcept>

#include "Connection.hpp"
#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "IPAddress.hpp"
#include "UDPInterface.hpp"
#include "UDPSocket.hpp"
#include "utility.hpp"


using namespace std::chrono_literals;


/** Update connections.
 *
 *  Adds a MAVLink address to the connection corresponding to the given IP
 *  address.  If this connection does not exist, it will be constructed from the
 *  connection factory.
 *
 *  \param mav_address The MAVLink address of the received packet.
 *  \param ip_address The IP address the packet was received on.
 */
void UDPInterface::update_connections_(
    const MAVAddress &mav_address, const IPAddress &ip_address)
{
    auto it = connections_.find(ip_address);

    if (it == connections_.end())
    {
        it = connections_.insert(
        {ip_address, connection_factory_->get(str(ip_address))}).first;
        connection_pool_->add(it->second);
    }

    it->second->add_address(mav_address);
}


/** Construct a UDP interface.
 *
 *  \param socket The UDP socket to communcate over.
 *  \param connection_pool The connection pool to use for sending packets and to
 *      register new connections with.
 *  \param connection_factory The connection factory to use for contructing
 *      connections.
 */
UDPInterface::UDPInterface(
    std::unique_ptr<UDPSocket> socket,
    std::shared_ptr<ConnectionPool> connection_pool,
    std::unique_ptr<ConnectionFactory<>> connection_factory)
    : socket_(std::move(socket)),
      connection_pool_(std::move(connection_pool)),
      connection_factory_(std::move(connection_factory)),
      last_ip_address_(IPAddress(0))
{
    if (socket_ == nullptr)
    {
        throw std::invalid_argument("Given socket pointer is null.");
    }

    if (connection_pool_ == nullptr)
    {
        throw std::invalid_argument("Given connection pool pointer is null.");
    }

    if (connection_factory_ == nullptr)
    {
        throw std::invalid_argument(
            "Given connection factory pointer is null.");
    }
}


/** \copydoc Interface::send_packet(const std::chrono::nanoseconds &)
 *
 *  Sends up to one packet from each connection belonging to the interface over
 *  the UDP socket.
 */
void UDPInterface::send_packet(const std::chrono::nanoseconds &timeout)
{
    bool not_first = false;

    // Wait for a packet on any of the interface's connections.
    if (connection_factory_->wait_for_packet(timeout))
    {
        for (auto &conn : connections_)
        {
            auto packet = conn.second->next_packet();

            // If connection has a packet send it.
            if (packet != nullptr)
            {
                socket_->send(packet->data(), conn.first);

                if (not_first)
                {
                    // Decrement semaphore once for each extra packet.
                    connection_factory_->wait_for_packet(0s);
                }

                not_first = true;
            }
        }
    }
}


/** \copydoc Interface::receive_packet(const std::chrono::nanoseconds &)
 *
 *  Receives up to one UDP packet worth of data and parses it into MAVLink
 *  packets before sending these packets onto the connection pool.
 */
void UDPInterface::receive_packet(const std::chrono::nanoseconds &timeout)
{
    auto [buffer, ip_address] = socket_->receive(timeout);

    if (!buffer.empty())
    {
        // Clear the parser if the IP address is different from the last UDP
        // packet received (we want complete MAVLink packets).
        if (ip_address != last_ip_address_)
        {
            parser_.clear();
            last_ip_address_ = ip_address;
        }

        // Parse the bytes.
        for (auto byte : buffer)
        {
            auto packet = parser_.parse_byte(byte);

            if (packet != nullptr)
            {
                update_connections_(packet->source(), ip_address);
                // It is a post condition of update_connections_ that there is a
                // connection for ip_address.
                packet->connection(connections_[ip_address]);
                connection_pool_->send(std::move(packet));
            }
        }
    }
}


/** \copydoc Interface::print_(std::ostream &os)const
 *
 *  Example:
 *  ```
 *  udp {
 *      port 14500;
 *      address 127.0.0.1;
 *  }
 *  ```
 */
std::ostream &UDPInterface::print_(std::ostream &os) const
{
    os << *socket_;
    return os;
}
