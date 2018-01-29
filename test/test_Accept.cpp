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


#include <catch.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "RecursionChecker.hpp"
#include "Action.hpp"
#include "Accept.hpp"

#include "common_Packet.hpp"


TEST_CASE("Accept's can be constructed.", "[Accept]")
{
    REQUIRE_NOTHROW(Accept());
}


TEST_CASE("Accept's 'action' method always returns Action::ACCEPT.", "[Accept]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto hb = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    Accept accept;
    REQUIRE(accept.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.1"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.2"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.3"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.4"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.5"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.6"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(ping, MAVAddress("192.7"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.0"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.1"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.2"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.3"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.4"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.5"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.6"), rc) == Action::ACCEPT);
    REQUIRE(accept.action(hb, MAVAddress("192.7"), rc) == Action::ACCEPT);
}


TEST_CASE("Accept's are printable.", "[Accept]")
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
