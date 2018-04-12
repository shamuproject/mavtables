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

#include "PartialSendError.hpp"


[[noreturn]] static void throw_partial_send_error(
    unsigned long bytes_sent, unsigned long total_bytes)
{
    throw PartialSendError(bytes_sent, total_bytes);
}


TEST_CASE("PartialSendError's can be thrown.", "[PartialSendError]")
{
    SECTION("And can be caught.")
    {
        REQUIRE_THROWS(throw_partial_send_error(50, 100));
    }
    SECTION("And can be caught as PartialSendError.")
    {
        REQUIRE_THROWS_AS(throw_partial_send_error(50, 100), PartialSendError);
    }
    SECTION("And can be caught as std::exception.")
    {
        REQUIRE_THROWS_AS(throw_partial_send_error(50, 100), std::exception);
    }
}


TEST_CASE("PartialSendError's have a message.", "[PartialSendError]")
{
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(10, 100), "Could only write 10 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(20, 100), "Could only write 20 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(30, 100), "Could only write 30 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(40, 100), "Could only write 40 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(50, 100), "Could only write 50 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(60, 100), "Could only write 60 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(70, 100), "Could only write 70 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(80, 100), "Could only write 80 of 100 bytes.");
    REQUIRE_THROWS_WITH(
        throw_partial_send_error(90, 100), "Could only write 90 of 100 bytes.");
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (PartialSendError).", "[PartialSendError]")
{
    PartialSendError *partial_send = nullptr;
    REQUIRE_NOTHROW(partial_send = new PartialSendError(50, 100));
    REQUIRE_NOTHROW(delete partial_send);
}
