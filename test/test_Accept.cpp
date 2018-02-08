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
#include "Rule.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE("Accept's are constructable.", "[Accept]")
{
    SECTION("Without a condition (match all packet/address combinations) or a "
            "priority.")
    {
        REQUIRE_NOTHROW(Accept());
    }
    SECTION("Without a condition (match all packet/address combinations) but "
            "with a priority.")
    {
        REQUIRE_NOTHROW(Accept(3));
    }
    SECTION("With a condition and without a priority.")
    {
        REQUIRE_NOTHROW(Accept(If()));
        REQUIRE_NOTHROW(Accept(If().type("PING")));
        REQUIRE_NOTHROW(Accept(If().from("192.168")));
        REQUIRE_NOTHROW(Accept(If().to("172.16")));
    }
    SECTION("With both a condition and a priority.")
    {
        REQUIRE_NOTHROW(Accept(3, If()));
        REQUIRE_NOTHROW(Accept(3, If().type("PING")));
        REQUIRE_NOTHROW(Accept(3, If().from("192.168")));
        REQUIRE_NOTHROW(Accept(3, If().to("172.16")));
    }
}


TEST_CASE("Accept's are comparable.", "[Accept]")
{
    SECTION("with ==")
    {
        REQUIRE(Accept() == Accept());
        REQUIRE(Accept(If().type("PING")) == Accept(If().type("PING")));
        REQUIRE(Accept(3) == Accept(3));
        REQUIRE(Accept(3, If().type("PING")) == Accept(3, If().type("PING")));
        REQUIRE_FALSE(
            Accept(If().type("PING")) == Accept(If().type("SET_MODE")));
        REQUIRE_FALSE(Accept(If().type("PING")) == Accept(If()));
        REQUIRE_FALSE(Accept(If().type("PING")) == Accept());
        REQUIRE_FALSE(Accept(3) == Accept(-3));
        REQUIRE_FALSE(Accept(3) == Accept());
    }
    SECTION("with !=")
    {
        REQUIRE(Accept(If().type("PING")) != Accept());
        REQUIRE(Accept(If().type("PING")) != Accept(If()));
        REQUIRE(Accept(If().type("PING")) != Accept(If().type("SET_MODE")));
        REQUIRE(Accept(3) != Accept(-3));
        REQUIRE(Accept(3) != Accept());
        REQUIRE_FALSE(Accept() != Accept());
        REQUIRE_FALSE(Accept(If().type("PING")) != Accept(If().type("PING")));
        REQUIRE_FALSE(Accept(3) != Accept(3));
        REQUIRE_FALSE(
            Accept(3, If().type("PING")) != Accept(3, If().type("PING")));
    }
}


TEST_CASE("Accept's 'action' method determines what to do with a "
          "packet/address combination.", "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    RecursionChecker rc;
    SECTION("Returns the accept action if there is no conditional.")
    {
        // Without priority.
        REQUIRE(
            Accept().action(ping, MAVAddress("192.168"), rc) ==
            Action::make_accept());
        // With priority.
        REQUIRE(
            Accept(3).action(ping, MAVAddress("192.168"), rc) ==
            Action::make_accept(3));
    }
    SECTION("Returns the accept action if the conditional is a match.")
    {
        // Without priority.
        REQUIRE(
            Accept(If().type("PING")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_accept());
        REQUIRE(
            Accept(If().to("192.168")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_accept());
        // With priority.
        REQUIRE(
            Accept(3, If().type("PING")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_accept(3));
        REQUIRE(
            Accept(3, If().to("192.168")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_accept(3));
    }
    SECTION("Returns the continue action if the conditional does not match.")
    {
        // Without priority.
        REQUIRE(
            Accept(If().type("SET_MODE")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
        REQUIRE(
            Accept(If().to("172.16")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
        // With priority.
        REQUIRE(
            Accept(3, If().type("SET_MODE")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
        REQUIRE(
            Accept(3, If().to("172.16")).action(
                ping, MAVAddress("192.168"), rc) == Action::make_continue());
    }
}


TEST_CASE("Accept's are printable (without a condition or a priority).",
          "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Accept accept;
    Rule &rule = accept;
    SECTION("By direct type.")
    {
        REQUIRE(str(accept) == "accept");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "accept");
    }
}


TEST_CASE("Accept's are printable (without a condition but with a priority).",
          "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Accept accept(-3);
    Rule &rule = accept;
    SECTION("By direct type.")
    {
        REQUIRE(str(accept) == "accept with priority -3");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "accept with priority -3");
    }
}


TEST_CASE("Accept's are printable (with a condition but without a priority).",
          "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Accept accept(If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = accept;
    SECTION("By direct type.")
    {
        REQUIRE(str(accept) == "accept if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "accept if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("Accept's are printable (with a condition and a priority).",
          "[Accept]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Accept accept(-3, If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = accept;
    SECTION("By direct type.")
    {
        REQUIRE(
            str(accept) ==
            "accept with priority -3 if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(
            str(rule) ==
            "accept with priority -3 if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("Accept's 'clone' method returns a polymorphic copy.", "[Accept]")
{
    Accept accept;
    Rule &rule = accept;
    std::unique_ptr<Rule> polymorphic_copy = rule.clone();
    REQUIRE(accept == *polymorphic_copy);
}
