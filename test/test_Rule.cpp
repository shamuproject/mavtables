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


#include <memory>

#include <catch.hpp>
#include <fakeit.hpp>

#include "Action.hpp"
#include "Accept.hpp"
#include "Call.hpp"
#include "GoTo.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "Reject.hpp"
#include "Rule.hpp"
#include "util.hpp"

#include "ChainTestClass.hpp"
#include "common_Packet.hpp"


TEST_CASE("Rule's are constructable.", "[Rule]")
{
    auto chain = std::make_shared<ChainTestClass>("test_chain");
    SECTION("With the default constructor.")
    {
        REQUIRE_NOTHROW(Rule());
    }
    SECTION("With action only.  Match all packet/address combinations.")
    {
        REQUIRE_NOTHROW(Rule(std::make_unique<Accept>()));
        REQUIRE_NOTHROW(Rule(std::make_unique<Reject>()));
        REQUIRE_NOTHROW(Rule(std::make_unique<Call>(chain)));
        REQUIRE_NOTHROW(Rule(std::make_unique<GoTo>(chain)));
    }
    SECTION("Ensures action is not nullptr.")
    {
        REQUIRE_THROWS_AS(Rule(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(Rule(nullptr), "Given Action pointer is null.");
    }
    SECTION("With action and condition.")
    {
        REQUIRE_NOTHROW(Rule(std::make_unique<Accept>(), Conditional()));
        REQUIRE_NOTHROW(
            Rule(std::make_unique<Reject>(), Conditional().type("PING")));
        REQUIRE_NOTHROW(
            Rule(std::make_unique<Call>(chain), Conditional().from("192.168")));
        REQUIRE_NOTHROW(
            Rule(std::make_unique<GoTo>(chain), Conditional().to("172.16")));
    }
}


TEST_CASE("Rule's 'action' method determines what to do with a packet.",
          "[Rule]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto chain = std::make_shared<ChainTestClass>("test_chain");
    RecursionChecker rc;
    SECTION("When no conditional is supplied.")
    {
        REQUIRE(Rule(std::make_unique<Accept>()).action(
                    ping, MAVAddress("192.168"), rc) == Action::ACCEPT);
        REQUIRE(Rule(std::make_unique<Reject>()).action(
                    ping, MAVAddress("192.168"), rc) == Action::REJECT);
        REQUIRE(Rule(std::make_unique<Call>(chain)).action(
                    ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(Rule(std::make_unique<Call>(chain)).action(
                    ping, MAVAddress("192.168"), rc) == Action::REJECT);
        REQUIRE(Rule(std::make_unique<GoTo>(chain)).action(
                    ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(Rule(std::make_unique<GoTo>(chain)).action(
                    ping, MAVAddress("192.168"), rc) == Action::REJECT);
        REQUIRE(Rule(std::make_unique<Accept>()).action(
                    heartbeat, MAVAddress("192.168"), rc) == Action::ACCEPT);
        REQUIRE(Rule(std::make_unique<Reject>()).action(
                    heartbeat, MAVAddress("192.168"), rc) == Action::REJECT);
        REQUIRE(Rule(std::make_unique<Call>(chain)).action(
                    heartbeat, MAVAddress("192.0"), rc) == Action::CONTINUE);
        REQUIRE(Rule(std::make_unique<Call>(chain)).action(
                    heartbeat, MAVAddress("192.168"), rc) == Action::CONTINUE);
        REQUIRE(Rule(std::make_unique<GoTo>(chain)).action(
                    heartbeat, MAVAddress("192.0"), rc) == Action::DEFAULT);
        REQUIRE(Rule(std::make_unique<GoTo>(chain)).action(
                    heartbeat, MAVAddress("192.168"), rc) == Action::DEFAULT);
    }
    SECTION("When a conditional is supplied.")
    {
        REQUIRE(Rule(std::make_unique<Accept>(), Conditional(4)).action(
                    ping, MAVAddress("192.168"), rc) == Action::ACCEPT);
        REQUIRE(Rule(std::make_unique<Accept>(), Conditional(4)).action(
                    heartbeat, MAVAddress("192.168"), rc) == Action::CONTINUE);
    }
    SECTION("Can set the priority of the packet.")
    {
        Rule(std::make_unique<Accept>()).with_priority(0).action(
                ping, MAVAddress("192.168"), rc);
        REQUIRE(ping.priority() == 0);
        Rule(std::make_unique<Accept>()).with_priority(-5).action(
                ping, MAVAddress("192.168"), rc);
        REQUIRE(ping.priority() == -5);
        Rule(std::make_unique<Accept>()).with_priority(5).action(
                ping, MAVAddress("192.168"), rc);
        REQUIRE(ping.priority() == 5);
    }
}


TEST_CASE("Rule's 'accept' method sets the action to Accept.", "[Rule]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    RecursionChecker rc;
    REQUIRE(Rule().accept().action(ping, MAVAddress("192.0"), rc) ==
            Action::ACCEPT);
    REQUIRE(Rule().accept().action(ping, MAVAddress("192.168"), rc) ==
            Action::ACCEPT);
    REQUIRE(Rule().accept().action(heartbeat, MAVAddress("192.0"), rc) ==
            Action::ACCEPT);
    REQUIRE(Rule().accept().action(heartbeat, MAVAddress("192.168"), rc) ==
            Action::ACCEPT);
}


TEST_CASE("Rule's 'reject' method sets the action to Reject.", "[Rule]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    RecursionChecker rc;
    REQUIRE(Rule().reject().action(ping, MAVAddress("192.0"), rc) ==
            Action::REJECT);
    REQUIRE(Rule().reject().action(ping, MAVAddress("192.168"), rc) ==
            Action::REJECT);
    REQUIRE(Rule().reject().action(heartbeat, MAVAddress("192.0"), rc) ==
            Action::REJECT);
    REQUIRE(Rule().reject().action(heartbeat, MAVAddress("192.168"), rc) ==
            Action::REJECT);
}


TEST_CASE("Rule's 'call' method sets the action to Call.", "[Rule]")
{
    auto chain = std::make_shared<ChainTestClass>("test_chain");
    auto conn = std::make_shared<ConnectionTestClass>();
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    RecursionChecker rc;
    REQUIRE(Rule().call(chain).action(ping, MAVAddress("192.0"), rc) ==
            Action::ACCEPT);
    REQUIRE(Rule().call(chain).action(ping, MAVAddress("192.168"), rc) ==
            Action::REJECT);
    REQUIRE(Rule().call(chain).action(heartbeat, MAVAddress("192.0"), rc) ==
            Action::CONTINUE);
    REQUIRE(Rule().call(chain).action(heartbeat, MAVAddress("192.168"), rc) ==
            Action::CONTINUE);
}


TEST_CASE("Rule's 'goto_' method sets the action to Goto.", "[Rule]")
{
    auto chain = std::make_shared<ChainTestClass>("test_chain");
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    REQUIRE(Rule().goto_(chain).action(ping, MAVAddress("192.0"), rc) ==
            Action::ACCEPT);
    REQUIRE(Rule().goto_(chain).action(ping, MAVAddress("192.168"), rc) ==
            Action::REJECT);
    REQUIRE(Rule().goto_(chain).action(heartbeat, MAVAddress("192.0"), rc) ==
            Action::DEFAULT);
    REQUIRE(Rule().goto_(chain).action(heartbeat, MAVAddress("192.168"), rc) ==
            Action::DEFAULT);
}


TEST_CASE("Rule's 'if_' method adds a conditional.", "[Rule]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    Rule rule;
    SECTION("Adding type condition.")
    {
        rule.accept().if_().type("PING");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(rule.action(heartbeat, MAVAddress("192.0"), rc) ==
                Action::CONTINUE);
        rule.accept().if_().type("HEARTBEAT");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::CONTINUE);
        REQUIRE(rule.action(heartbeat, MAVAddress("192.0"), rc) ==
                Action::ACCEPT);
    }
    SECTION("Adding source subnet condition.")
    {
        rule.accept().if_().from("60.0/8");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(rule.action(heartbeat, MAVAddress("192.0"), rc) ==
                Action::CONTINUE);
        rule.accept().if_().from("1.0/8");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::CONTINUE);
        REQUIRE(rule.action(heartbeat, MAVAddress("192.0"), rc) ==
                Action::ACCEPT);
    }
    SECTION("Adding destination subnet condition.")
    {
        rule.accept().if_().to("192.0/8");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(rule.action(ping, MAVAddress("192.168"), rc) == Action::ACCEPT);
        rule.accept().if_().to("192.0");
        REQUIRE(rule.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
        REQUIRE(rule.action(ping, MAVAddress("192.168"), rc) ==
                Action::CONTINUE);
    }
}


TEST_CASE("Rule's are printable.", "[Rule]")
{
    auto chain = std::make_shared<ChainTestClass>("test_chain");
    Rule rule;
    SECTION("Nothing is printed when the rule does not have an action.")
    {
        REQUIRE(str(rule) == "");
    }
    SECTION("When there is only an action.")
    {
        REQUIRE(str(rule.accept()) == "accept");
        REQUIRE(str(rule.reject()) == "reject");
        REQUIRE(str(rule.call(chain)) == "call test_chain");
        REQUIRE(str(rule.goto_(chain)) == "goto test_chain");
    }
    SECTION("When there is an action and a conditional.")
    {
        rule.accept().if_().type("PING");
        REQUIRE(str(rule) == "accept if PING");
        rule.reject().if_().from("192.0/8");
        REQUIRE(str(rule) == "reject if from 192.0/8");
        rule.call(chain).if_().to("172.16");
        REQUIRE(str(rule) == "call test_chain if to 172.16");
        rule.goto_(chain).if_().type("PING").from("192.0/8").to("172.16");
        REQUIRE(str(rule) == "goto test_chain if PING from 192.0/8 to 172.16");
    }
}
