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
#include <thread>

#include <catch.hpp>

#include <Packet.hpp>
#include <PacketQueue.hpp>
#include "PacketVersion2.hpp"

#include "common_Packet.hpp"


using namespace std::chrono_literals;


TEST_CASE("PacketQueue's can be constructed.", "[AddressPool]")
{
    REQUIRE_NOTHROW(PacketQueue());
}


TEST_CASE("PacketQueue's 'push' method ensures that the packet pointer is "
          "not null.", "[PacketQueue]")
{
    PacketQueue queue;
    REQUIRE_THROWS_AS(queue.push(nullptr), std::invalid_argument);
    REQUIRE_THROWS_WITH(queue.push(nullptr), "Given packet pointer is null.");
}


TEST_CASE("PacketQueue's can be managed with 'push' and 'pop' methods.", 
          "[PacketQueue]")
{
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    auto ping =
        std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto set_mode =
        std::make_shared<packet_v2::Packet>(to_vector(SetModeV2()));
    auto mission_set_current =
        std::make_shared<packet_v2::Packet>(to_vector(MissionSetCurrentV2()));
    auto encapsulated_data =
        std::make_shared<packet_v2::Packet>(to_vector(EncapsulatedDataV2()));
    auto param_ext_request_list =
        std::make_shared<packet_v2::Packet>(to_vector(ParamExtRequestListV2()));
    PacketQueue queue;
    SECTION("Maintains order among the same priority")
    {
        queue.push(heartbeat);
        queue.push(ping, 0);
        queue.push(set_mode);
        queue.push(mission_set_current, 0);
        queue.push(encapsulated_data);
        queue.push(param_ext_request_list, 0);
        // HEARTBEAT
        auto packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *heartbeat);
        // PING
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *ping);
        // SET_MODE
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *set_mode);
        // MISSION_SET_CURRENT
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *mission_set_current);
        // ENCAPSULATED_DATA
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *encapsulated_data);
        // PARAM_EXT_REQUEST_LIST
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *param_ext_request_list);
    }
    SECTION("Maintains priority order.")
    {
        queue.push(heartbeat, -1);
        queue.push(ping, 0);
        queue.push(set_mode, 1);
        queue.push(mission_set_current, -3);
        queue.push(encapsulated_data, -2);
        queue.push(param_ext_request_list, 3);
        // PARAM_EXT_REQUEST_LIST
        auto packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *param_ext_request_list);
        // SET_MODE
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *set_mode);
        // PING
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *ping);
        // HEARTBEAT
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *heartbeat);
        // ENCAPSULATED_DATA
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *encapsulated_data);
        // MISSION_SET_CURRENT
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *mission_set_current);
    }
    SECTION("Maintains order among the same priority as well as "
            "priority order.")
    {
        queue.push(heartbeat);
        queue.push(ping, 2);
        queue.push(set_mode);
        queue.push(mission_set_current, 2);
        queue.push(encapsulated_data);
        queue.push(param_ext_request_list, 2);
        // PING
        auto packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *ping);
        // MISSION_SET_CURRENT
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *mission_set_current);
        // PARAM_EXT_REQUEST_LIST
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *param_ext_request_list);
        // HEARTBEAT
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *heartbeat);
        // SET_MODE
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *set_mode);
        // ENCAPSULATED_DATA
        packet = queue.pop();
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *encapsulated_data);
    }
}


TEST_CASE("PacketQueue's is threadsafe.", "[PacketQueue]")
{
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    PacketQueue queue;
    SECTION("The 'pop' method can optionally block.  Will be released when "
            "a packet becomes available.")
    {
        auto future = std::async(std::launch::async, [&](){
            return queue.pop(true);
        });
        auto status = future.wait_for(0s);
        REQUIRE(status != std::future_status::ready);
        queue.push(ping);
        auto result = future.get();
        REQUIRE(result != nullptr);
        REQUIRE(*result == *ping);
    }
    SECTION("The 'pop' method can optionally block.  Will be released when "
            "the 'shutdown' method is called.")
    {
        auto future1 = std::async(std::launch::async, [&](){
            return queue.pop(true);
        });
        auto future2 = std::async(std::launch::async, [&](){
            return queue.pop(true);
        });
        auto status1 = future1.wait_for(0s);
        auto status2 = future2.wait_for(0s);
        REQUIRE(status1 != std::future_status::ready);
        REQUIRE(status2 != std::future_status::ready);
        queue.shutdown();
        auto result1 = future1.get();
        auto result2 = future2.get();
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
    }
}
