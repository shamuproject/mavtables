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


#include <stdexcept>
#include <utility>

#include <catch.hpp>

#include "MAVAddress.hpp"
#include "utility.hpp"


TEST_CASE("MAVAddress's store a system and component ID.", "[MAVAddress]")
{
    MAVAddress a(0, 0);
    REQUIRE(a.system() == 0);
    REQUIRE(a.component() == 0);
    REQUIRE(a.address() == 0);
    MAVAddress b(255, 255);
    REQUIRE(b.system() == 255);
    REQUIRE(b.component() == 255);
    REQUIRE(b.address() == 0xFFFF);
    MAVAddress c(255, 0);
    REQUIRE(c.system() == 255);
    REQUIRE(c.component() == 0);
    REQUIRE(c.address() == 0xFF00);
    MAVAddress d(0, 255);
    REQUIRE(d.system() == 0);
    REQUIRE(d.component() == 255);
    REQUIRE(d.address() == 0x00FF);
}


TEST_CASE("MAVAddress's are comparable.", "[MAVAddress]")
{
    SECTION("with ==")
    {
        REQUIRE(MAVAddress(0, 0) == MAVAddress(0, 0));
        REQUIRE(MAVAddress(128, 128) == MAVAddress(128, 128));
        REQUIRE(MAVAddress(255, 255) == MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) == MAVAddress(0, 1));
        REQUIRE_FALSE(MAVAddress(0, 0) == MAVAddress(1, 0));
        REQUIRE_FALSE(MAVAddress(255, 255) == MAVAddress(255, 254));
        REQUIRE_FALSE(MAVAddress(255, 255) == MAVAddress(254, 255));
    }
    SECTION("with !=")
    {
        REQUIRE(MAVAddress(0, 0) != MAVAddress(0, 1));
        REQUIRE(MAVAddress(0, 0) != MAVAddress(1, 0));
        REQUIRE(MAVAddress(255, 255) != MAVAddress(255, 254));
        REQUIRE(MAVAddress(255, 255) != MAVAddress(254, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) != MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(128, 128) != MAVAddress(128, 128));
        REQUIRE_FALSE(MAVAddress(255, 255) != MAVAddress(255, 255));
    }
    SECTION("with <")
    {
        REQUIRE(MAVAddress(0, 0) < MAVAddress(0, 1));
        REQUIRE(MAVAddress(0, 0) < MAVAddress(1, 0));
        REQUIRE(MAVAddress(0, 1) < MAVAddress(1, 0));
        REQUIRE(MAVAddress(255, 254) < MAVAddress(255, 255));
        REQUIRE(MAVAddress(254, 255) < MAVAddress(255, 255));
        REQUIRE(MAVAddress(0, 0) < MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 1) < MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(1, 0) < MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(1, 0) < MAVAddress(0, 1));
        REQUIRE_FALSE(MAVAddress(255, 255) < MAVAddress(255, 254));
        REQUIRE_FALSE(MAVAddress(255, 255) < MAVAddress(254, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) < MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(128, 128) < MAVAddress(128, 128));
        REQUIRE_FALSE(MAVAddress(255, 255) < MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(255, 255) < MAVAddress(0, 0));
    }
    SECTION("with >")
    {
        REQUIRE(MAVAddress(0, 1) > MAVAddress(0, 0));
        REQUIRE(MAVAddress(1, 0) > MAVAddress(0, 0));
        REQUIRE(MAVAddress(1, 0) > MAVAddress(0, 1));
        REQUIRE(MAVAddress(255, 255) > MAVAddress(255, 254));
        REQUIRE(MAVAddress(255, 255) > MAVAddress(254, 254));
        REQUIRE(MAVAddress(255, 255) > MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(0, 0) > MAVAddress(0, 1));
        REQUIRE_FALSE(MAVAddress(0, 0) > MAVAddress(1, 0));
        REQUIRE_FALSE(MAVAddress(0, 1) > MAVAddress(1, 0));
        REQUIRE_FALSE(MAVAddress(255, 254) > MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(254, 255) > MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) > MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(128, 128) > MAVAddress(128, 128));
        REQUIRE_FALSE(MAVAddress(255, 255) > MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) > MAVAddress(255, 255));
    }
    SECTION("with <=")
    {
        REQUIRE(MAVAddress(0, 0) <= MAVAddress(0, 0));
        REQUIRE(MAVAddress(128, 128) <= MAVAddress(128, 128));
        REQUIRE(MAVAddress(255, 255) <= MAVAddress(255, 255));
        REQUIRE(MAVAddress(0, 0) <= MAVAddress(0, 1));
        REQUIRE(MAVAddress(0, 0) <= MAVAddress(1, 0));
        REQUIRE(MAVAddress(0, 1) <= MAVAddress(1, 0));
        REQUIRE(MAVAddress(255, 254) <= MAVAddress(255, 255));
        REQUIRE(MAVAddress(254, 255) <= MAVAddress(255, 255));
        REQUIRE(MAVAddress(0, 0) <= MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 1) <= MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(1, 0) <= MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(1, 0) <= MAVAddress(0, 1));
        REQUIRE_FALSE(MAVAddress(255, 255) <= MAVAddress(255, 254));
        REQUIRE_FALSE(MAVAddress(255, 255) <= MAVAddress(254, 255));
        REQUIRE_FALSE(MAVAddress(255, 255) <= MAVAddress(0, 0));
    }
    SECTION("with >=")
    {
        REQUIRE(MAVAddress(0, 0) >= MAVAddress(0, 0));
        REQUIRE(MAVAddress(128, 128) >= MAVAddress(128, 128));
        REQUIRE(MAVAddress(255, 255) >= MAVAddress(255, 255));
        REQUIRE(MAVAddress(0, 1) >= MAVAddress(0, 0));
        REQUIRE(MAVAddress(1, 0) >= MAVAddress(0, 0));
        REQUIRE(MAVAddress(1, 0) >= MAVAddress(0, 1));
        REQUIRE(MAVAddress(255, 255) >= MAVAddress(255, 254));
        REQUIRE(MAVAddress(255, 255) >= MAVAddress(254, 255));
        REQUIRE(MAVAddress(255, 255) >= MAVAddress(0, 0));
        REQUIRE_FALSE(MAVAddress(0, 0) >= MAVAddress(0, 1));
        REQUIRE_FALSE(MAVAddress(0, 0) >= MAVAddress(1, 0));
        REQUIRE_FALSE(MAVAddress(0, 1) >= MAVAddress(1, 0));
        REQUIRE_FALSE(MAVAddress(255, 254) >= MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(254, 255) >= MAVAddress(255, 255));
        REQUIRE_FALSE(MAVAddress(0, 0) >= MAVAddress(255, 255));
    }
}


TEST_CASE("MAVAddress's can be constructed from a numeric address.",
          "[MAVAddress]")
{
    REQUIRE(MAVAddress(0x0000) == MAVAddress(0x0000));
    REQUIRE(MAVAddress(0x8000) == MAVAddress(0x8000));
    REQUIRE(MAVAddress(0x0080) == MAVAddress(0x0080));
    REQUIRE(MAVAddress(0xFF00) == MAVAddress(0xFF00));
    REQUIRE(MAVAddress(0x00FF) == MAVAddress(0x00FF));
    REQUIRE(MAVAddress(0xFFFF) == MAVAddress(0xFFFF));
    SECTION("And ensures System and Component ID's are within range.")
    {
        REQUIRE_THROWS_AS(MAVAddress(static_cast<unsigned int>(0x0000 - 1)),
                          std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress(0xFFFF + 1), std::out_of_range);
    }
}


TEST_CASE("MAVAddress's can be constructed from System and Component ID's.",
          "[MAVAddress]")
{
    REQUIRE(MAVAddress(0, 0) == MAVAddress(0x0000));
    REQUIRE(MAVAddress(128, 0) == MAVAddress(0x8000));
    REQUIRE(MAVAddress(0, 128) == MAVAddress(0x0080));
    REQUIRE(MAVAddress(255, 0) == MAVAddress(0xFF00));
    REQUIRE(MAVAddress(0, 255) == MAVAddress(0x00FF));
    REQUIRE(MAVAddress(255, 255) == MAVAddress(0xFFFF));
    SECTION("And ensures System and Component ID's are within range.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVAddress(static_cast<unsigned int>(-1), 0),
                          std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress(0, static_cast<unsigned int>(-1)),
                          std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress(256, 255), std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress(255, 256), std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress(256, 256), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVAddress(256, 255),
            "System ID (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            MAVAddress(255, 256),
            "Component ID (256) is outside of the allowed range (0 - 255).");
        // NOTE: MAVAddress(256, 256) is not checked because the order of
        //       checking the inputs is not defined.
    }
}


TEST_CASE("MAVAddress's can be constructed from strings.", "[MAVAddress]")
{
    REQUIRE(MAVAddress("0.0") == MAVAddress(0x0000));
    REQUIRE(MAVAddress("128.0") == MAVAddress(0x8000));
    REQUIRE(MAVAddress("0.128") == MAVAddress(0x0080));
    REQUIRE(MAVAddress("255.0") == MAVAddress(0xFF00));
    REQUIRE(MAVAddress("0.255") == MAVAddress(0x00FF));
    REQUIRE(MAVAddress("255.255") == MAVAddress(0xFFFF));
    REQUIRE(MAVAddress("000.000") == MAVAddress(0x0000));
    REQUIRE(MAVAddress("001.001") == MAVAddress(0x0101));
    REQUIRE(MAVAddress("010.010") == MAVAddress(0x0A0A));
    REQUIRE(MAVAddress("192.168") == MAVAddress(192, 168));
    SECTION("And ensures the address string is valid.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVAddress("1"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("1."), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("1.2."), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("1.2.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("a.2.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("1.b.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("1.2.c"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("+1.0"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("0.+1"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("-1.0"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVAddress("0.-1"), std::invalid_argument);
        // Error message.
        REQUIRE_THROWS_WITH(
            MAVAddress("1"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("1."), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("1.2."), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("1.2.3"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("a.2.3"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("1.b.3"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("1.2.c"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("+1.0"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("0.+1"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("-1.0"), "Invalid MAVLink address string.");
        REQUIRE_THROWS_WITH(
            MAVAddress("0.-1"), "Invalid MAVLink address string.");
    }
    SECTION("And ensures System and Component ID's are within range.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVAddress("256.255"), std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress("255.256"), std::out_of_range);
        REQUIRE_THROWS_AS(MAVAddress("256.256"), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVAddress("256.255"),
            "System ID (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            MAVAddress("255.256"),
            "Component ID (256) is outside of the allowed range (0 - 255).");
        // NOTE: MAVAddress("256.256") is not checked because the order of
        //       checking the inputs is not defined.
    }
}


TEST_CASE("MAVAddress's are copyable.", "[MAVAddress]")
{
    MAVAddress a(0, 0);
    MAVAddress b(255, 255);
    MAVAddress a_copy = a;
    MAVAddress b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a == a_copy);
    REQUIRE(&b != &b_copy);
    REQUIRE(b == b_copy);
}


TEST_CASE("MAVAddress's are movable.", "[MAVAddress]")
{
    MAVAddress a(0, 0);
    MAVAddress b(255, 255);
    MAVAddress a_moved = std::move(a);
    MAVAddress b_moved(std::move(b));
    REQUIRE(a_moved == MAVAddress(0, 0));
    REQUIRE(b_moved == MAVAddress(255, 255));
}


TEST_CASE("MAVAddress's are assignable.", "[MAVAddress]")
{
    MAVAddress a(0, 0);
    MAVAddress b(255, 255);
    REQUIRE(a == MAVAddress(0, 0));
    a = b;
    REQUIRE(a == MAVAddress(255, 255));
}


TEST_CASE("MAVAddress's are assignable (by move semantics).", "[MAVAddress]")
{
    MAVAddress a(0, 0);
    MAVAddress b(255, 255);
    REQUIRE(a == MAVAddress(0, 0));
    a = std::move(b);
    REQUIRE(a == MAVAddress(255, 255));
}


TEST_CASE("MAVAddress's are printable", "[MAVAddress]")
{
    REQUIRE(str(MAVAddress(192, 168)) == "192.168");
    REQUIRE(str(MAVAddress(32, 128)) == "32.128");
}
