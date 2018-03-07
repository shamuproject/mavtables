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
#include <future>
#include <memory>
#include <stdexcept>

#include <catch.hpp>

#include "ConnectionFactory.hpp"
#include "Filter.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;


TEST_CASE("ConnectionFactory's can be constructed.", "[ConnectionFactory]")
{
    fakeit::Mock<Filter> mock_filter;
    auto filter = mock_shared(mock_filter);
    REQUIRE_NOTHROW(ConnectionFactory<>(filter));
    REQUIRE_NOTHROW(ConnectionFactory<>(filter, true));
    REQUIRE_NOTHROW(ConnectionFactory<>(filter, false));
    SECTION("And ensures the given filter is not null.")
    {
        REQUIRE_THROWS_AS(ConnectionFactory<>(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            ConnectionFactory<>(nullptr), "Given filter pointer is null.");
    }
}


TEST_CASE(
    "ConnectionFactory's 'get' method returns a new connection.",
    "[ConnectionFactory]")
{
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    fakeit::Mock<Filter> mock_filter;
    fakeit::When(Method(mock_filter, will_accept))
        .AlwaysDo([](auto &a, auto &b) {
            (void)a;
            (void)b;
            return std::pair<bool, int>(true, 0);
        });
    auto filter = mock_shared(mock_filter);
    REQUIRE(filter != nullptr);
    ConnectionFactory<> connection_factory(filter);
    std::unique_ptr<Connection> conn = connection_factory.get();
    conn->add_address(MAVAddress("192.168"));
    conn->send(heartbeat);
    auto packet = conn->next_packet(0s);
    REQUIRE(packet != nullptr);
    REQUIRE(*packet == *heartbeat);
}


TEST_CASE(
    "ConnectionFactory's 'wait_for_packet' method waits for a packet "
    "on any of the connections created by the factory.",
    "[ConnectionFactory]")
{
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    fakeit::Mock<Filter> mock_filter;
    fakeit::When(Method(mock_filter, will_accept))
        .AlwaysDo([](auto &a, auto &b) {
            (void)a;
            (void)b;
            return std::pair<bool, int>(true, 0);
        });
    auto filter = mock_shared(mock_filter);
    REQUIRE(filter != nullptr);
    ConnectionFactory<> connection_factory(filter);
    std::unique_ptr<Connection> conn1 = connection_factory.get();
    std::unique_ptr<Connection> conn2 = connection_factory.get();
    conn1->add_address(MAVAddress("192.168"));
    conn2->add_address(MAVAddress("192.168"));
    SECTION("First connection.")
    {
        auto future = std::async(std::launch::async, [&]() {
            return connection_factory.wait_for_packet(10s);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        conn1->send(heartbeat);
        REQUIRE(future.wait_for(1ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Second connection.")
    {
        auto future = std::async(std::launch::async, [&]() {
            return connection_factory.wait_for_packet(10s);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        conn2->send(heartbeat);
        REQUIRE(future.wait_for(1ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Returns false on timeout.")
    {
        auto future = std::async(std::launch::async, [&]() {
            return connection_factory.wait_for_packet(1ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE_FALSE(future.get());
    }
}
