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


#include <utility>
#include <stdexcept>

#include <catch.hpp>
#include <fakeit.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Conditional.hpp"


using namespace fakeit;


TEST_CASE("Conditional's are constructable.", "[Conditional]")
{
    SECTION("With the default constructor.")
    {
        REQUIRE_NOTHROW(Conditional());
    }
    SECTION("With ID, source subnet, and destination subnet arguments.")
    {
        REQUIRE_NOTHROW(Conditional({}, {}, {}));
        // ID 4 is PING.
        REQUIRE_NOTHROW(Conditional(4, {}, {}));
        REQUIRE_NOTHROW(Conditional({}, MAVSubnet("192.0/8"), {}));
        REQUIRE_NOTHROW(Conditional({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_NOTHROW(
            Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        // ID 11 is SET_MODE.
        REQUIRE_NOTHROW(
            Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
    SECTION("Source and destination address have default values.")
    {
        REQUIRE_NOTHROW(Conditional(4));
        REQUIRE_NOTHROW(Conditional(11, MAVSubnet("192.0/8")));
    }
    SECTION("Ensures the packet ID is valid.")
    {
        // Currently, ID's 255 and 5000 are invalid.
        REQUIRE_THROWS_AS(Conditional(255), std::invalid_argument);
        REQUIRE_THROWS_WITH(Conditional(255), "Invalid packet ID (#255).");
        REQUIRE_THROWS_AS(Conditional(5000), std::invalid_argument);
        REQUIRE_THROWS_WITH(Conditional(5000), "Invalid packet ID (#5000).");
    }
}


TEST_CASE("Conditional's are comparable.", "[Conditional]")
{
    SECTION("with ==")
    {
        // This also tests the default arguments.
        REQUIRE(Conditional() == Conditional());
        REQUIRE(Conditional({}, {}, {}) == Conditional());
        REQUIRE(Conditional(4, {}, {}) == Conditional(4));
        REQUIRE(Conditional({}, MAVSubnet("192.0/8"), {}) ==
                Conditional({}, MAVSubnet("192.0/8")));
        REQUIRE(Conditional({}, {}, MAVSubnet("192.0/8")) ==
                Conditional({}, {}, MAVSubnet("192.0/8")));
        REQUIRE(Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
                Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE(Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
                Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(Conditional(0, {}, {}) == Conditional());
        REQUIRE_FALSE(Conditional(4, {}, {}) == Conditional(0));
        REQUIRE_FALSE(Conditional({}, MAVSubnet("192.0/8"), {}) ==
                      Conditional({}, MAVSubnet("192.0/7")));
        REQUIRE_FALSE(Conditional({}, {}, MAVSubnet("191.0/8")) ==
                      Conditional({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(
            Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.1/8")));
        REQUIRE_FALSE(
            Conditional(1, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) ==
            Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
    SECTION("with !=")
    {
        // This also tests the default arguments.
        REQUIRE(Conditional(0, {}, {}) != Conditional());
        REQUIRE(Conditional(4, {}, {}) != Conditional(0));
        REQUIRE(Conditional({}, MAVSubnet("192.0/8"), {}) !=
                Conditional({}, MAVSubnet("192.0/7")));
        REQUIRE(Conditional({}, {}, MAVSubnet("191.0/8")) !=
                Conditional({}, {}, MAVSubnet("192.0/8")));
        REQUIRE(Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
                Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.1/8")));
        REQUIRE(Conditional(1, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
                Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(Conditional() != Conditional());
        REQUIRE_FALSE(Conditional({}, {}, {}) != Conditional());
        REQUIRE_FALSE(Conditional(4, {}, {}) != Conditional(4));
        REQUIRE_FALSE(Conditional({}, MAVSubnet("192.0/8"), {}) !=
                      Conditional({}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(Conditional({}, {}, MAVSubnet("192.0/8")) !=
                      Conditional({}, {}, MAVSubnet("192.0/8")));
        REQUIRE_FALSE(
            Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            Conditional({}, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")) !=
            Conditional(11, MAVSubnet("192.0/8"), MAVSubnet("168.0/8")));
    }
}


TEST_CASE("Conditional's are copyable.", "[Conditional]")
{
    Conditional original(4, MAVSubnet("192.168"), MAVSubnet("172.16"));
    Conditional copy(original);
    REQUIRE(copy == Conditional(4, MAVSubnet("192.168"), MAVSubnet("172.16")));
}


TEST_CASE("Conditional's are movable.", "[Conditional]")
{
    Conditional original(4, MAVSubnet("192.168"), MAVSubnet("172.16"));
    Conditional moved(std::move(original));
    REQUIRE(moved == Conditional(4, MAVSubnet("192.168"), MAVSubnet("172.16")));
}


TEST_CASE("Conditional's are assignable.", "[Conditional]")
{
    Conditional a(4, {}, MAVSubnet("255.255"));
    Conditional b(11, MAVSubnet("255.255"), {});
    REQUIRE(a == Conditional(4, {}, MAVSubnet("255.255")));
    a = b;
    REQUIRE(a == Conditional(11, MAVSubnet("255.255"), {}));
}


TEST_CASE("Conditional's are assignable (by move semantics.)", "[Conditional]")
{
    Conditional a(4, {}, MAVSubnet("255.255"));
    Conditional b(11, MAVSubnet("255.255"), {});
    REQUIRE(a == Conditional(4, {}, MAVSubnet("255.255")));
    a = std::move(b);
    REQUIRE(a == Conditional(11, MAVSubnet("255.255"), {}));
}


TEST_CASE("Conditional's 'check' method determines if a packet and destination"
          "address matches the conditional.", "[Conditional]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("Default all matching case.")
    {
        REQUIRE(Conditional().check(packet, address));
    }
    SECTION("Based on packet ID only.")
    {
        REQUIRE_FALSE(Conditional(1).check(packet, address));
        REQUIRE(Conditional(4).check(packet, address));
        REQUIRE_FALSE(Conditional(321).check(packet, address));
    }
    SECTION("Based on source subnet only.")
    {
        REQUIRE(Conditional({}, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(
            Conditional({}, MAVSubnet("193.168")).check(packet, address));
        REQUIRE(Conditional({}, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional({}, MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("Based on destination subnet only.")
    {
        REQUIRE(
            Conditional({}, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(
            Conditional({}, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE(
            Conditional({}, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional({}, {}, MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("Based on packet ID and source subnet only.")
    {
        REQUIRE(Conditional(4, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(4, MAVSubnet("193.168")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("193.168")).check(packet, address));
        REQUIRE(Conditional(4, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(4, MAVSubnet("193.0/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("Based on packet ID and destination subnet only.")
    {
        REQUIRE(
            Conditional(4, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, {}, MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(4, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, {}, MAVSubnet("171.16")).check(packet, address));
        REQUIRE(
            Conditional(4, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, {}, MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(4, {}, MAVSubnet("171.16/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional(11, {}, MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("Based on packet ID, source subnet, and destination subnet.")
    {
        REQUIRE(
            Conditional(4, MAVSubnet("192.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("192.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(4, MAVSubnet("193.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("193.168"), MAVSubnet("172.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(4, MAVSubnet("192.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("192.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(4, MAVSubnet("193.168"), MAVSubnet("171.16")).check(
                packet, address));
        REQUIRE_FALSE(
            Conditional(11, MAVSubnet("193.168"), MAVSubnet("171.16")).check(
                packet, address));
    }
}


TEST_CASE("Conditional's 'type' method sets the packet ID for matching.",
          "[Conditional]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When given a numeric ID.")
    {
        REQUIRE_FALSE(Conditional().type(0).check(packet, address));
        REQUIRE(Conditional().type(4).check(packet, address));
        REQUIRE_FALSE(Conditional().type(11).check(packet, address));
    }
    SECTION("And ensures the numeric ID is valid.")
    {
        // Note: ID's 255 and 5000 are not currently valid.
        REQUIRE_THROWS_AS(Conditional().type(255), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Conditional().type(255), "Invalid packet ID (#255).");
        REQUIRE_THROWS_AS(Conditional().type(5000), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Conditional().type(5000), "Invalid packet ID (#5000).");
    }
    SECTION("When given a packet name.")
    {
        REQUIRE_FALSE(Conditional().type("HEARTBEAT").check(packet, address));
        REQUIRE(Conditional().type("PING").check(packet, address));
        REQUIRE_FALSE(Conditional().type("SET_MODE").check(packet, address));
    }
    SECTION("And ensures the packet name is valid.")
    {
        REQUIRE_THROWS_AS(
            Conditional().type("CRAZY_PACKET_NAME"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Conditional().type("CRAZY_PACKET_NAME"),
            "Invalid packet name (\"CRAZY_PACKET_NAME\").");
    }
}


TEST_CASE("Conditional's 'from' method sets the source subnet to match.",
          "[Conditional]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When a MAVSubnet object is given.")
    {
        REQUIRE(
            Conditional().from(MAVSubnet("192.168")).check(packet, address));
        REQUIRE_FALSE(
            Conditional().from(MAVSubnet("193.168")).check(packet, address));
        REQUIRE(
            Conditional().from(MAVSubnet("192.0/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional().from(MAVSubnet("193.0/8")).check(packet, address));
    }
    SECTION("When a string is given.")
    {
        REQUIRE(Conditional().from("192.168").check(packet, address));
        REQUIRE_FALSE(Conditional().from("193.168").check(packet, address));
        REQUIRE(Conditional().from("192.0/8").check(packet, address));
        REQUIRE_FALSE(Conditional().from("193.0/8").check(packet, address));
    }
}


TEST_CASE("Conditional's 'to' method sets the source subnet to match.",
          "[Conditional]")
{
    Mock<Packet> mock;
    When(Method(mock, id)).AlwaysReturn(4);
    When(Method(mock, source)).AlwaysReturn(MAVAddress("192.168"));
    Packet &packet = mock.get();
    MAVAddress address("172.16");
    SECTION("When a MAVSubnet object is given.")
    {
        REQUIRE(
            Conditional().to(MAVSubnet("172.16")).check(packet, address));
        REQUIRE_FALSE(
            Conditional().to(MAVSubnet("171.16")).check(packet, address));
        REQUIRE(
            Conditional().to(MAVSubnet("172.16/8")).check(packet, address));
        REQUIRE_FALSE(
            Conditional().to(MAVSubnet("171.16/8")).check(packet, address));
    }
    SECTION("When a string is given.")
    {
        REQUIRE(Conditional().to("172.16").check(packet, address));
        REQUIRE_FALSE(Conditional().to("171.16").check(packet, address));
        REQUIRE(Conditional().to("172.16/8").check(packet, address));
        REQUIRE_FALSE(Conditional().to("171.16/8").check(packet, address));
    }
}


TEST_CASE("Conditional's are printable.", "[Conditional]")
{
    REQUIRE(str(Conditional()) == "if any");
    REQUIRE(str(Conditional().type("HEARTBEAT")) == "if HEARTBEAT");
    REQUIRE(str(Conditional().type("PING")) == "if PING");
    REQUIRE(str(Conditional().type("SET_MODE")) == "if SET_MODE");
    REQUIRE(str(Conditional().from("192.168")) == "if from 192.168");
    REQUIRE(str(Conditional().from("192.0/8")) == "if from 192.0/8");
    REQUIRE(str(Conditional().to("172.16")) == "if to 172.16");
    REQUIRE(str(Conditional().to("172.0/8")) == "if to 172.0/8");
    REQUIRE(str(Conditional().type("PING").from("192.168")) ==
            "if PING from 192.168");
    REQUIRE(str(Conditional().type("PING").to("172.16")) ==
            "if PING to 172.16");
    REQUIRE(str(Conditional().from("192.168").to("172.16")) ==
            "if from 192.168 to 172.16");
    REQUIRE(str(Conditional().type("PING").from("192.168").to("172.16")) ==
            "if PING from 192.168 to 172.16");
}
