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


#ifndef UNIXUDPSOCKET_HPP_
#define UNIXUDPSOCKET_HPP_


#include <chrono>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

#include "IPAddress.hpp"
#include "UDPSocket.hpp"
#include "UnixSyscalls.hpp"


/** A Unix UDP socket, listening on a port/address combination.
 */
class UnixUDPSocket : public UDPSocket
{
    public:
        UnixUDPSocket(
            unsigned int port, std::optional<IPAddress> address = {},
            unsigned long max_bitrate = 0,
            std::unique_ptr<UnixSyscalls> syscalls =
                std::make_unique<UnixSyscalls>());
        virtual ~UnixUDPSocket();
        virtual void send(
            const std::vector<uint8_t> &data, const IPAddress &address) final;
        virtual std::pair<std::vector<uint8_t>, IPAddress> receive(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero()) final;

    protected:
        std::ostream &print_(std::ostream &os) const final;

    private:
        // Variables
        unsigned int port_;
        std::optional<IPAddress> address_;
        unsigned long max_bitrate_;
        std::unique_ptr<UnixSyscalls> syscalls_;
        int socket_;
        std::chrono::time_point<std::chrono::steady_clock> next_time_;
        // Methods
        void create_socket_();
        std::pair<std::vector<uint8_t>, IPAddress> receive_();
};


#endif // UNIXUDPSOCKET_HPP_
