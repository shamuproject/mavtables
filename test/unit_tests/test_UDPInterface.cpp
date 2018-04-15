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
#include <memory>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include <catch.hpp>
#include <fakeit.hpp>

#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
#include "Filter.hpp"
#include "IPAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"
#include "UDPInterface.hpp"
#include "UDPSocket.hpp"
#include "util.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;


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
    // MAVLink packets.
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto mission_set_current =
        std::make_shared<packet_v2::Packet>(to_vector(MissionSetCurrentV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    // Connection pool
    ConnectionPool pool_obj;
    fakeit::Mock<ConnectionPool> spy_pool(pool_obj);
    fakeit::Spy(Method(spy_pool, add));
    auto pool = mock_shared(spy_pool);
    // Filter
    fakeit::Mock<Filter> mock_filter;
    std::multiset<packet_v2::Packet,
        bool(*)(const packet_v2::Packet &, const packet_v2::Packet &)>
        will_accept_packets([](const auto &a, const auto &b)
    {
        return a.data() < b.data();
    });
    std::multiset<MAVAddress> will_accept_addresses;
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto &a, auto &b)
    {
        will_accept_packets.insert(
            dynamic_cast<const packet_v2::Packet &>(a));
        will_accept_addresses.insert(b);
        return std::pair<bool, int>(true, 0);
    });
    auto filter = mock_shared(mock_filter);
    // Socket
    using receive_type =
        IPAddress(std::back_insert_iterator<std::vector<uint8_t>>,
                  const std::chrono::nanoseconds &);
    UDPSocket udp_socket;
    fakeit::Mock<UDPSocket> mock_socket(udp_socket);
    auto socket = mock_unique(mock_socket);
    // Interface
    UDPInterface udp(
        std::move(socket), pool,
        std::make_unique<ConnectionFactory<>>(filter));
    std::chrono::nanoseconds timeout = 250ms;
    SECTION("No packet received.")
    {
        // Mocks
        fakeit::When(
            OverloadedMethod(
                mock_socket, receive, receive_type)).AlwaysReturn(IPAddress(0));
        // Test
        udp.receive_packet(timeout);
        // Verification
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
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end() - 1, a);
            return IPAddress("127.0.0.1:4000");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(0);
        fakeit::Verify(Method(spy_pool, add)).Exactly(1);
    }
    SECTION("Full packet received.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(2);
        fakeit::Verify(Method(mock_filter, will_accept)).Once();
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 1);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.1")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Exactly(2);
    }
    SECTION("Multiple packets received (same IP and MAVLink addresses).")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(3);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(2);
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.1")) == 2);
        fakeit::Verify(Method(spy_pool, add)).Exactly(2);
    }
    SECTION("Multiple packets received (same IP, different MAVLink addresses).")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(MissionSetCurrentV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(3);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(2);
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 1);
        REQUIRE(will_accept_packets.count(*mission_set_current) == 1);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.1")) == 2);
        fakeit::Verify(Method(spy_pool, add)).Exactly(2);
    }
    SECTION("Multiple packets received (different IP and MAVLink addresses).")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(MissionSetCurrentV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4002");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(3);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 1);
        REQUIRE(will_accept_packets.count(*mission_set_current) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.1")) == 2);
        REQUIRE(will_accept_addresses.count(MAVAddress("224.255")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Exactly(3);
    }
    SECTION("Partial packets with same IP address should be combined and "
            "parsed.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end() - 10, a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.end() - 10, vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(3);
        fakeit::Verify(Method(mock_filter, will_accept)).Once();
        REQUIRE(will_accept_packets.count(*encapsulated_data) == 1);
        REQUIRE(will_accept_addresses.count(MAVAddress("127.1")) == 1);
        fakeit::Verify(Method(spy_pool, add)).Exactly(2);
    }
    SECTION("Partial packets with different IP addresses should be dropped.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end() - 10, a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.end() - 10, vec.end(), a);
            return IPAddress("127.0.0.1:4002");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        // Verification
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Exactly(3);
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(0);
        fakeit::Verify(Method(spy_pool, add)).Once();
    }
}


TEST_CASE("UDPInterace's 'send_packet' method.", "[UPDInterface]")
{
    // MAVLink packets.
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    // Filter
    fakeit::Mock<Filter> mock_filter;
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto & a, auto & b)
    {
        (void)a;
        (void)b;
        return std::pair<bool, int>(true, 0);
    });
    auto filter = mock_shared(mock_filter);
    // Socket
    std::multiset<std::vector<uint8_t>> send_bytes;
    std::multiset<IPAddress> send_addresses;
    using receive_type =
        IPAddress(std::back_insert_iterator<std::vector<uint8_t>>,
                  const std::chrono::nanoseconds &);
    using send_type =
        void(std::vector<uint8_t>::const_iterator,
             std::vector<uint8_t>::const_iterator,
             const IPAddress &);
    UDPSocket udp_socket;
    fakeit::Mock<UDPSocket> mock_socket(udp_socket);
    fakeit::When(OverloadedMethod(mock_socket, send, send_type)
                ).AlwaysDo([&](auto a, auto b, auto c)
    {
        std::vector<uint8_t> vec;
        std::copy(a, b, std::back_inserter(vec));
        send_bytes.insert(vec);
        send_addresses.insert(c);
    });
    auto socket = mock_unique(mock_socket);
    // Connection Factory
    ConnectionFactory<> factory_obj(filter);
    fakeit::Mock<ConnectionFactory<>> spy_factory(factory_obj);
    fakeit::Spy(Method(spy_factory, wait_for_packet));
    // Interface
    UDPInterface udp(
        std::move(socket),
        std::make_shared<ConnectionPool>(),
        mock_unique(spy_factory));
    std::chrono::nanoseconds timeout = 1ms;
    SECTION("No packets, timeout.")
    {
        // Test
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(0);
    }
    SECTION("Single connection, single packet.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(OverloadedMethod(mock_socket, send, send_type)).Once();
        REQUIRE(send_bytes.size() == 1);
        REQUIRE(send_bytes.count(to_vector(EncapsulatedDataV2())) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 1);
    }
    SECTION("Single connection, multiple packets.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(MissionSetCurrentV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(OverloadedMethod(mock_socket, send, send_type)).Once();
        REQUIRE(send_bytes.size() == 1);
        REQUIRE(send_bytes.count(to_vector(EncapsulatedDataV2())) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 1);
        // Test
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(spy_factory, wait_for_packet).Using(1ms)).Exactly(2);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(2);
        REQUIRE(send_bytes.size() == 2);
        REQUIRE(send_bytes.count(to_vector(EncapsulatedDataV2())) == 1);
        REQUIRE(send_bytes.count(to_vector(MissionSetCurrentV2())) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 2);
    }
    SECTION("Multiple connections, multiple packets.")
    {
        // Mocks
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(MissionSetCurrentV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4002");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Exactly(2);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(2);
        REQUIRE(send_bytes.size() == 2);
        REQUIRE(send_bytes.count(to_vector(EncapsulatedDataV2())) == 1);
        REQUIRE(send_bytes.count(to_vector(MissionSetCurrentV2())) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4001")) == 1);
        // Test
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(
            Method(spy_factory, wait_for_packet).Using(1ms)).Exactly(2);
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Exactly(3);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(3);
        REQUIRE(send_bytes.size() == 3);
        REQUIRE(send_bytes.count(to_vector(EncapsulatedDataV2())) == 1);
        REQUIRE(send_bytes.count(to_vector(MissionSetCurrentV2())) == 2);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 2);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4001")) == 1);
    }
    SECTION("Multiple connections with broadcast packet.")
    {
        // Mocks
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)b;

            if (a.name() == "MISSION_SET_CURRENT")
            {
                return std::pair<bool, int>(true, 0);
            }

            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into first connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            // Load 224.255 mavlink address into second connection.
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(MissionSetCurrentV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4002");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Exactly(2);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(2);
        REQUIRE(send_bytes.size() == 2);
        REQUIRE(send_bytes.count(to_vector(MissionSetCurrentV2())) == 2);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4001")) == 1);
        // Test
        udp.send_packet(timeout);
        // Verification (no futher operations)
        fakeit::Verify(
            Method(spy_factory, wait_for_packet).Using(1ms)).Exactly(2);
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Exactly(3);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Exactly(2);
    }
    SECTION("Multiple connections with targeted packet.")
    {
        // Mocks
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)b;

            if (a.name() == "PING")
            {
                return std::pair<bool, int>(true, 0);
            }

            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).Do([](auto a, auto b)
        {
            // Load 127.1 mavlink address into first connection.
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4000");
        }).Do([](auto a, auto b)
        {
            // Load 224.255 mavlink address into second connection.
            (void)b;
            auto vec = to_vector(EncapsulatedDataV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4001");
        }).Do([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(PingV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("127.0.0.1:4002");
        });
        // Test
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.receive_packet(timeout);
        udp.send_packet(timeout);
        // Verification
        fakeit::Verify(Method(spy_factory, wait_for_packet).Using(1ms)).Once();
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Once();
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Once();
        REQUIRE(send_bytes.size() == 1);
        REQUIRE(send_bytes.count(to_vector(PingV2())) == 1);
        REQUIRE(send_addresses.count(IPAddress("127.0.0.1:4000")) == 1);
        // Test
        udp.send_packet(timeout);
        // Verification (no futher operations)
        fakeit::Verify(
            Method(spy_factory, wait_for_packet).Using(1ms)).Exactly(2);
        fakeit::Verify(Method(spy_factory, wait_for_packet)).Exactly(2);
        fakeit::Verify(
            OverloadedMethod(mock_socket, send, send_type)).Once();
    }
}


TEST_CASE("UDPInterface's are printable.", "[UDPInterface]")
{
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<ConnectionFactory<>> mock_factory;
    auto pool = mock_shared(mock_pool);
    auto factory = mock_unique(mock_factory);
    auto socket = std::make_unique<UDPSocket>();
    UDPInterface udp(std::move(socket), pool, std::move(factory));
    REQUIRE(str(udp) == "unknown UDP socket");
}


// The tests below are from attempts to test the UDPInterface class in different
// ways, all failing to provide a method to test the entire class.  They have
// been kept because they could theoretically find problems not covered by the
// tests above and do not cause much overhead.
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("OLD TEST: UDPInterface's tests.", "[UPDInterface]")
{
    using receive_type = IPAddress(
                             std::back_insert_iterator<std::vector<uint8_t>>,
                             const std::chrono::nanoseconds &);
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    fakeit::Mock<Filter> mock_filter;
    UDPSocket udp_socket;
    fakeit::Mock<UDPSocket> mock_socket(udp_socket);
    auto filter = mock_shared(mock_filter);
    auto socket = mock_unique(mock_socket);
    auto pool = std::make_shared<ConnectionPool>();
    SECTION("Timeout")
    {
        fakeit::When(OverloadedMethod(
                         mock_socket, receive, receive_type)).AlwaysReturn(
                             IPAddress(0));
        UDPInterface udp(
            std::move(socket), pool,
            std::make_unique<ConnectionFactory<>>(filter));
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [&](auto a, auto b)
        {
            (void)a;
            return b == 250ms;
        })).Once();
    }
}


TEST_CASE("OLD TEST: UDPInterface's 'receive_packet' method receives one or "
          "more MAVLink packets.", "[UPDInterface]")
{
    using receive_type = IPAddress(
                             std::back_insert_iterator<std::vector<uint8_t>>,
                             const std::chrono::nanoseconds &);
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    fakeit::Mock<Filter> mock_filter;
    UDPSocket udp_socket;
    fakeit::Mock<UDPSocket> mock_socket(udp_socket);
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<ConnectionFactory<>> mock_factory;
    auto filter = mock_shared(mock_filter);
    auto socket = mock_unique(mock_socket);
    auto pool = mock_shared(mock_pool);
    auto factory = mock_unique(mock_factory);
    SECTION("When no packets available for sending.")
    {
        fakeit::When(OverloadedMethod(mock_socket, receive, receive_type)
                    ).AlwaysDo([](auto a, auto b)
        {
            (void)b;
            auto vec = to_vector(HeartbeatV2());
            std::copy(vec.begin(), vec.end(), a);
            return IPAddress("192.168.0.0");
        });
        fakeit::Fake(Method(mock_pool, send));
        fakeit::Fake(Method(mock_pool, add));
        fakeit::When(Method(mock_factory, get)).AlwaysDo([&]()
        {
            return std::make_unique<Connection>(filter);
        });
        UDPInterface udp(std::move(socket), pool, std::move(factory));
        std::chrono::nanoseconds timeout = 250ms;
        udp.receive_packet(timeout);
        fakeit::Verify(
            OverloadedMethod(mock_socket, receive, receive_type).Matching(
                [](auto a, auto c)
        {
            (void)a;
            return c == 250ms;
        })).Once();
        fakeit::Verify(Method(mock_pool, send).Matching(
                           [&](auto & a)
        {
            return a != nullptr && *a == *heartbeat;
        })).Once();
    }
}


TEST_CASE("OLD TEST: UDPInterface's 'send_packet' method sends one or more "
          " MAVLink packets.", "[UPDInterface]")
{
    UDPSocket udp_socket;
    fakeit::Mock<UDPSocket> mock_socket(udp_socket);
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<ConnectionFactory<>> mock_factory;
    auto socket = mock_unique(mock_socket);
    auto pool = mock_shared(mock_pool);
    auto factory = mock_unique(mock_factory);
    SECTION("When no packets available for sending.")
    {
        fakeit::When(Method(mock_factory, wait_for_packet)).AlwaysReturn(false);
        UDPInterface udp(std::move(socket), pool, std::move(factory));
        std::chrono::nanoseconds timeout = 250ms;
        udp.send_packet(timeout);
        fakeit::Verify(
            Method(mock_factory, wait_for_packet).Using(250ms)).Once();
    }
}
