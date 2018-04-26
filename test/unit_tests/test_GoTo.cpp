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
#include "Chain.hpp"
#include "GoTo.hpp"
#include "If.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "Rule.hpp"
#include "utility.hpp"

#include "common.hpp"
#include "common_Packet.hpp"
#include "common_Rule.hpp"


TEST_CASE("GoTo's are constructable.", "[GoTo]")
{
    fakeit::Mock<Chain> mock;
    std::shared_ptr<Chain> chain = mock_shared(mock);
    SECTION("Without a condition (match all packet/address combinations) or a "
            "priority.")
    {
        REQUIRE_NOTHROW(GoTo(chain));
    }
    SECTION("Without a condition (match all packet/address combinations) but "
            "with a priority.")
    {
        REQUIRE_NOTHROW(GoTo(chain, 3));
    }
    SECTION("With a condition and without a priority.")
    {
        REQUIRE_NOTHROW(GoTo(chain, If()));
        REQUIRE_NOTHROW(GoTo(chain, If().type("PING")));
        REQUIRE_NOTHROW(GoTo(chain, If().from("192.168")));
        REQUIRE_NOTHROW(GoTo(chain, If().to("172.16")));
    }
    SECTION("With both a condition and a priority.")
    {
        REQUIRE_NOTHROW(GoTo(chain, 3, If()));
        REQUIRE_NOTHROW(GoTo(chain, 3, If().type("PING")));
        REQUIRE_NOTHROW(GoTo(chain, 3, If().from("192.168")));
        REQUIRE_NOTHROW(GoTo(chain, 3, If().to("172.16")));
    }
    SECTION("Ensures the chain's shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(GoTo(nullptr), std::invalid_argument);
        REQUIRE_THROWS_AS(GoTo(nullptr, 3), std::invalid_argument);
        REQUIRE_THROWS_AS(
            GoTo(nullptr, 3, If().type("PING")), std::invalid_argument);
        REQUIRE_THROWS_WITH(GoTo(nullptr), "Given chain pointer is null.");
        REQUIRE_THROWS_WITH(GoTo(nullptr, 3), "Given chain pointer is null.");
        REQUIRE_THROWS_WITH(
            GoTo(nullptr, 3, If().type("PING")),
            "Given chain pointer is null.");
    }
}


TEST_CASE("GoTo's are comparable.", "[GoTo]")
{
    fakeit::Mock<Chain> mock1;
    fakeit::Mock<Chain> mock2;
    std::shared_ptr<Chain> chain1 = mock_shared(mock1);
    std::shared_ptr<Chain> chain2 = mock_shared(mock2);
    SECTION("with ==")
    {
        REQUIRE(GoTo(chain1) == GoTo(chain1));
        REQUIRE(
            GoTo(chain1, If().type("PING")) == GoTo(chain1, If().type("PING")));
        REQUIRE(GoTo(chain1, 3) == GoTo(chain1, 3));
        REQUIRE(
            GoTo(chain1, 3, If().type("PING")) ==
            GoTo(chain1, 3, If().type("PING")));
        REQUIRE_FALSE(GoTo(chain1) == GoTo(chain2));
        REQUIRE_FALSE(
            GoTo(chain1, If().type("PING")) ==
            GoTo(chain1, If().type("SET_MODE")));
        REQUIRE_FALSE(GoTo(chain1, If().type("PING")) == GoTo(chain1, If()));
        REQUIRE_FALSE(GoTo(chain1, If().type("PING")) == GoTo(chain1));
        REQUIRE_FALSE(GoTo(chain1, 3) == GoTo(chain1, -3));
        REQUIRE_FALSE(GoTo(chain1, 3) == GoTo(chain1));
    }
    SECTION("with !=")
    {
        REQUIRE(GoTo(chain1, If().type("PING")) != GoTo(chain1));
        REQUIRE(GoTo(chain1, If().type("PING")) != GoTo(chain1, If()));
        REQUIRE(
            GoTo(chain1, If().type("PING")) !=
            GoTo(chain1, If().type("SET_MODE")));
        REQUIRE(GoTo(chain1, 3) != GoTo(chain1, -3));
        REQUIRE(GoTo(chain1, 3) != GoTo(chain1));
        REQUIRE(GoTo(chain1) != GoTo(chain2));
        REQUIRE_FALSE(GoTo(chain1) != GoTo(chain1));
        REQUIRE_FALSE(
            GoTo(chain1, If().type("PING")) != GoTo(chain1, If().type("PING")));
        REQUIRE_FALSE(GoTo(chain1, 3) != GoTo(chain1, 3));
        REQUIRE_FALSE(
            GoTo(chain1, 3, If().type("PING")) !=
            GoTo(chain1, 3, If().type("PING")));
    }
}


TEST_CASE("GoTo's 'action' method determines what to do with a "
          "packet/address combination.", "[GoTo]")
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
        REQUIRE(GoTo(std::make_shared<TestChain>()).action(
                    ping, MAVAddress("192.168")) == Action::make_accept());
        fakeit::Mock<Chain> mock;
        fakeit::When(Method(mock, action)).AlwaysReturn(Action::make_accept());
        std::shared_ptr<Chain> chain = mock_shared(mock);
        MAVAddress address("192.168");
        GoTo(chain).action(ping, address);
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
            GoTo(accept_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_accept());
        REQUIRE(
            GoTo(reject_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_reject());
        REQUIRE(
            GoTo(continue_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_default());
        REQUIRE(
            GoTo(default_chain).action(ping, MAVAddress("192.168")) ==
            Action::make_default());
        // With priority (adds priority).
        REQUIRE(
            GoTo(accept_chain, 3).action(ping, MAVAddress("192.168")) ==
            Action::make_accept(3));
        // Priority already set (no override).
        REQUIRE(
            GoTo(accept10_chain, 3).action(ping, MAVAddress("192.168")) ==
            Action::make_accept(10));
    }
    SECTION("Delegates to the contained chain if the conditional is a match.")
    {
        // Without priority.
        REQUIRE(
            GoTo(accept_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept());
        REQUIRE(
            GoTo(reject_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_reject());
        REQUIRE(
            GoTo(continue_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_default());
        REQUIRE(
            GoTo(default_chain, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_default());
        // With priority (adds priority).
        REQUIRE(
            GoTo(accept_chain, 3, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept(3));
        // Priority already set (no override).
        REQUIRE(
            GoTo(accept10_chain, 3, If().to("192.168")).action(
                ping, MAVAddress("192.168")) == Action::make_accept(10));
    }
    SECTION("Returns the continue action if the conditional does not match.")
    {
        // Without priority.
        REQUIRE(
            GoTo(accept_chain, If().to("172.16")).action(
                ping, MAVAddress("192.168")) == Action::make_continue());
        // With priority.
        REQUIRE(
            GoTo(accept_chain, 3, If().to("172.16")).action(
                ping, MAVAddress("192.168")) == Action::make_continue());
    }
}


TEST_CASE("GoTo's are printable (without a condition or a priority).", "[GoTo]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    GoTo goto_(chain);
    Rule &rule = goto_;
    SECTION("By direct type.")
    {
        REQUIRE(str(goto_) == "goto test_chain");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "goto test_chain");
    }
}


TEST_CASE("GoTo's are printable (without a condition but with a priority).",
          "[GoTo]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    GoTo goto_(chain, -3);
    Rule &rule = goto_;
    SECTION("By direct type.")
    {
        REQUIRE(str(goto_) == "goto test_chain with priority -3");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(rule) == "goto test_chain with priority -3");
    }
}


TEST_CASE("GoTo's are printable (with a condition but without a priority).",
          "[GoTo]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    GoTo goto_(chain, If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = goto_;
    SECTION("By direct type.")
    {
        REQUIRE(
            str(goto_) == "goto test_chain if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(
            str(rule) == "goto test_chain if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("GoTo's are printable (with a condition and a priority).", "[GoTo]")
{
    auto chain = std::make_shared<TestChain>();
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    GoTo goto_(chain, -3, If().type("PING").from("192.168/8").to("172.16/4"));
    Rule &rule = goto_;
    SECTION("By direct type.")
    {
        REQUIRE(
            str(goto_) ==
            "goto test_chain with priority -3 "
            "if PING from 192.168/8 to 172.16/4");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(
            str(rule) ==
            "goto test_chain with priority -3 "
            "if PING from 192.168/8 to 172.16/4");
    }
}


TEST_CASE("GoTo's 'clone' method returns a polymorphic copy.", "[GoTo]")
{
    auto chain = std::make_shared<TestChain>();
    SECTION("Without a priority.")
    {
        GoTo goto_(chain, If().type("PING"));
        Rule &rule = goto_;
        std::unique_ptr<Rule> polymorphic_copy = rule.clone();
        REQUIRE(goto_ == *polymorphic_copy);
    }
    SECTION("With a priority.")
    {
        GoTo goto_(chain, 3, If().type("PING"));
        Rule &rule = goto_;
        std::unique_ptr<Rule> polymorphic_copy = rule.clone();
        REQUIRE(goto_ == *polymorphic_copy);
    }
}
