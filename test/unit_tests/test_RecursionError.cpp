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


#include <string>
#include <exception>

#include <catch.hpp>

#include "RecursionError.hpp"


[[noreturn]] static void throw_recursion_error(std::string message)
{
    throw RecursionError(message);
}


TEST_CASE("RecursionError's can be thrown.", "[RecursionError]")
{
    SECTION("And can be caught.")
    {
        REQUIRE_THROWS(throw_recursion_error("example"));
    }
    SECTION("And can be caught as DNSLookupError.")
    {
        REQUIRE_THROWS_AS(throw_recursion_error("example"), RecursionError);
    }
    SECTION("And can be caught as std::exception.")
    {
        REQUIRE_THROWS_AS(throw_recursion_error("example"), std::exception);
    }
}


TEST_CASE("RecursionError's have a message.", "[RecursionError]")
{
    REQUIRE_THROWS_WITH(throw_recursion_error("example"), "example");
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (RecursionError).", "[RecursionError]")
{
    RecursionError *recursion = nullptr;
    REQUIRE_NOTHROW(recursion = new RecursionError("error"));
    REQUIRE_NOTHROW(delete recursion);
}
