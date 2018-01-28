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
#include "Reject.hpp"

#include "common_Packet.hpp"


TEST_CASE("Rejects's can be constructed.", "[Reject]")
{
    REQUIRE_NOTHROW(Reject());
}


TEST_CASE("Rejects's 'action' metohod always returns true.", "[Reject]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    Reject action;
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.0")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.1")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.2")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.3")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.4")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.5")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.6")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.7")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.0")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.1")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.2")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.3")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.4")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.5")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.6")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.7")));
}


TEST_CASE("Rejects's are printable.", "[Reject]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    Reject reject;
    Action &action = reject;
    SECTION("By direct type.")
    {
        REQUIRE(str(reject) == "reject");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(reject) == "reject");
    }
}
