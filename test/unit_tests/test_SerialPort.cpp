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

#include "SerialPort.hpp"


using namespace std::chrono_literals;


TEST_CASE("SerialPort's 'read' method takes a timeout and returns a vector "
          "of bytes.", "[SerialPort]")
{
    // This test ensures that one receive method calls the other.
    SerialPort serial;
    fakeit::Mock<SerialPort> mock_port(serial);
    fakeit::When(
        OverloadedMethod(
            mock_port, read,
            void(std::back_insert_iterator<std::vector<uint8_t>>,
                 const std::chrono::nanoseconds &))).AlwaysDo(
                     [](auto a, auto b)
    {
        (void)b;
        std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::copy(vec.begin(), vec.end(), a);
    });
    SerialPort &port = mock_port.get();
    std::vector<uint8_t> vec_compare = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::chrono::nanoseconds timeout = 1ms;
    auto data = port.read(timeout);
    REQUIRE(data == vec_compare);
    fakeit::Verify(
        OverloadedMethod(
            mock_port, read,
            void(std::back_insert_iterator<std::vector<uint8_t>>,
                 const std::chrono::nanoseconds &)).Matching(
            [](auto a, auto b)
    {
        (void)a;
        return b == 1ms;
    })).Once();
}



TEST_CASE("SerialPort's 'read' method takes a back inserter and a timeout "
          "writes the received bytes to the back inserter.", "[SerialPort]")
{
    // This test ensures that one read method calls the other.
    using read_type = std::vector<uint8_t>(const std::chrono::nanoseconds &);
    SerialPort serial;
    fakeit::Mock<SerialPort> mock_port(serial);
    fakeit::When(
        OverloadedMethod(
            mock_port, read, read_type)).AlwaysDo([](auto a)
    {
        (void)a;
        std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        return vec;
    });
    SerialPort &port = mock_port.get();
    std::vector<uint8_t> vec_compare = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::chrono::nanoseconds timeout = 1ms;
    std::vector<uint8_t> data;
    port.read(std::back_inserter(data), timeout);
    REQUIRE(data == vec_compare);
    fakeit::Verify(
        OverloadedMethod(
            mock_port, read, read_type).Matching([](auto a)
    {
        return a == 1ms;
    })).Once();
}


TEST_CASE("SerialPort's 'write' method accepts a vector of bytes.",
          "[SerialPort]")
{
    // This test ensures that one write method calls the other.
    SerialPort serial;
    fakeit::Mock<SerialPort> mock_port(serial);
    fakeit::Fake(
        OverloadedMethod(
            mock_port, write,
            void(std::vector<uint8_t>::const_iterator,
                 std::vector<uint8_t>::const_iterator)));
    SerialPort &port = mock_port.get();
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    port.write(vec);
    fakeit::Verify(
        OverloadedMethod(
            mock_port, write,
            void(std::vector<uint8_t>::const_iterator,
                 std::vector<uint8_t>::const_iterator)).Matching(
            [&](auto a, auto b)
    {
        return a == vec.begin() && b == vec.end();
    })).Once();
}


TEST_CASE("SerialPort's 'write' method accepts two vector iterators",
          "[UDPSocket]")
{
    // This test ensures that one write method calls the other.
    SerialPort serial;
    fakeit::Mock<SerialPort> mock_port(serial);
    std::vector<uint8_t> write_vec;
    fakeit::When(
        OverloadedMethod(
            mock_port, write,
            void(const std::vector<uint8_t> &))).AlwaysDo(
                [&](auto a)
    {
        write_vec = a;
    });
    SerialPort &port = mock_port.get();
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    port.write(vec.begin(), vec.end());
    fakeit::Verify(
        OverloadedMethod(
            mock_port, write,
            void(const std::vector<uint8_t> &))).Once();
    REQUIRE(vec == write_vec);
}
