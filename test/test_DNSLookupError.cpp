// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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
#include "DNSLookupError.hpp"


[[noreturn]] static void throw_lookup_error(std::string hostname)
{
    throw DNSLookupError(hostname);
}


TEST_CASE("DNSLookupError's can be thrown", "[error]")
{
    SECTION("can be caught")
    {
        REQUIRE_THROWS(throw_lookup_error("example.com"));
    }
    SECTION("can be caught as GNSLookupError")
    {
        REQUIRE_THROWS_AS(throw_lookup_error("example.com"), DNSLookupError);
    }
    SECTION("can be caught as std::exception")
    {
        REQUIRE_THROWS_AS(throw_lookup_error("example.com"), std::exception);
    }
}


TEST_CASE("'what' method gives unresolved hostname", "[error]")
{
    REQUIRE(std::string(DNSLookupError("example.com").what()) ==
            "DNSLookupError: Could not find an IP address for \"example.com\"");
}
