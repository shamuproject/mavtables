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

#include "Action.hpp"
#include "util.hpp"


TEST_CASE("Action's 'make_accept' factory method constructs an ACCEPT action.",
          "[Action]")
{
    SECTION("Without a priority.")
    {
        auto result = Action::make_accept();
        REQUIRE(result.action() == Action::ACCEPT);
        REQUIRE(result.priority() == 0);
    }
    SECTION("With a priority.")
    {
        auto result = Action::make_accept(-10);
        REQUIRE(result.action() == Action::ACCEPT);
        REQUIRE(result.priority() == -10);
    }
}


TEST_CASE("Action's 'make_reject' factory method constructs a REJECT action.",
          "[Action]")
{
    auto result = Action::make_reject();
    REQUIRE(result.action() == Action::REJECT);
    REQUIRE(result.priority() == 0);
}


TEST_CASE("Action's 'make_continue' factory method constructs a CONTINUE "
          "action.", "[Action]")
{
    auto result = Action::make_continue();
    REQUIRE(result.action() == Action::CONTINUE);
    REQUIRE(result.priority() == 0);
}


TEST_CASE("Action's 'make_default' factory method constructs a DEFAULT action.",
          "[Action]")
{
    auto result = Action::make_default();
    REQUIRE(result.action() == Action::DEFAULT);
    REQUIRE(result.priority() == 0);
}


TEST_CASE("Action's 'priority' method sets and gets the priority.",
          "[Action]")
{
    SECTION("Can be set exactly once for accept results without a priority.")
    {
        auto result = Action::make_accept();
        REQUIRE(result.priority() == 0);
        result.priority(10);
        REQUIRE(result.priority() == 10);
        result.priority(100);
        REQUIRE(result.priority() == 10);
    }
    SECTION("Cannot be set for accept results with a priority.")
    {
        auto result = Action::make_accept(10);
        REQUIRE(result.priority() == 10);
        result.priority(100);
        REQUIRE(result.priority() == 10);
    }
    SECTION("Cannot be set on reject, continue, or default actions.")
    {
        auto reject = Action::make_reject();
        reject.priority(10);
        REQUIRE(reject.priority() == 0);
        auto continue_ = Action::make_continue();
        continue_.priority(10);
        REQUIRE(continue_.priority() == 0);
        auto default_ = Action::make_default();
        default_.priority(10);
        REQUIRE(default_.priority() == 0);
    }
}


TEST_CASE("Action's are comparable.", "[Action]")
{
    SECTION("with ==")
    {
        REQUIRE(Action::make_accept() == Action::make_accept());
        REQUIRE(Action::make_accept(10) == Action::make_accept(10));
        REQUIRE_FALSE(Action::make_accept(1) == Action::make_accept());
        REQUIRE_FALSE(Action::make_accept(1) == Action::make_accept(-1));
        REQUIRE_FALSE(Action::make_accept() == Action::make_reject());
        REQUIRE_FALSE(Action::make_accept() == Action::make_continue());
        REQUIRE_FALSE(Action::make_accept() == Action::make_default());
    }
    SECTION("with !=")
    {
        REQUIRE(Action::make_accept(1) != Action::make_accept());
        REQUIRE(Action::make_accept(1) != Action::make_accept(-1));
        REQUIRE(Action::make_accept() != Action::make_reject());
        REQUIRE(Action::make_accept() != Action::make_continue());
        REQUIRE(Action::make_accept() != Action::make_default());
        REQUIRE_FALSE(Action::make_accept() != Action::make_accept());
        REQUIRE_FALSE(Action::make_accept(10) != Action::make_accept(10));
    }
}


TEST_CASE("Action's are copyable.", "[Action]")
{
    auto original = Action::make_accept(10);
    auto copy(original);
    REQUIRE(copy == Action::make_accept(10));
}


TEST_CASE("Action's are movable.", "[Action]")
{
    auto original = Action::make_accept(10);
    auto moved(std::move(original));
    REQUIRE(moved == Action::make_accept(10));
}


TEST_CASE("Action's are assignable.", "[Action]")
{
    auto a = Action::make_accept(-10);
    auto b = Action::make_accept(100);
    REQUIRE(a == Action::make_accept(-10));
    a = b;
    REQUIRE(a == Action::make_accept(100));
}


TEST_CASE("Action's are assignable (by move semantics).", "[Action]")
{
    auto a = Action::make_accept(-10);
    auto b = Action::make_accept(100);
    REQUIRE(a == Action::make_accept(-10));
    a = std::move(b);
    REQUIRE(a == Action::make_accept(100));
}


TEST_CASE("Action's are printable.")
{
    REQUIRE(str(Action::make_accept()) == "accept");
    REQUIRE(str(Action::make_accept(-10)) == "accept with priority -10");
    REQUIRE(str(Action::make_accept(10)) == "accept with priority 10");
    REQUIRE(str(Action::make_reject()) == "reject");
    REQUIRE(str(Action::make_continue()) == "continue");
    REQUIRE(str(Action::make_default()) == "default");
}
