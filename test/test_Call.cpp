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
#include <fakeit.hpp>

#include "Action.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "If.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "Rule.hpp"
#include "util.hpp"

#include "common.hpp"
#include "common_Packet.hpp"
#include "common_Rule.hpp"


TEST_CASE("Call's are constructable.", "[Call]")
{
    fakeit::Mock<Chain> mock;
    std::shared_ptr<Chain> chain = mock_shared(mock);
    SECTION("Without a condition (match all packet/address combinations) or a "
            "priority.")
    {
        REQUIRE_NOTHROW(Call(chain));
    }
    SECTION("Without a condition (match all packet/address combinations) but "
            "with a priority.")
    {
        REQUIRE_NOTHROW(Call(chain, 3));
    }
    SECTION("With a condition and without a priority.")
    {
        REQUIRE_NOTHROW(Call(chain, If()));
        REQUIRE_NOTHROW(Call(chain, If().type("PING")));
        REQUIRE_NOTHROW(Call(chain, If().from("192.168")));
        REQUIRE_NOTHROW(Call(chain, If().to("172.16")));
    }
    SECTION("With both a condition and a priority.")
    {
        REQUIRE_NOTHROW(Call(chain, 3, If()));
        REQUIRE_NOTHROW(Call(chain, 3, If().type("PING")));
        REQUIRE_NOTHROW(Call(chain, 3, If().from("192.168")));
        REQUIRE_NOTHROW(Call(chain, 3, If().to("172.16")));
    }
    SECTION("Ensures the chain's shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(Call(nullptr), std::invalid_argument);
        REQUIRE_THROWS_AS(Call(nullptr, 3), std::invalid_argument);
        REQUIRE_THROWS_AS(
            Call(nullptr, 3, If().type("PING")), std::invalid_argument);
        REQUIRE_THROWS_WITH(Call(nullptr), "Given chain pointer is null.");
        REQUIRE_THROWS_WITH(Call(nullptr, 3), "Given chain pointer is null.");
        REQUIRE_THROWS_WITH(
            Call(nullptr, 3, If().type("PING")),
            "Given chain pointer is null.");
    }
}


TEST_CASE("Call's are comparable.", "[Call]")
{
    fakeit::Mock<Chain> mock1;
    fakeit::Mock<Chain> mock2;
    std::shared_ptr<Chain> chain1 = mock_shared(mock1);
    std::shared_ptr<Chain> chain2 = mock_shared(mock2);
    SECTION("with ==")
    {
        REQUIRE(Call(chain1) == Call(chain1));
        REQUIRE(
            Call(chain1, If().type("PING")) == Call(chain1, If().type("PING")));
        REQUIRE(Call(chain1, 3) == Call(chain1, 3));
        REQUIRE(
            Call(chain1, 3, If().type("PING")) ==
            Call(chain1, 3, If().type("PING")));
        REQUIRE_FALSE(Call(chain1) == Call(chain2));
        REQUIRE_FALSE(
            Call(chain1, If().type("PING")) ==
            Call(chain1, If().type("SET_MODE")));
        REQUIRE_FALSE(Call(chain1, If().type("PING")) == Call(chain1, If()));
        REQUIRE_FALSE(Call(chain1, If().type("PING")) == Call(chain1));
        REQUIRE_FALSE(Call(chain1, 3) == Call(chain1, -3));
        REQUIRE_FALSE(Call(chain1, 3) == Call(chain1));
    }
    SECTION("with !=")
    {
        REQUIRE(Call(chain1, If().type("PING")) != Call(chain1));
        REQUIRE(Call(chain1, If().type("PING")) != Call(chain1, If()));
        REQUIRE(
            Call(chain1, If().type("PING")) !=
            Call(chain1, If().type("SET_MODE")));
        REQUIRE(Call(chain1, 3) != Call(chain1, -3));
        REQUIRE(Call(chain1, 3) != Call(chain1));
        REQUIRE(Call(chain1) != Call(chain2));
        REQUIRE_FALSE(Call(chain1) != Call(chain1));
        REQUIRE_FALSE(
            Call(chain1, If().type("PING")) != Call(chain1, If().type("PING")));
        REQUIRE_FALSE(Call(chain1, 3) != Call(chain1, 3));
        REQUIRE_FALSE(
            Call(chain1, 3, If().type("PING")) !=
            Call(chain1, 3, If().type("PING")));
    }
}


TEST_CASE("Call's 'action' method determines what to do with a "
          "packet/address combination.", "[Call]")
{
    fakeit::Mock<Chain> accept_mock;
    fakeit::When(Method(accept_mock, action)).AlwaysReturn(
        Action::make_accept());
    std::shared_ptr<Chain> accept_chain = mock_shared(accept_mock);
    fakeit::Mock<Chain> reject_mock;
    fakeit::When(Method(reject_mock, action)).AlwaysReturn(
        Action::make_reject());
    std::shared_ptr<Chain> reject_chain = mock_shared(reject_mock);
    fakeit::Mock<Chain> continue_mock;
    fakeit::When(Method(continue_mock, action)).AlwaysReturn(
        Action::make_continue());
    std::shared_ptr<Chain> continue_chain = mock_shared(continue_mock);
    fakeit::Mock<Chain> default_mock;
    fakeit::When(Method(default_mock, action)).AlwaysReturn(
        Action::make_default());
    std::shared_ptr<Chain> default_chain = mock_shared(default_mock);
    fakeit::Mock<Chain> accept10_mock;
    fakeit::When(Method(accept10_mock, action)).AlwaysReturn(
        Action::make_accept(10));
    std::shared_ptr<Chain> accept10_chain = mock_shared(accept10_mock);
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    SECTION("Check call to chain's action method.")
    {
        REQUIRE(Call(std::make_shared<TestChain>()).action(
                    ping, MAVAddress("192.168")) == Action::make_accept());
        fakeit::Mock<Chain> mock;
        fakeit::When(Method(mock, action)).AlwaysReturn(Action::make_accept());
        std::shared_ptr<Chain> chain = mock_shared(mock);
        MAVAddress address("192.168");
        Call(chain).action(ping, address);
        fakeit::Verify(
            Method(mock, action).Matching([&](auto & a, auto & b)
        {
            return a == ping && b == MAVAddress("192.168");
        })).Once();
    }
    SECTION("Delegates to the contained chain if there is no conditional.")
    {
        // Without priority.
        REQUIRE(
            Call(accept_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_accept());
        REQUIRE(
            Call(reject_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_reject());
        REQUIRE(
            Call(continue_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_continue());
        REQUIRE(
            Call(default_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_default());
        // With priority (adds priority).
        REQUIRE(
            Call(accept_chain, 3).action(ping, MAVAddress("192.168")) ==
            Action::make_accept(3));
        // Priority already set (no override).
        REQUIRE(
            Call(accept10_chain, 3).action(ping, MAVAddress("192.168")) ==
            Action::make_accept(10));
    }
    SECTION("Delegates to the contained chain if the conditional is a match.")
    {
        // Without priority.
        REQUIRE(
            Call(accept_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept());
        REQUIRE(
            Call(reject_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_reject());
        REQUIRE(
            Call(continue_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_continue());
        REQUIRE(
            Call(default_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_default());
        // With priority (adds priority).
        REQUIRE(
            Call(accept_chain, 3, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept(3));
        // Priority already set (no override).
        REQUIRE(
            Call(accept10_chain, 3, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept(10));
    }
    SECTION("Returns the continue action if the conditional does not match.")
    {
        // Without priority.
        REQUIRE(
            Call(accept_chain, If().to("172.16")).action(
                ping, MAVAddress("192.168")) == Action::make_continue());
        // With priority.
        REQUIRE(
            Call(accept_chain, 3, If().to("172.16")).action(
                ping, MAVAddress("192.168")) == Action::make_continue());
    }
}


TEST_CASE("Call's are printable (without a condition or a priority).", "[Call]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Call call(chain);
    Rule &rule = call;
    SECTION("By direct type.")
    {
        REQUIRE(str(call) == "call test_chain");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "call test_chain");
    }
}


TEST_CASE("Call's are printable (without a condition but with a priority).",
          "[Call]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Call call(chain, -3);
    Rule &rule = call;
    SECTION("By direct type.")
    {
        REQUIRE(str(call) == "call test_chain with priority -3");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "call test_chain with priority -3");
    }
}


TEST_CASE("Call's are printable (with a condition but without a priority).",
          "[Call]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Call call(chain, If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = call;
    SECTION("By direct type.")
    {
        REQUIRE(
            str(call) == "call test_chain if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(
            str(rule) == "call test_chain if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("Call's are printable (with a condition and a priority).", "[Call]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    Call call(chain, -3, If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = call;
    SECTION("By direct type.")
    {
        REQUIRE(
            str(call) ==
            "call test_chain with priority -3 "
            "if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(
            str(rule) ==
            "call test_chain with priority -3 "
            "if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("Call's 'clone' method returns a polymorphic copy.", "[Call]")
{
    auto chain = std::make_shared<TestChain>();
    SECTION("Without a priority.")
    {
        Call call(chain, If().type("PING"));
        Rule &rule = call;
        std::unique_ptr<Rule> polymorphic_copy = rule.clone();
        REQUIRE(call == *polymorphic_copy);
    }
    SECTION("With a priority.")
    {
        Call call(chain, 3, If().type("PING"));
        Rule &rule = call;
        std::unique_ptr<Rule> polymorphic_copy = rule.clone();
        REQUIRE(call == *polymorphic_copy);
    }
}
