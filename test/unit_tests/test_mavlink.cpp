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

#include "mavlink.hpp"


TEST_CASE("'message_info' returns the message info structure.", "[mavlink]")
{
    SECTION("When given a numeric message ID.")
    {
        REQUIRE(mavlink::name(0) == "HEARTBEAT");
        REQUIRE(mavlink::name(4) == "PING");
        REQUIRE(mavlink::name(11) == "SET_MODE");
        REQUIRE(mavlink::name(41) == "MISSION_SET_CURRENT");
        REQUIRE(mavlink::name(131) == "ENCAPSULATED_DATA");
        REQUIRE(mavlink::name(321) == "PARAM_EXT_REQUEST_LIST");
    }
    SECTION("Throws and error when given an invalid message ID.")
    {
        // Currently #255 and #5000 are not valid message ID's.
        REQUIRE_THROWS_AS(mavlink::name(255), std::invalid_argument);
        REQUIRE_THROWS_WITH(mavlink::name(255), "Invalid packet ID (#255).");
        REQUIRE_THROWS_AS(mavlink::name(5000), std::invalid_argument);
        REQUIRE_THROWS_WITH(mavlink::name(5000), "Invalid packet ID (#5000).");
    }
    SECTION("When given a message name.")
    {
        REQUIRE(mavlink::id("HEARTBEAT") == 0);
        REQUIRE(mavlink::id("PING") == 4);
        REQUIRE(mavlink::id("SET_MODE") == 11);
        REQUIRE(mavlink::id("MISSION_SET_CURRENT") == 41);
        REQUIRE(mavlink::id("ENCAPSULATED_DATA") == 131);
        REQUIRE(mavlink::id("PARAM_EXT_REQUEST_LIST") == 321);
    }
    SECTION("Throws and error when given an invalid message name.")
    {
        REQUIRE_THROWS_AS(
            mavlink::id("CRAZY_MESSAGE_ID"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            mavlink::id("CRAZY_MESSAGE_ID"),
            "Invalid packet name (\"CRAZY_MESSAGE_ID\").");
    }
}
