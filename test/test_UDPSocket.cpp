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

#include "UDPSocket.hpp"


using namespace std::chrono_literals;

// namespace
// {
//
// #ifdef __clang__
//     #pragma clang diagnostic push
//     #pragma clang diagnostic ignored "-Wweak-vtables"
// #endif
//
//     // Subclass of Interface used for testing the abstract class Interface.
//     class UDPSocketTestClass : public UDPSocket
//     {
//         public:
//             unsigned int port_;
//             std::optional<IPAddress> address_;
//             std::vector<std::chrono::microseconds> _timeout;
//             std::vector<std::chrono::microseconds> receive_timeout;
//             UDPSocket(unsigned int port, std::optional<IPAddress> address = {})
//                 : port_(port), address_(std::move(address))
//             {
//             }
//             // LCOV_EXCL_START
//             ~UDPSocketTestClass() = default;
//             // LCOV_EXCL_STOP
//             void send(
//                 std::vector<uint8_t>::const_iterator first,
//                 std::vector<uint8_t>::const_iterator last,
//                 const IPAddress &address) final
//             {
//
//             }
//             virtual IPAddress receive(
//                 std::back_insert_iterator<std::vector<uint8_t>> it,
//                 const std::chrono::microseconds &timeout =
//                     std::chrono::microseconds(100000)) final
//             {
//                 std::vector<uint8_t> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//                 std::copy(vec.begin(), vec.end(), it);
//             }
//     };
//
// #ifdef __clang__
//     #pragma clang diagnostic pop
// #endif
//
// }


TEST_CASE("UDPSocket's 'send' method accepts a vector and address.",
          "[UDPSocket]")
{
    // This test ensures the non virtual convenience send method correctly calls
    // the pure virtual send method that takes iterators.
    fakeit::Mock<UDPSocket> mock_socket;
    fakeit::Fake(OverloadedMethod(
                     mock_socket, send,
                     void(std::vector<uint8_t>::const_iterator,
                          std::vector<uint8_t>::const_iterator,
                          const IPAddress & address)));
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    IPAddress addr("192.168.0.0");
    socket.send(vec, addr);
    fakeit::Verify(OverloadedMethod(
                       mock_socket, send,
                       void(std::vector<uint8_t>::const_iterator,
                            std::vector<uint8_t>::const_iterator,
                            const IPAddress & address)).Matching(
                       [&](auto a, auto b, auto c)
    {
        return a == vec.begin() && b == vec.end() && c == addr;
    })).Once();
}


TEST_CASE("UDPSocket's 'receive' method takes a timeout and returns a vector "
          "of bytes and the IP address that sent them.", "[UDPSocket]")
{
    // This test ensures the non virtual convenience receive method correctly
    // calls the pure virtual receive method that takes iterators to write the
    // data into.
    fakeit::Mock<UDPSocket> mock_socket;
    fakeit::When(OverloadedMethod(
                     mock_socket, receive,
                     IPAddress(std::back_insert_iterator<std::vector<uint8_t>>,
                               const std::chrono::microseconds &))).AlwaysDo(
                                   [](auto a, auto b)
    {
        (void)b;
        std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::copy(vec.begin(), vec.end(), a);
        return IPAddress("192.168.0.0");
    });
    UDPSocket &socket = mock_socket.get();
    std::vector<uint8_t> vec_compare = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto [data, addr] = socket.receive(1s);
    REQUIRE(data == vec_compare);
    REQUIRE(addr == IPAddress("192.168.0.0"));
    fakeit::Verify(OverloadedMethod(
                     mock_socket, receive,
                     IPAddress(std::back_insert_iterator<std::vector<uint8_t>>,
                               const std::chrono::microseconds &)).Matching(
                                   [](auto a, auto b)
    {
        (void)a;
        return b == 1s;
    })).Exactly(1);
}
