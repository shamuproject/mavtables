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
#include <cstdint>
#include <map>
#include <memory>
#include <iterator>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "IPAddress.hpp"
#include "UDPInterface.hpp"
#include "UDPSocket.hpp"


void UDPInterface::update_connections_(
    const MAVAddress &mav_address, const IPAddress &ip_address)
{
    (void)mav_address;
    auto it = connections_.find(ip_address);
    if (it == connections_.end())
    {
        it = connections_.insert({ip_address, connection_factory_()}).first;
        connection_pool_->add(it->second);

    }
    it->second->add_address(mav_address);
}


UDPInterface::UDPInterface(
    std::unique_ptr<UDPSocket> socket,
    std::shared_ptr<ConnectionPool> connection_pool,
    std::function<std::unique_ptr<Connection>(void)> connection_factory)
    : socket_(std::move(socket)),
      connection_pool_(std::move(connection_pool)),
      connection_factory_(std::move(connection_factory)),
      last_ip_address_(IPAddress(0))
{
}


void UDPInterface::send_packet(const std::chrono::microseconds &timeout)
{
    (void)timeout;
}


void UDPInterface::receive_packet(const std::chrono::microseconds &timeout)
{
    auto [buffer, ip_address] = socket_->receive(timeout);

    if (!buffer.empty())
    {
        // Clear the parser if the IP address is different from the last UDP
        // packet recieved (we want complete MAVLink packets).
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
                connection_pool_->send(std::move(packet));
            }
        }
    }
}
