// MAVLink router and firewall.
// Copyright (C) 2017-2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#include <exception>
#include <string>

#include <catch.hpp>

#include "DNSLookupError.hpp"


[[noreturn]] static void throw_lookup_error(std::string hostname)
{
    throw DNSLookupError(hostname);
}


TEST_CASE("DNSLookupError's can be thrown.", "[DNSLookupError]")
{
    SECTION("And can be caught.")
    {
        REQUIRE_THROWS(throw_lookup_error("example.com"));
    }
    SECTION("And can be caught as DNSLookupError.")
    {
        REQUIRE_THROWS_AS(throw_lookup_error("example.com"), DNSLookupError);
    }
    SECTION("And can be caught as std::exception.")
    {
        REQUIRE_THROWS_AS(throw_lookup_error("example.com"), std::exception);
    }
}


TEST_CASE("The 'what' method gives the unresolved hostname.",
          "[DNSLookupError]")
{
    REQUIRE_THROWS_WITH(
        throw_lookup_error("example.com"),
        "DNSLookupError: Could not find an IP address for \"example.com\"");
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (DNSLookupError).", "[DNSLookupError]")
{
    DNSLookupError *dns = nullptr;
    REQUIRE_NOTHROW(dns = new DNSLookupError("error"));
    REQUIRE_NOTHROW(delete dns);
}
