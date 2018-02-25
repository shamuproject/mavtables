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
#include <memory>
#include <stdexcept>
#include <vector>
#include <utility>
#include <set>

#include <catch.hpp>

#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
#include "UDPInterface.hpp"
#include "UDPSocket.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "IPAddress.hpp"
#include "Filter.hpp"
#include "util.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;

#include <iostream>



TEST_CASE("UDPInterface's can be constructed.", "[UPDInterface]")
{
    fakeit::Mock<UDPSocket> mock_socket;
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<ConnectionFactory<>> mock_factory;
    auto socket = mock_unique(mock_socket);
    auto pool = mock_shared(mock_pool);
    auto factory = mock_unique(mock_factory);
    SECTION("When all inputs are valid.")
    {
        REQUIRE_NOTHROW(
            UDPInterface(std::move(socket), pool, std::move(factory)));
    }
    SECTION("Ensures the socket pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(nullptr, pool, std::move(factory)),
            std::invalid_argument);
        factory = mock_unique(mock_factory);
        REQUIRE_THROWS_WITH(
            UDPInterface(nullptr, pool, std::move(factory)),
            "Given socket pointer is null.");
    }
    SECTION("Ensures the connection pool pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(std::move(socket), nullptr, std::move(factory)),
            std::invalid_argument);
        socket = mock_unique(mock_socket);
        factory = mock_unique(mock_factory);
        REQUIRE_THROWS_WITH(
            UDPInterface(std::move(socket), nullptr, std::move(factory)),
            "Given connection pool pointer is null.");
    }
    SECTION("Ensures the connection factory pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(std::move(socket), pool, nullptr),
            std::invalid_argument);
        socket = mock_unique(mock_socket);
        REQUIRE_THROWS_WITH(
            UDPInterface(std::move(socket), pool, nullptr),
            "Given connection factory pointer is null.");
    }
}


TEST_CASE("UDPInterace's 'receive_packet' method.", "[UPDInterface]")
{
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    ConnectionPool pool_obj;
    fakeit::Mock<ConnectionPool> spy_pool(pool_obj);
    fakeit::Spy(Method(spy_pool, add));
    auto pool = mock_shared(spy_pool);
    // auto pool = std::make_shared<ConnectionPool>();
    // Vectors for capturing reference arguments.
    std::multiset<packet_v2::Packet,
        bool(*)(packet_v2::Packet, packet_v2::Packet)>
        will_accept_packets([](auto a, auto b)
    {
        return a.data() < b.data();
    });
    std::multiset<MAVAddress> will_accept_addresses;
    using receive_type =
        IPAddress(std::back_insert_iterator<std::vector<uint8_t>>,
                  const std::chrono::nanoseconds &);
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<UDPSocket> mock_socket;
    auto filter = mock_shared(mock_filter);
    auto socket = mock_unique(mock_socket);
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto & a, auto & b)
    {
        std::cout << "will_accept(" << a << ", " << b << ")" << std::endl;
        will_accept_packets.insert(
            dynamic_cast<const packet_v2::Packet &>(a));
        will_accept_addresses.insert(b);
        return std::pair<bool, int>(true, 2);
    });
    UDPInterface udp(
        std::move(socket), pool,
        std::make_unique<ConnectionFactory<>>(filter));
    SECTION("No packet received.")
    {
        fakeit::When(
            OverloadedMethod(
                mock_socket, receive, receive_type)).AlwaysReturn(IPAddress(0));
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(0);
        fakeit::Verify(Method(spy_pool, add)).Exactly(0);
    }
    SECTION("Partial packet received.")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end() - 1, a);
            return IPAddress("127.0.0.1:4000");
        });
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(0);
        fakeit::Verify(Method(spy_pool, add)).Exactly(0);
    }
    SECTION("Full packet received.")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        });
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_filter, will_accept)).Once();
        REQUIRE(will_accept_packets.count(*heartbeat) == 1);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.0")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Once();
    }
    SECTION("Multiple packets received (same IP and MAVLink addresses).")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        });
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(2);
        REQUIRE(will_accept_packets.count(*heartbeat) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.0")) == 2);
        fakeit::Verify(Method(spy_pool, add)).Once();
    }
    SECTION("Multiple packets received (same IP, different MAVLink addresses).")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        });
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        REQUIRE(will_accept_packets.count(*heartbeat) == 1);
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.0")) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("224.255")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Once();
    }
    SECTION("Multiple packets received (different IP and MAVLink addresses).")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 1);
        REQUIRE(will_accept_packets.count(*heartbeat) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("224.255")) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.0")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Exactly(2);
    }
}

// Check receive_packet via filter mock
// No packet
// Single packet.
// Multiple packets same MAVLink address and same IP address.
// Multiple packets different MAVLink addresses, but same IP address.
// Multiple packets different MAVLink addresses and different IP address.
// Partial packets with same IP address should be combined and parsed.
// Partial packets across IP addresses should be dropped.
//
// Then check send_packet by using all the same setups, but validating the
// socket mock.




// TEST_CASE("UDPInterface's tests.", "[UPDInterface]")
// {
//     using receive_type = IPAddress(
//                              std::back_insert_iterator<std::vector<uint8_t>>,
//                              const std::chrono::nanoseconds &);
//     auto heartbeat =
//         std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
//     fakeit::Mock<Filter> mock_filter;
//     fakeit::Mock<UDPSocket> mock_socket;
//     auto filter = mock_shared(mock_filter);
//     auto socket = mock_unique(mock_socket);
//     auto pool = std::make_shared<ConnectionPool>();
//     SECTION("Timeout")
//     {
//         fakeit::When(OverloadedMethod(
//                          mock_socket, receive, receive_type)).AlwaysReturn(
//                              IPAddress(0));
//         UDPInterface udp(
//             std::move(socket), pool,
//             std::make_unique<ConnectionFactory<>>(filter));
//         std::chrono::nanoseconds delay = 250ms;
//         udp.receive_packet(250ms);
//         fakeit::Verify(
//             OverloadedMethod(mock_socket, receive, receive_type).Matching(
//                 [&](auto a, auto b)
//         {
//             (void)a;
//             return b == 250ms;
//         })).Once();
//     }
//     // SECTION("Stuff")
//     // {
//     //     fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
//     //                 ).Do([](auto a, auto b)
//     //     {
//     //         (void)b;
//     //         auto vec = to_vector(HeartbeatV2());
//     //         std::copy(vec.begin(), vec.end(), a);
//     //         return IPAddress("127.0.0.1:4000");
//     //     }).Do([](auto a, auto b)
//     //     {
//     //         (void)b;
//     //         auto vec = to_vector(HeartbeatV2());
//     //         std::copy(vec.begin(), vec.end(), a);
//     //         return IPAddress("127.0.0.1:4001");
//     //     });
//     //     fakeit::When(Method(mock_filter, will_accept)).AlwaysReturn(
//     //             std::pair<bool, int>(true, 2));
//     //     UDPInterface udp(
//     //         std::move(socket), pool,
//     //         std::make_unique<ConnectionFactory<>>(filter));
//     //     udp.receive_packet(250ms);
//     //     udp.receive_packet(250ms);
//     // }
//     // Check receive_packet via filter mock
//     // No packet
//     // Single packet.
//     // Multiple packets same MAVLink address and same IP address.
//     // Multiple packets different MAVLink addresses, but same IP address.
//     // Multiple packets different MAVLink addresses and different IP address.
//     // Partial packets with same IP address should be combined and parsed.
//     // Partial packets across IP addresses should be dropped.
//     //
//     // Then check send_packet by using all the same setups, but validating the
//     // socket mock.
// }
//
//
//
//
//
//
// TEST_CASE("UDPInterface's 'receive_packet' method receives one or more MAVLink "
//           "packets.", "[UPDInterface]")
// {
//     using receive_type = IPAddress(
//                              std::back_insert_iterator<std::vector<uint8_t>>,
//                              const std::chrono::nanoseconds &);
//     auto heartbeat =
//         std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
//     fakeit::Mock<Filter> mock_filter;
//     fakeit::Mock<UDPSocket> mock_socket;
//     fakeit::Mock<ConnectionPool> mock_pool;
//     fakeit::Mock<ConnectionFactory<>> mock_factory;
//     auto filter = mock_shared(mock_filter);
//     auto socket = mock_unique(mock_socket);
//     auto pool = mock_shared(mock_pool);
//     auto factory = mock_unique(mock_factory);
//     SECTION("When no packets available for sending.")
//     {
//         fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
//                     ).AlwaysDo([](auto a, auto b)
//         {
//             (void)b;
//             auto vec = to_vector(HeartbeatV2());
//             std::copy(vec.begin(), vec.end(), a);
//             return IPAddress("192.168.0.0");
//         });
//         fakeit::Fake(Method(mock_pool, send));
//         fakeit::Fake(Method(mock_pool, add));
//         fakeit::When(Method(mock_factory, get)).AlwaysDo([&]()
//         {
//             return std::make_unique<Connection>(filter);
//         });
//         UDPInterface udp(std::move(socket), pool, std::move(factory));
//         std::chrono::nanoseconds timeout = 250ms;
//         udp.receive_packet(timeout);
//         fakeit::Verify(
//             OverloadedMethod(mock_socket, receive, receive_type).Matching(
//                 [](auto a, auto c)
//         {
//             (void)a;
//             return c == 250ms;
//         })).Once();
//         fakeit::Verify(Method(mock_pool, send).Matching(
//                            [&](auto & a)
//         {
//             return a != nullptr && *a == *heartbeat;
//         })).Once();
//     }
// }
//
//
//
//
// TEST_CASE("UDPInterface's 'send_packet' method sends one or more MAVLink "
//           "packets.", "[UPDInterface]")
// {
//     fakeit::Mock<UDPSocket> mock_socket;
//     fakeit::Mock<ConnectionPool> mock_pool;
//     fakeit::Mock<ConnectionFactory<>> mock_factory;
//     auto socket = mock_unique(mock_socket);
//     auto pool = mock_shared(mock_pool);
//     auto factory = mock_unique(mock_factory);
//     SECTION("When no packets available for sending.")
//     {
//         fakeit::When(Method(mock_factory, wait_for_packet)).AlwaysReturn(false);
//         UDPInterface udp(std::move(socket), pool, std::move(factory));
//         std::chrono::nanoseconds timeout = 250ms;
//         udp.send_packet(timeout);
//         fakeit::Verify(
//             Method(mock_factory, wait_for_packet).Using(250ms)).Once();
//     }
//     // SECTION("When no packets available for sending.")
//     // {
//     //     fakeit::When(Method(mock_factory, wait_for_packet)).AlwaysReturn(false);
//     //     UDPInterface udp(std::move(socket), pool, std::move(factory));
//     //     std::chrono::nanoseconds timeout = 250ms;
//     //     udp.send_packet(timeout);
//     //     fakeit::Verify(
//     //         Method(mock_factory, wait_for_packet).Using(250ms)).Once();
//     // }
// }
