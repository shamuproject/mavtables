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
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "Reject.hpp"
#include "Rule.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE("Reject's are constructable.", "[Reject]")
{
    SECTION("Without a condition (match all packet/address combinations).")
    {
        REQUIRE_NOTHROW(Reject());
    }
    SECTION("With a condition.")
    {
        REQUIRE_NOTHROW(Reject(If()));
        REQUIRE_NOTHROW(Reject(If().type("PING")));
        REQUIRE_NOTHROW(Reject(If().from("192.168")));
        REQUIRE_NOTHROW(Reject(If().to("172.16")));
    }
}


TEST_CASE("Reject's are comparable.", "[Reject]")
{
    SECTION("with ==")
    {
        REQUIRE(Reject() == Reject());
        REQUIRE(Reject(If().type("PING")) == Reject(If().type("PING")));
        REQUIRE_FALSE(
            Reject(If().type("PING")) == Reject(If().type("SET_MODE")));
        REQUIRE_FALSE(Reject(If().type("PING")) == Reject(If()));
        REQUIRE_FALSE(Reject(If().type("PING")) == Reject());
    }
    SECTION("with !=")
    {
        REQUIRE(Reject(If().type("PING")) != Reject());
        REQUIRE(Reject(If().type("PING")) != Reject(If()));
        REQUIRE(Reject(If().type("PING")) != Reject(If().type("SET_MODE")));
        REQUIRE_FALSE(Reject() != Reject());
        REQUIRE_FALSE(Reject(If().type("PING")) != Reject(If().type("PING")));
    }
}


TEST_CASE("Reject's 'action' method determines what to do with a "
          "packet/address combination.", "[Reject]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    RecursionChecker rc;
    SECTION("Returns the reject action if there is no conditional.")
    {
        REQUIRE(
            Reject().action(ping, MAVAddress("192.168"), rc) ==
            Action::make_reject());
    }
    SECTION("Returns the reject action if the conditional is a match.")
    {
        REQUIRE(
            Reject(If().type("PING")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_reject());
        REQUIRE(
            Reject(If().to("192.168")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_reject());
    }
    SECTION("Returns the continue action if the conditional does not match.")
    {
        REQUIRE(
            Reject(If().type("SET_MODE")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
        REQUIRE(
            Reject(If().to("172.16")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
    }
}


TEST_CASE("Reject's are printable (without a condition).", "[Reject]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Reject reject;
    Rule &rule = reject;
    SECTION("By direct type.")
    {
        REQUIRE(str(reject) == "reject");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "reject");
    }
}


TEST_CASE("Reject's are printable (with a condition).", "[Reject]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Reject reject(If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = reject;
    SECTION("By direct type.")
    {
        REQUIRE(str(reject) == "reject if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "reject if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("Reject's 'clone' method returns a polymorphic copy.", "[Reject]")
{
    Reject reject;
    Rule &rule = reject;
    std::unique_ptr<Rule> polymorphic_copy = rule.clone();
    REQUIRE(reject == *polymorphic_copy);
}
