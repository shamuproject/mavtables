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


#include <stdexcept>
#include <utility>

#include <catch.hpp>
#include <fakeit.hpp>

#include "If.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "utility.hpp"


using namespace fakeit;


TEST_CASE("If's are constructable.", "[If]")
{
    SECTION("With the default constructor.")
    {
        REQUIRE_NOTHROW(If());
    }
    SECTION("With ID, source subnet, and destination subnet arguments.")
    {
        REQUIRE_NOTHROW(If({}, {}, {}));
        // ID 4 is PING.
        REQUIRE_NOTHROW(If(4, {}, {}));
        REQUIRE_NOTHROW(If({}, MAVSubnet("192.0/8"), {}));
        REQUIRE_NOTHROW(If({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_NOTHROW(If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        // ID 11 is SET_MODE.
        REQUIRE_NOTHROW(If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
    SECTION("ID as well as source and destination address are optional.")
    {
        REQUIRE_NOTHROW(If());
        REQUIRE_NOTHROW(If(4));
        REQUIRE_NOTHROW(If(11, MAVSubnet("192.0/8")));
    }
    SECTION("Ensures the packet ID is valid.")
    {
        // Currently, ID's 255 and 5000 are invalid.
        REQUIRE_THROWS_AS(If(255), std::invalid_argument);
        REQUIRE_THROWS_WITH(If(255), "Invalid packet ID (#255).");
        REQUIRE_THROWS_AS(If(5000), std::invalid_argument);
        REQUIRE_THROWS_WITH(If(5000), "Invalid packet ID (#5000).");
    }
}


TEST_CASE("If's are comparable.", "[If]")
{
    SECTION("with ==")
    {
        // This also tests the default arguments.
        REQUIRE(If() == If());
        REQUIRE(If({}, {}, {}) == If());
        REQUIRE(If(4, {}, {}) == If(4));
        REQUIRE(
            If({}, MAVSubnet("192.0/8"), {}) ==
            If({}, MAVSubnet("192.0/8")));
        REQUIRE(
            If({}, {}, MAVSubnet("192.0/8")) ==
            If({}, {}, MAVSubnet("192.0/8")));
        REQUIRE(
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE(
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(If(0, {}, {}) == If());
        REQUIRE_FALSE(If(4, {}, {}) == If(0));
        REQUIRE_FALSE(
            If({}, MAVSubnet("192.0/8"), {}) ==
            If({}, MAVSubnet("192.0/7")));
        REQUIRE_FALSE(
            If({}, {}, MAVSubnet("191.0/8")) ==
            If({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.1/8")));
        REQUIRE_FALSE(
            If(1, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
    SECTION("with !=")
    {
        // This also tests the default arguments.
        REQUIRE(If(0, {}, {}) != If());
        REQUIRE(If(4, {}, {}) != If(0));
        REQUIRE(
            If({}, MAVSubnet("192.0/8"), {}) !=
            If({}, MAVSubnet("192.0/7")));
        REQUIRE(
            If({}, {}, MAVSubnet("191.0/8")) !=
            If({}, {}, MAVSubnet("192.0/8")));
        REQUIRE(
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.1/8")));
        REQUIRE(
            If(1, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(If() != If());
        REQUIRE_FALSE(If({}, {}, {}) != If());
        REQUIRE_FALSE(If(4, {}, {}) != If(4));
        REQUIRE_FALSE(
            If({}, MAVSubnet("192.0/8"), {}) !=
            If({}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(
            If({}, {}, MAVSubnet("192.0/8")) !=
            If({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            If({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            If(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
}


TEST_CASE("If's are copyable.", "[If]")
{
    If original(4, MAVSubnet("192.168"), MAVSubnet("172.16"));
    If copy(original);
    REQUIRE(copy == If(4, MAVSubnet("192.168"), MAVSubnet("172.16")));
}


TEST_CASE("If's are movable.", "[If]")
{
    If original(4, MAVSubnet("192.168"), MAVSubnet("172.16"));
    If moved(std::move(original));
    REQUIRE(moved == If(4, MAVSubnet("192.168"), MAVSubnet("172.16")));
}


TEST_CASE("If's are assignable.", "[If]")
{
    If if_a(4, {}, MAVSubnet("255.255"));
    If if_b(11, MAVSubnet("255.255"), {});
    REQUIRE(if_a == If(4, {}, MAVSubnet("255.255")));
    if_a = if_b;
    REQUIRE(if_a == If(11, MAVSubnet("255.255"), {}));
}


TEST_CASE("If's are assignable (by move semantics.)", "[If]")
{
    If if_a(4, {}, MAVSubnet("255.255"));
    If if_b(11, MAVSubnet("255.255"), {});
    REQUIRE(if_a == If(4, {}, MAVSubnet("255.255")));
    if_a = std::move(if_b);
    REQUIRE(if_a == If(11, MAVSubnet("255.255"), {}));
}


TEST_CASE("If's 'check' method determines if a packet and destination"
          "address matches the conditional.", "[If]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("Default all matching case.")
    {
        REQUIRE(If().check(packet, address));
    }
    SECTION("Based on packet ID only.")
    {
        REQUIRE_FALSE(If(1).check(packet, address));
        REQUIRE(If(4).check(packet, address));
        REQUIRE_FALSE(If(321).check(packet, address));
    }
    SECTION("Based on source subnet only.")
    {
        REQUIRE(If({}, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(If({}, MAVSubnet("193.168")).check(packet, address));
        REQUIRE(If({}, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(If({}, MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("Based on destination subnet only.")
    {
        REQUIRE(If({}, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(If({}, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE(If({}, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(If({}, {}, MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("Based on packet ID and source subnet only.")
    {
        REQUIRE(If(4, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(If(11, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(If(4, MAVSubnet("193.168")).check(packet, address));
        REQUIRE_FALSE(If(11, MAVSubnet("193.168")).check(packet, address));
        REQUIRE(If(4, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(If(11, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(If(4, MAVSubnet("193.0/8")).check(packet, address));
        REQUIRE_FALSE(If(11, MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("Based on packet ID and destination subnet only.")
    {
        REQUIRE(If(4, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(If(11, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(If(4, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE_FALSE(If(11, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE(If(4, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(If(11, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(If(4, {}, MAVSubnet("171.16/8")).check(packet, address));
        REQUIRE_FALSE(If(11, {}, MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("Based on packet ID, source subnet, and destination subnet.")
    {
        REQUIRE(
            If(4, MAVSubnet("192.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(11, MAVSubnet("192.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(4, MAVSubnet("193.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(11, MAVSubnet("193.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(4, MAVSubnet("192.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(11, MAVSubnet("192.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(4, MAVSubnet("193.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            If(11, MAVSubnet("193.168"), MAVSubnet("171.16")).check(
                packet, address));
    }
}


TEST_CASE("If's 'type' method sets the packet ID for matching.",
          "[If]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When given a numeric ID.")
    {
        REQUIRE_FALSE(If().type(0).check(packet, address));
        REQUIRE(If().type(4).check(packet, address));
        REQUIRE_FALSE(If().type(11).check(packet, address));
    }
    SECTION("And ensures the numeric ID is valid.")
    {
        // Note: ID's 255 and 5000 are not currently valid.
        REQUIRE_THROWS_AS(If().type(255), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            If().type(255), "Invalid packet ID (#255).");
        REQUIRE_THROWS_AS(If().type(5000), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            If().type(5000), "Invalid packet ID (#5000).");
    }
    SECTION("When given a packet name.")
    {
        REQUIRE_FALSE(If().type("HEARTBEAT").check(packet, address));
        REQUIRE(If().type("PING").check(packet, address));
        REQUIRE_FALSE(If().type("SET_MODE").check(packet, address));
    }
    SECTION("And ensures the packet name is valid.")
    {
        REQUIRE_THROWS_AS(
            If().type("CRAZY_PACKET_NAME"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            If().type("CRAZY_PACKET_NAME"),
            "Invalid packet name (\"CRAZY_PACKET_NAME\").");
    }
}


TEST_CASE("If's 'from' method sets the source subnet to match.",
          "[If]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When a MAVSubnet object is given.")
    {
        REQUIRE(If().from(MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(If().from(MAVSubnet("193.168")).check(packet, address));
        REQUIRE(If().from(MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(If().from(MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("When a string is given.")
    {
        REQUIRE(If().from("192.168").check(packet, address));
        REQUIRE_FALSE(If().from("193.168").check(packet, address));
        REQUIRE(If().from("192.0/8").check(packet, address));
        REQUIRE_FALSE(If().from("193.0/8").check(packet, address));
    }
}


TEST_CASE("If's 'to' method sets the source subnet to match.",
          "[If]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When a MAVSubnet object is given.")
    {
        REQUIRE(If().to(MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(If().to(MAVSubnet("171.16")).check(packet, address));
        REQUIRE(If().to(MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(If().to(MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("When a string is given.")
    {
        REQUIRE(If().to("172.16").check(packet, address));
        REQUIRE_FALSE(If().to("171.16").check(packet, address));
        REQUIRE(If().to("172.16/8").check(packet, address));
        REQUIRE_FALSE(If().to("171.16/8").check(packet, address));
    }
}


TEST_CASE("If's are printable.", "[If]")
{
    REQUIRE(str(If()) == "if any");
    REQUIRE(str(If().type("HEARTBEAT")) == "if HEARTBEAT");
    REQUIRE(str(If().type("PING")) == "if PING");
    REQUIRE(str(If().type("SET_MODE")) == "if SET_MODE");
    REQUIRE(str(If().from("192.168")) == "if from 192.168");
    REQUIRE(str(If().from("192.0/8")) == "if from 192.0/8");
    REQUIRE(str(If().to("172.16")) == "if to 172.16");
    REQUIRE(str(If().to("172.0/8")) == "if to 172.0/8");
    REQUIRE(
        str(If().type("PING").from("192.168")) == "if PING from 192.168");
    REQUIRE(
        str(If().type("PING").to("172.16")) == "if PING to 172.16");
    REQUIRE(
        str(If().from("192.168").to("172.16")) == "if from 192.168 to 172.16");
    REQUIRE(
        str(If().type("PING").from("192.168").to("172.16")) ==
        "if PING from 192.168 to 172.16");
}
