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


#include <ostream>

#include <catch.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "Accept.hpp"

#include "common_Packet.hpp"


TEST_CASE("Accepts's can be constructed.", "[Accept]")
{
    REQUIRE_NOTHROW(Accept());
}


TEST_CASE("Accepts's 'action' metohod always returns true.", "[Accept]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    Accept action;
    REQUIRE(action.action(ping, MAVAddress("192.0")));
    REQUIRE(action.action(ping, MAVAddress("192.1")));
    REQUIRE(action.action(ping, MAVAddress("192.2")));
    REQUIRE(action.action(ping, MAVAddress("192.3")));
    REQUIRE(action.action(ping, MAVAddress("192.4")));
    REQUIRE(action.action(ping, MAVAddress("192.5")));
    REQUIRE(action.action(ping, MAVAddress("192.6")));
    REQUIRE(action.action(ping, MAVAddress("192.7")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.0")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.1")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.2")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.3")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.4")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.5")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.6")));
    REQUIRE(action.action(heartbeat, MAVAddress("192.7")));
}


TEST_CASE("Accepts's are printable.", "[Accepts]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    Accept accept;
    Action &action = accept;
    SECTION("By direct type.")
    {
        REQUIRE(str(accept) == "accept");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(action) == "accept");
    }
}
