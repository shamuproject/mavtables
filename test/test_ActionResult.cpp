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

#include <catch.hpp>

#include "ActionResult.hpp"
#include "util.hpp"


TEST_CASE("ActionResult's 'make_accept' factory method constructs an ACCEPT "
          "action.", "[ActionResult]")
{
    SECTION("Without a priority.")
    {
        auto result = ActionResult::make_accept();
        REQUIRE(result.action == ActionResult::ACCEPT);
        REQUIRE_FALSE(result.priority());
    }
    SECTION("With a priority.")
    {
        auto result = ActionResult::make_accept(-10);
        REQUIRE(result.action == ActionResult::ACCEPT);
        REQUIRE(result.priority());
        REQUIRE(result.priority().value() == -10);
    }
}


TEST_CASE("ActionResult's 'make_reject' factory method constructs a REJECT "
          "action.", "[ActionResult]")
{
    auto result = ActionResult::make_reject();
    REQUIRE(result.action == ActionResult::REJECT);
    REQUIRE_FALSE(result.priority());
}


TEST_CASE("ActionResult's 'make_continue' factory method constructs a CONTINUE "
          "action.", "[ActionResult]")
{
    auto result = ActionResult::make_continue();
    REQUIRE(result.action == ActionResult::CONTINUE);
    REQUIRE_FALSE(result.priority());
}


TEST_CASE("ActionResult's 'make_default' factory method constructs a DEFAULT "
          "action.", "[ActionResult]")
{
    auto result = ActionResult::make_default();
    REQUIRE(result.action == ActionResult::DEFAULT);
    REQUIRE_FALSE(result.priority());
}


TEST_CASE("ActionResult's 'priority' method sets and gets the priority.",
          "[ActionResult]")
{
    SECTION("Can be set exactly once for accept results without a priority.")
    {
        auto result = ActionResult::make_accept();
        REQUIRE_FALSE(result.priority());
        result.priority(10);
        REQUIRE(result.priority().value() == 10);
        result.priority(100);
        REQUIRE(result.priority().value() == 10);
    }
    SECTION("Cannot be set for accept results with a priority.")
    {
        auto result = ActionResult::make_accept(10);
        REQUIRE(result.priority().value() == 10);
        result.priority(100);
        REQUIRE(result.priority().value() == 10);
    }
    SECTION("Cannot be set on reject, continue, or default actions.")
    {
        auto reject = ActionResult::make_reject();
        reject.priority(10);
        REQUIRE_FALSE(reject.priority());
        auto continue_ = ActionResult::make_continue();
        continue_.priority(10);
        REQUIRE_FALSE(continue_.priority());
        auto default_ = ActionResult::make_default();
        default_.priority(10);
        REQUIRE_FALSE(default_.priority());
    }
}


TEST_CASE("ActionResult's are comparable.", "[ActionResult]")
{
    SECTION("with ==")
    {
        REQUIRE(ActionResult::make_accept() == ActionResult::make_accept());
        REQUIRE(ActionResult::make_accept(10) == ActionResult::make_accept(10));
        REQUIRE_FALSE(
            ActionResult::make_accept(1) == ActionResult::make_accept());
        REQUIRE_FALSE(
            ActionResult::make_accept(1) == ActionResult::make_accept(-1));
        REQUIRE_FALSE(
            ActionResult::make_accept() == ActionResult::make_reject());
        REQUIRE_FALSE(
            ActionResult::make_accept() == ActionResult::make_continue());
        REQUIRE_FALSE(
            ActionResult::make_accept() == ActionResult::make_default());
    }
    SECTION("with !=")
    {
        REQUIRE(ActionResult::make_accept(1) != ActionResult::make_accept());
        REQUIRE(ActionResult::make_accept(1) != ActionResult::make_accept(-1));
        REQUIRE(ActionResult::make_accept() != ActionResult::make_reject());
        REQUIRE(ActionResult::make_accept() != ActionResult::make_continue());
        REQUIRE(ActionResult::make_accept() != ActionResult::make_default());
        REQUIRE_FALSE(
            ActionResult::make_accept() != ActionResult::make_accept());
        REQUIRE_FALSE(
            ActionResult::make_accept(10) != ActionResult::make_accept(10));
    }
}


TEST_CASE("ActionResult's are copyable.", "[ActionResult]")
{
    auto original = ActionResult::make_accept(10);
    auto copy(original);
    REQUIRE(copy == ActionResult::make_accept(10));
}


TEST_CASE("ActionResult's are movable.", "[ActionResult]")
{
    auto original = ActionResult::make_accept(10);
    auto moved(std::move(original));
    REQUIRE(moved == ActionResult::make_accept(10));
}


TEST_CASE("ActionResult's are printable.")
{
    REQUIRE(str(ActionResult::make_accept()) == "accept");
    REQUIRE(str(ActionResult::make_accept(-10)) == "accept with priority -10");
    REQUIRE(str(ActionResult::make_accept(10)) == "accept with priority 10");
    REQUIRE(str(ActionResult::make_reject()) == "reject");
    REQUIRE(str(ActionResult::make_continue()) == "continue");
    REQUIRE(str(ActionResult::make_default()) == "default");
}
