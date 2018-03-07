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


#include <algorithm>
#include <chrono>
#include <iterator>
#include <optional>
#include <utility>

#include <catch.hpp>
#include <fakeit.hpp>

#include "IPAddress.hpp"
#include "UDPSocket.hpp"


using namespace std::chrono_literals;


TEST_CASE(
    "UDPSocket's 'send' method accepts a vector of bytes and an address.",
    "[UDPSocket]")
{
    // This test ensures that one send method calls the other.
    UDPSocket udp;
    fakeit::Mock<UDPSocket> mock_socket(udp);
    fakeit::Fake(OverloadedMethod(
        mock_socket, send,
        void(
            std::vector<uint8_t>::const_iterator,
            std::vector<uint8_t>::const_iterator, const IPAddress &address)));
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    IPAddress addr("192.168.0.0");
    socket.send(vec, addr);
    fakeit::Verify(
        OverloadedMethod(
            mock_socket, send,
            void(
                std::vector<uint8_t>::const_iterator,
                std::vector<uint8_t>::const_iterator, const IPAddress &address))
            .Matching([&](auto a, auto b, auto c) {
                return a == vec.begin() && b == vec.end() && c == addr;
            }))
        .Once();
}


TEST_CASE(
    "UDPSocket's 'send' method accepts two vector iterators and an "
    "address.",
    "[UDPSocket]")
{
    // This test ensures that one send method calls the other.
    UDPSocket udp;
    fakeit::Mock<UDPSocket> mock_socket(udp);
    std::vector<uint8_t> send_vec;
    fakeit::When(OverloadedMethod(
                     mock_socket, send,
                     void(const std::vector<uint8_t> &, const IPAddress &)))
        .AlwaysDo([&](auto a, auto b) {
            (void)b;
            send_vec = a;
        });
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    IPAddress addr("192.168.0.0");
    socket.send(vec.begin(), vec.end(), addr);
    fakeit::Verify(OverloadedMethod(
                       mock_socket, send,
                       void(const std::vector<uint8_t> &, const IPAddress &)))
        .Once();
    REQUIRE(vec == send_vec);
}


TEST_CASE(
    "UDPSocket's 'receive' method takes a timeout and returns a vector "
    "of bytes and the IP address that sent them.",
    "[UDPSocket]")
{
    // This test ensures that one receive method calls the other.
    UDPSocket udp;
    fakeit::Mock<UDPSocket> mock_socket(udp);
    fakeit::When(OverloadedMethod(
                     mock_socket, receive,
                     IPAddress(
                         std::back_insert_iterator<std::vector<uint8_t>>,
                         const std::chrono::nanoseconds &)))
        .AlwaysDo([](auto a, auto b) {
            (void)b;
            std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("192.168.0.0");
        });
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec_compare = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::chrono::nanoseconds timeout = 1ms;
    auto[data, addr] = socket.receive(timeout);
    REQUIRE(data == vec_compare);
    REQUIRE(addr == IPAddress("192.168.0.0"));
    fakeit::Verify(OverloadedMethod(
                       mock_socket, receive,
                       IPAddress(
                           std::back_insert_iterator<std::vector<uint8_t>>,
                           const std::chrono::nanoseconds &))
                       .Matching([](auto a, auto b) {
                           (void)a;
                           return b == 1ms;
                       }))
        .Once();
}


TEST_CASE(
    "UDPSocket's 'receive' method takes a back inserter and a timeout "
    "and returns the IP address that sent the bytes written to the "
    "back inserter.",
    "[UDPSocket]")
{
    // This test ensures that one receive method calls the other.
    using receive_type = std::pair<std::vector<uint8_t>, IPAddress>(
        const std::chrono::nanoseconds &);
    UDPSocket udp;
    fakeit::Mock<UDPSocket> mock_socket(udp);
    fakeit::When(OverloadedMethod(mock_socket, receive, receive_type))
        .AlwaysDo([](auto a) {
            (void)a;
            std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            return std::pair<std::vector<uint8_t>, IPAddress>(
                vec, IPAddress("192.168.0.0"));
        });
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec_compare = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::chrono::nanoseconds timeout = 1ms;
    std::vector<uint8_t> data;
    auto addr = socket.receive(std::back_inserter(data), timeout);
    REQUIRE(data == vec_compare);
    REQUIRE(addr == IPAddress("192.168.0.0"));
    fakeit::Verify(OverloadedMethod(mock_socket, receive, receive_type)
                       .Matching([](auto a) { return a == 1ms; }))
        .Once();
}
