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

#include "Accept.hpp"
#include "Action.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE("Accept's can be constructed.", "[Accept]")
{
    REQUIRE_NOTHROW(Accept());
    REQUIRE_NOTHROW(Accept(3));
}


TEST_CASE("Accept's are comparable.", "[Accept]")
{
    SECTION("with ==")
    {
        REQUIRE(Accept() == Accept());
        REQUIRE_FALSE(Accept() == Accept(3));
    }
    SECTION("with !=")
    {
        REQUIRE(Accept() != Accept(3));
        REQUIRE_FALSE(Accept() != Accept());
    }
}


TEST_CASE("Accept's 'action' method always returns Action::ACCEPT.", "[Accept]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    SECTION("Without a priority.")
    {
        Accept accept;
        REQUIRE(
            accept.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_accept());
    }
    SECTION("With a priority.")
    {
        Accept accept(3);
        REQUIRE(
            accept.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            accept.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_accept(3));
    }
}


TEST_CASE("Accept's are printable by polymorphic type.", "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    SECTION("Without priority.")
    {
        Accept accept;
        Action &action = accept;
        REQUIRE(str(accept) == "accept");
        REQUIRE(str(action) == "accept");
    }
    SECTION("With priority.")
    {
        Accept accept(-3);
        Action &action = accept;
        REQUIRE(str(accept) == "accept with priority -3");
        REQUIRE(str(action) == "accept with priority -3");
    }
}


TEST_CASE("Accept's 'clone' method returns a polymorphic copy.", "[Accept]")
{
    Accept accept;
    Action &action = accept;
    std::unique_ptr<Action> polymorphic_copy = action.clone();
    REQUIRE(accept == *polymorphic_copy);
}
