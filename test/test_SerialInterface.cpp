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
#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>

#include <catch.hpp>
#include <fakeit.hpp>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"
#include "SerialInterface.hpp"
#include "SerialPort.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;


TEST_CASE("SerialInterface's can be constructed.", "[SerialInterface]")
{
    fakeit::Mock<SerialPort> mock_port;
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<Connection> mock_connection;
    fakeit::Fake(Method(mock_pool, add));
    auto port = mock_unique(mock_port);
    auto pool = mock_shared(mock_pool);
    auto connection = mock_unique(mock_connection);
    SECTION("When all inputs are valid, registers connection with pool.")
    {
        Connection *conn = nullptr;
        fakeit::When(Method(mock_pool, add)).AlwaysDo([&](auto a)
        {
            conn = a.lock().get();
        });
        REQUIRE_NOTHROW(
            SerialInterface(std::move(port), pool, std::move(connection)));
        fakeit::Verify(Method(mock_pool, add)).Once();
        REQUIRE(conn == &mock_connection.get());
    }
    SECTION("Ensures the serial port pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            SerialInterface(nullptr, pool, std::move(connection)),
            std::invalid_argument);
        connection = mock_unique(mock_connection);
        REQUIRE_THROWS_WITH(
            SerialInterface(nullptr, pool, std::move(connection)),
            "Given serial port pointer is null.");
    }
    SECTION("Ensures the connection pool pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            SerialInterface(std::move(port), nullptr, std::move(connection)),
            std::invalid_argument);
        port = mock_unique(mock_port);
        connection = mock_unique(mock_connection);
        REQUIRE_THROWS_WITH(
            SerialInterface(std::move(port), nullptr, std::move(connection)),
            "Given connection pool pointer is null.");
    }
    SECTION("Ensures the connection pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            SerialInterface(std::move(port), pool, nullptr),
            std::invalid_argument);
        port = mock_unique(mock_port);
        REQUIRE_THROWS_WITH(
            SerialInterface(std::move(port), pool, nullptr),
            "Given connection pointer is null.");
    }
}


TEST_CASE("SerialInterace's 'receive_packet' method.", "[SerialInterface]")
{
    // MAVLink packets.
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    // Serial port
    fakeit::Mock<SerialPort> mock_port;
    auto port = mock_unique(mock_port);
    using read_type =
        void(std::back_insert_iterator<std::vector<uint8_t>>,
                  const std::chrono::nanoseconds &);
    // Pool
    fakeit::Mock<ConnectionPool> mock_pool;
    auto pool = mock_shared(mock_pool);
    fakeit::Fake(Method(mock_pool, add));
    std::multiset<packet_v2::Packet,
        bool(*)(packet_v2::Packet, packet_v2::Packet)>
        send_packets([](auto a, auto b)
    {
        return a.data() < b.data();
    });
    fakeit::When(Method(mock_pool, send)).AlwaysDo([&](auto &a)
    {
        // std::make_shared<packet_v2::Packet>*a
        const Packet *packet = a.get();
        send_packets.insert(packet_v2::Packet(
                    *dynamic_cast<const packet_v2::Packet *>(packet)));
    });
    // Connection
    fakeit::Mock<Connection> mock_connection;
    auto connection = mock_unique(mock_connection);
    fakeit::Fake(Method(mock_connection, add_address));
    // Interface
    SerialInterface serial(std::move(port), pool, std::move(connection));
    std::chrono::nanoseconds timeout = 250ms;
    SECTION("No packet received.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).AlwaysDo([](auto a, auto b)
        {
            (void)a;
            (void)b;
        });
        // Test
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_connection, add_address)).Exactly(0);
        fakeit::Verify(Method(mock_pool, send)).Exactly(0);
    }
    SECTION("Partial packet received.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end() - 1, a);
        });
        // Test
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_connection, add_address)).Exactly(0);
        fakeit::Verify(Method(mock_pool, send)).Exactly(0);
    }
    SECTION("Full packet received.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
        });
        // Test
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_connection, add_address
                    ).Using(MAVAddress("127.1"))).Exactly(1);
        fakeit::Verify(Method(mock_pool, send)).Exactly(1);
        REQUIRE(send_packets.count(*heartbeat) == 1);
    }
    SECTION("Multiple packets received (same MAVLink address).")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
        });
        // Test
        serial.receive_packet(timeout);
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_connection, add_address
                    ).Using(MAVAddress("127.1"))).Exactly(2);
        fakeit::Verify(Method(mock_pool, send)).Exactly(2);
        REQUIRE(send_packets.count(*heartbeat) == 2);
    }
    SECTION("Multiple packets received (different MAVLink address).")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
        });
        // Test
        serial.receive_packet(timeout);
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_connection, add_address
                    ).Using(MAVAddress("127.1"))).Exactly(1);
        fakeit::Verify(Method(mock_connection, add_address
                    ).Using(MAVAddress("224.255"))).Exactly(1);
        fakeit::Verify(Method(mock_pool, send)).Exactly(2);
        REQUIRE(send_packets.count(*heartbeat) == 1);
        REQUIRE(send_packets.count(*encapsulated_data) == 1);
    }
    SECTION("Partial packets should be combined and parsed.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_port, read, read_type)
                ).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end() - 10, a);
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.end() - 10, vec.end(), a);
        });
        // Test
        serial.receive_packet(timeout);
        serial.receive_packet(timeout);
        // Verification
        fakeit::Verify(OverloadedMethod(mock_port, read, read_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_connection, add_address
                    ).Using(MAVAddress("224.255"))).Exactly(1);
        fakeit::Verify(Method(mock_pool, send)).Exactly(1);
        REQUIRE(send_packets.count(*encapsulated_data) == 1);
    }
}


TEST_CASE("SerialInterace's 'send_packet' method.", "[UPDInterface]")
{
    // MAVLink packets.
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    // Serial port
    fakeit::Mock<SerialPort> mock_port;
    auto port = mock_unique(mock_port);
    using write_type =
        void(std::vector<uint8_t>::const_iterator first,
             std::vector<uint8_t>::const_iterator last);
    std::multiset<std::vector<uint8_t>> write_bytes;
    fakeit::When(OverloadedMethod(mock_port, write, write_type)).AlwaysDo(
            [&](auto a, auto b)
    {
        std::vector<uint8_t> vec;
        std::copy(a, b, std::back_inserter(vec));
        write_bytes.insert(vec);
    });
    // Pool
    fakeit::Mock<ConnectionPool> mock_pool;
    auto pool = mock_shared(mock_pool);
    fakeit::Fake(Method(mock_pool, add));
    // Connection
    fakeit::Mock<Connection> mock_connection;
    auto connection = mock_unique(mock_connection);
    // Interface
    SerialInterface serial(std::move(port), pool, std::move(connection));
    std::chrono::nanoseconds timeout = 250ms;
    SECTION("No packets, timeout.")
    {
        // Mocks
        fakeit::When(Method(mock_connection, next_packet)).Return(nullptr);
        // Test
        serial.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(mock_connection, next_packet).Using(250ms)).Once();
        fakeit::Verify(
            OverloadedMethod(mock_port, write, write_type)).Exactly(0);
    }
    SECTION("Single packet.")
    {
        // Mocks
        fakeit::When(Method(mock_connection, next_packet)).Return(heartbeat);
        // Test
        serial.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(mock_connection, next_packet).Using(250ms)).Once();
        fakeit::Verify(
            OverloadedMethod(mock_port, write, write_type)).Exactly(1);
        REQUIRE(write_bytes.count(heartbeat->data()) == 1);
    }
    SECTION("Multiple packets.")
    {
        // Mocks
        fakeit::When(Method(mock_connection, next_packet)
                ).Return(heartbeat).Return(encapsulated_data);
        // Test
        serial.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(mock_connection, next_packet).Using(250ms)).Once();
        fakeit::Verify(
            OverloadedMethod(mock_port, write, write_type)).Exactly(1);
        REQUIRE(write_bytes.count(heartbeat->data()) == 1);
        // Test
        serial.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(mock_connection, next_packet).Using(250ms)).Exactly(2);
        fakeit::Verify(
            OverloadedMethod(mock_port, write, write_type)).Exactly(2);
        REQUIRE(write_bytes.count(heartbeat->data()) == 1);
        REQUIRE(write_bytes.count(encapsulated_data->data()) == 1);
    }
}
