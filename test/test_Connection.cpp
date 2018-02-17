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


#include <memory>
#include <stdexcept>
#include <vector>
#include <utility>

#include <catch.hpp>
#include <fakeit.hpp>

#include "AddressPool.hpp"
#include "Connection.hpp"
#include "MAVAddress.hpp"
#include "Filter.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"
#include "PacketQueue.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;


// NOTE: This testing file is not the best at not testing the implementation.
//       If a better solution is found it should be rewritten.


TEST_CASE("Connection's can be constructed.", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    SECTION("As a regular connection.")
    {
        REQUIRE_NOTHROW(Connection(filter, std::move(pool), std::move(queue)));
    }
    SECTION("As a mirror connection.")
    {
        REQUIRE_NOTHROW(
            Connection(filter, std::move(pool), std::move(queue), true));
    }
    SECTION("Ensures the filter pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection(nullptr, std::move(pool), std::move(queue)),
            std::invalid_argument);
        pool = mock_unique(mock_pool);
        queue = mock_unique(mock_queue);
        REQUIRE_THROWS_WITH(
            Connection(nullptr, std::move(pool), std::move(queue)),
            "Given filter pointer is null.");
    }
    SECTION("Ensures the pool pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection(filter, nullptr, std::move(queue)),
            std::invalid_argument);
        queue = mock_unique(mock_queue);
        REQUIRE_THROWS_WITH(
            Connection(filter, nullptr, std::move(queue)),
            "Given pool pointer is null.");
    }
    SECTION("Ensures the queue pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection(filter, std::move(pool), nullptr),
            std::invalid_argument);
        pool = mock_unique(mock_pool);
        REQUIRE_THROWS_WITH(
            Connection(filter, std::move(pool), nullptr),
            "Given queue pointer is null.");
    }
}


TEST_CASE("Connection's 'add_address' method adds/updates addresses.", 
          "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_pool, add));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn(filter, std::move(pool), std::move(queue));
    conn.add_address(MAVAddress("192.168"));
    fakeit::Verify(Method(mock_pool, add).Matching([](auto a)
    {
        return a == MAVAddress("192.168");
    })).Once();
}


TEST_CASE("Connection's 'next_packet' method.", "[Connection]")
{
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn(filter, std::move(pool), std::move(queue));
    SECTION("Returns the next packet.")
    {
        fakeit::When(Method(mock_queue, pop)).Return(ping);
        auto packet = conn.next_packet();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *ping);
        fakeit::Verify(Method(mock_queue, pop).Matching([](auto a)
        {
            return a == true; // blocking
        })).Once();
    }
    SECTION("Returns nullptr if the connection is shutdown.")
    {
        fakeit::When(Method(mock_queue, pop)).Return(nullptr);
        auto packet = conn.next_packet();
        REQUIRE(packet == nullptr);
        fakeit::Verify(Method(mock_queue, pop).Matching([](auto a)
        {
            return a == true; // blocking
        })).Once();
    }
}


TEST_CASE("Connection's 'send' method ensures the given packet is not "
          "nullptr.", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn(filter, std::move(pool), std::move(queue));
    SECTION("Ensures the given packet is not nullptr.")
    {
        REQUIRE_THROWS_AS(conn.send(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            conn.send(nullptr), "Given packet pointer is null.");
    }
}


TEST_CASE("Connection's 'send' method (with destination address).", 
          "[Connection]")
{
    // Vectors for capturing reference arguments.
    std::vector<packet_v2::Packet> will_accept_packets;
    std::vector<MAVAddress> will_accept_addresses;
    std::vector<MAVAddress> contains_addresses;
    // Packets for testing.
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn(filter, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the destination can be "
            "reached on this connection.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            return std::pair<bool, int>(true, 2);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto &a){
            contains_addresses.push_back(a);
            return true;
        });
        conn.send(ping);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 1);
        REQUIRE(will_accept_packets[0] == *ping);
        REQUIRE(will_accept_addresses.size() == 1);
        REQUIRE(will_accept_addresses[0] == MAVAddress("255.64"));
        REQUIRE(contains_addresses.size() == 1);
        REQUIRE(contains_addresses[0] == MAVAddress("255.64"));
        fakeit::Verify(Method(mock_filter, will_accept)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *ping && b == 2;
        })).Once();
        fakeit::Verify(Method(mock_pool, contains)).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto &a){
            contains_addresses.push_back(a);
            return true;
        });
        conn.send(ping);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 1);
        REQUIRE(will_accept_packets[0] == *ping);
        REQUIRE(will_accept_addresses.size() == 1);
        REQUIRE(will_accept_addresses[0] == MAVAddress("255.64"));
        REQUIRE(contains_addresses.size() == 1);
        REQUIRE(contains_addresses[0] == MAVAddress("255.64"));
        fakeit::Verify(Method(mock_filter, will_accept)).Once();
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
        fakeit::Verify(Method(mock_pool, contains)).Once();
    }
    SECTION("Silently drops the packet if the destination cannot be "
            "reached on this connection.")
    {
        fakeit::Fake(Method(mock_filter, will_accept));
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto &a){
            contains_addresses.push_back(a);
            return false;
        });
        conn.send(ping);
        REQUIRE(contains_addresses.size() == 1);
        REQUIRE(contains_addresses[0] == MAVAddress("255.64"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(0);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
        fakeit::Verify(Method(mock_pool, contains)).Once();
    }
}


TEST_CASE("Connection's 'send' method (without destination address).",
          "[Connection]")
{
    // Vectors for capturing reference arguments.
    std::vector<packet_v2::Packet> will_accept_packets;
    std::vector<MAVAddress> will_accept_addresses;
    // Packets for testing.
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn(filter, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(increasing priority).")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            if (b == MAVAddress("192.168"))
            {
                return std::pair<bool, int>(true, 2);
            }
            if (b == MAVAddress("172.16"))
            {
                return std::pair<bool, int>(true, -3);
            }
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("10.10"),
                MAVAddress("172.16"),
                MAVAddress("192.168")
            };
            return addr;
        });
        conn.send(heartbeat);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *heartbeat);
        REQUIRE(will_accept_packets[1] == *heartbeat);
        REQUIRE(will_accept_packets[2] == *heartbeat);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("10.10"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("192.168"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *heartbeat && b == 2;
        })).Once();
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(decreasing priority).")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            if (b == MAVAddress("192.168"))
            {
                return std::pair<bool, int>(true, 2);
            }
            if (b == MAVAddress("172.16"))
            {
                return std::pair<bool, int>(true, -3);
            }
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(heartbeat);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *heartbeat);
        REQUIRE(will_accept_packets[1] == *heartbeat);
        REQUIRE(will_accept_packets[2] == *heartbeat);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("192.168"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("10.10"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *heartbeat && b == 2;
        })).Once();
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it for all "
            "reachable addresses.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(heartbeat);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *heartbeat);
        REQUIRE(will_accept_packets[1] == *heartbeat);
        REQUIRE(will_accept_packets[2] == *heartbeat);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("192.168"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("10.10"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
}


TEST_CASE("Connection's 'send' method (with broadcast address 0.0).",
          "[Connection]")
{
    // Vectors for capturing reference arguments.
    std::vector<packet_v2::Packet> will_accept_packets;
    std::vector<MAVAddress> will_accept_addresses;
    // Packets for testing.
    auto mission_set_current =
        std::make_shared<packet_v2::Packet>(to_vector(MissionSetCurrentV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn(filter, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(increasing priority).")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            if (b == MAVAddress("192.168"))
            {
                return std::pair<bool, int>(true, 2);
            }
            if (b == MAVAddress("172.16"))
            {
                return std::pair<bool, int>(true, -3);
            }
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("10.10"),
                MAVAddress("172.16"),
                MAVAddress("192.168")
            };
            return addr;
        });
        conn.send(mission_set_current);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *mission_set_current);
        REQUIRE(will_accept_packets[1] == *mission_set_current);
        REQUIRE(will_accept_packets[2] == *mission_set_current);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("10.10"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("192.168"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *mission_set_current && b == 2;
        })).Once();
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(decreasing priority).")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            if (b == MAVAddress("192.168"))
            {
                return std::pair<bool, int>(true, 2);
            }
            if (b == MAVAddress("172.16"))
            {
                return std::pair<bool, int>(true, -3);
            }
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(mission_set_current);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *mission_set_current);
        REQUIRE(will_accept_packets[1] == *mission_set_current);
        REQUIRE(will_accept_packets[2] == *mission_set_current);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("192.168"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("10.10"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *mission_set_current && b == 2;
        })).Once();
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it for all "
            "reachable addresses.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
                [&](auto &a, auto &b){
            will_accept_packets.push_back(
                    dynamic_cast<const packet_v2::Packet&>(a));
            will_accept_addresses.push_back(b);
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([](){
            std::vector<MAVAddress> addr= {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(mission_set_current);
        // Get around const reference problem.
        // https://github.com/eranpeer/FakeIt/issues/31#issuecomment-130676043
        REQUIRE(will_accept_packets.size() == 3);
        REQUIRE(will_accept_packets[0] == *mission_set_current);
        REQUIRE(will_accept_packets[1] == *mission_set_current);
        REQUIRE(will_accept_packets[2] == *mission_set_current);
        REQUIRE(will_accept_addresses.size() == 3);
        REQUIRE(will_accept_addresses[0] == MAVAddress("192.168"));
        REQUIRE(will_accept_addresses[1] == MAVAddress("172.16"));
        REQUIRE(will_accept_addresses[2] == MAVAddress("10.10"));
        fakeit::Verify(Method(mock_filter, will_accept)).Exactly(3);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
        fakeit::Verify(Method(mock_pool, addresses)).Once();
    }
}


TEST_CASE("Connection's 'shutdown' method shutdown the contained threadsafe "
          " PacketQueue.", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_pool, add));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    fakeit::Fake(Method(mock_queue, shutdown));
    Connection conn(filter, std::move(pool), std::move(queue));
    conn.shutdown();
    fakeit::Verify(Method(mock_queue, shutdown)).Once();
}
