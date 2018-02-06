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

#include "Action.hpp"
#include "ActionResult.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "Reject.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE("Reject's can be constructed.", "[Reject]")
{
    REQUIRE_NOTHROW(Reject());
}


TEST_CASE("Reject's are comparable.", "[Reject]")
{
    REQUIRE(Reject() == Reject());
    REQUIRE_FALSE(Reject() != Reject());
}


TEST_CASE("Reject's 'action' method always returns a reject result.",
          "[Reject]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    Reject reject;
    REQUIRE(
        reject.action(ping, MAVAddress("192.0"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.1"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.2"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.3"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.4"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.5"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.6"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(ping, MAVAddress("192.7"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.0"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.1"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.2"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.3"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.4"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.5"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.6"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        reject.action(set_mode, MAVAddress("192.7"), rc) ==
        ActionResult::make_reject());
}


TEST_CASE("Reject's are printable.", "[Reject]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
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


TEST_CASE("Reject's 'clone' method returns a polymorphic copy.", "[Reject]")
{
    Reject reject;
    Action &action = reject;
    std::unique_ptr<Action> polymorphic_copy = action.clone();
    REQUIRE(reject == *polymorphic_copy);
}
