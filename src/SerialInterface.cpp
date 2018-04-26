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
#include "ConnectionPool.hpp"
#include "SerialInterface.hpp"
#include "SerialPort.hpp"


/** Construct a serial port interface.
 *
 *  \param port The serial port to communcate over.
 *  \param connection_pool The connection pool to use for sending packets and to
 *      register the \p connection with.
 *  \param connection The connection to send packets from.  This will be
 *      registered with the given \ref ConnectionPool.
 */
SerialInterface::SerialInterface(
    std::unique_ptr<SerialPort> port,
    std::shared_ptr<ConnectionPool> connection_pool,
    std::unique_ptr<Connection> connection)
    : port_(std::move(port)),
      connection_pool_(std::move(connection_pool)),
      connection_(std::move(connection))
{
    if (port_ == nullptr)
    {
        throw std::invalid_argument("Given serial port pointer is null.");
    }

    if (connection_pool_ == nullptr)
    {
        throw std::invalid_argument("Given connection pool pointer is null.");
    }

    if (connection_ == nullptr)
    {
        throw std::invalid_argument("Given connection pointer is null.");
    }

    connection_pool_->add(connection_);
}


/** \copydoc Interface::send_packet(const std::chrono::nanoseconds &)
 *
 *  Writes up to one packet from the contained connection to the serial port.
 */
void SerialInterface::send_packet(const std::chrono::nanoseconds &timeout)
{
    auto packet = connection_->next_packet(timeout);

    if (packet != nullptr)
    {
        port_->write(packet->data());
    }
}


/** \copydoc Interface::receive_packet(const std::chrono::nanoseconds &)
 *
 *  Reads the data in the serial port's receive buffer or waits for up to\p
 *  timeout until data arives if no data is present the serial port buffer.
 */
void SerialInterface::receive_packet(const std::chrono::nanoseconds &timeout)
{
    auto buffer = port_->read(timeout);

    if (!buffer.empty())
    {
        // Parse the bytes.
        for (auto byte : buffer)
        {
            auto packet = parser_.parse_byte(byte);

            if (packet != nullptr)
            {
                packet->connection(connection_);
                connection_->add_address(packet->source());
                connection_pool_->send(std::move(packet));
            }
        }
    }
}


/** \copydoc Interface::print_(std::ostream &os)const
 *
 *  Example:
 *  ```
 *  serial {
 *      device /dev/ttyUSB0;
 *      baudrate 115200;
 *      flow_control yes;
 *  }
 *  ```
 */
std::ostream &SerialInterface::print_(std::ostream &os) const
{
    os << *port_;
    return os;
}
