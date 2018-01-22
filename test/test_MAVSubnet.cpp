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


#include <catch.hpp>
#include <stdexcept>

#include "util.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"


TEST_CASE("MAVSubnet's are comparable.", "[MAVSubnet]")
{
    SECTION("with ==")
    {
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) ==
                MAVSubnet(MAVAddress(0x0000), 0x0000));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) ==
                MAVSubnet(MAVAddress(0x1234), 0x5678));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) ==
                MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) ==
                      MAVSubnet(MAVAddress(0x0001), 0x0000));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) ==
                      MAVSubnet(MAVAddress(0x0000), 0x0001));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) ==
                      MAVSubnet(MAVAddress(0x1235), 0x5678));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) ==
                      MAVSubnet(MAVAddress(0x1234), 0x5679));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) ==
                      MAVSubnet(MAVAddress(0xFFFE), 0xFFFF));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) ==
                      MAVSubnet(MAVAddress(0xFFFF), 0xFFFE));
    }
    SECTION("with !=")
    {
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) !=
                MAVSubnet(MAVAddress(0x0001), 0x0000));
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) !=
                MAVSubnet(MAVAddress(0x0000), 0x0001));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) !=
                MAVSubnet(MAVAddress(0x1235), 0x5678));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) !=
                MAVSubnet(MAVAddress(0x1234), 0x5679));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) !=
                MAVSubnet(MAVAddress(0xFFFE), 0xFFFF));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) !=
                MAVSubnet(MAVAddress(0xFFFF), 0xFFFE));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) !=
                      MAVSubnet(MAVAddress(0x0000), 0x0000));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) !=
                      MAVSubnet(MAVAddress(0x1234), 0x5678));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) !=
                      MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));
    }
}


TEST_CASE("MAVSubnet's can be constructed from a MAVLink address "
          "and a numeric mask.", "[MAVSubnet]")
{
    REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) ==
            MAVSubnet(MAVAddress(0x0000), 0x0000));
    REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) ==
            MAVSubnet(MAVAddress(0x1234), 0x5678));
    REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) ==
            MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));
    SECTION("And ensures the mask is within range (0x0000 - 0xFFFF).")
    {
        REQUIRE_THROWS_AS(
            MAVSubnet(MAVAddress(0x0000),
                      static_cast<unsigned int>(0x0000 - 1)),
            std::out_of_range);
        REQUIRE_THROWS_AS(
                MAVSubnet(MAVAddress(0x0000), 0xFFFF + 1), std::out_of_range);
    }
}


TEST_CASE("MAVSubnet's can be constructed from a MAVLink address, "
          "system mask, and component mask.", "[MAVSubnet]")
{
    REQUIRE(MAVSubnet(MAVAddress(0x0000), 0, 0) ==
            MAVSubnet(MAVAddress(0x0000), 0));
    REQUIRE(MAVSubnet(MAVAddress(0x0000), 255, 0) ==
            MAVSubnet(MAVAddress(0x0000), 0xFF00));
    REQUIRE(MAVSubnet(MAVAddress(0x0000), 0, 255) ==
            MAVSubnet(MAVAddress(0x0000), 0x00FF));
    REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 255, 255) ==
            MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));
    REQUIRE(MAVSubnet(MAVAddress(0x1234), 64, 128) ==
            MAVSubnet(MAVAddress(0x1234), 0x4080));
    REQUIRE(MAVSubnet(MAVAddress(0x1234), 128, 64) ==
            MAVSubnet(MAVAddress(0x1234), 0x8040));
    SECTION("And ensures the system and component masks are within range "
            "(0x00 - 0xFF).")
    {
        auto addr = MAVAddress(0x0000);
        // Errors
        REQUIRE_THROWS_AS(
            MAVSubnet(addr, static_cast<unsigned int>(-1), 0),
            std::out_of_range);
        REQUIRE_THROWS_AS(
            MAVSubnet(addr, 0, static_cast<unsigned int>(-1)),
            std::out_of_range);
        REQUIRE_THROWS_AS(
            MAVSubnet(addr,
                      static_cast<unsigned int>(-1),
                      static_cast<unsigned int>(-1)),
            std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet(addr, 256, 255), std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet(addr, 255, 256), std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet(addr, 256, 256), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVSubnet(addr, 256, 255),
            "System mask (0x100) is outside of the allowed range "
            "(0x00 - 0xFF).");
        REQUIRE_THROWS_WITH(
            MAVSubnet(addr, 255, 256),
            "Component mask (0x100) is outside of the allowed range "
            "(0x00 - 0xFF).");
    }
}


TEST_CASE("MAVSubnet's can be constructed from strings.", "[MAVSubnet]")
{
    auto addr = MAVAddress(255, 16);
    SECTION("Using long notation.")
    {
        REQUIRE(MAVSubnet("255.16:123.234") == MAVSubnet(addr, 123, 234));
        REQUIRE(MAVSubnet("255.16:0.0") == MAVSubnet(addr, 0, 0));
        REQUIRE(MAVSubnet("255.16:128.0") == MAVSubnet(addr, 128, 0));
        REQUIRE(MAVSubnet("255.16:0.240") == MAVSubnet(addr, 0, 240));
        REQUIRE(MAVSubnet("255.16:128.240") == MAVSubnet(addr, 128, 240));
    }
    SECTION("Using forward slash notation.")
    {
        REQUIRE(MAVSubnet("255.16/0") == MAVSubnet(addr, 0b0000000000000000));
        REQUIRE(MAVSubnet("255.16/1") == MAVSubnet(addr, 0b1000000000000000));
        REQUIRE(MAVSubnet("255.16/2") == MAVSubnet(addr, 0b1100000000000000));
        REQUIRE(MAVSubnet("255.16/3") == MAVSubnet(addr, 0b1110000000000000));
        REQUIRE(MAVSubnet("255.16/4") == MAVSubnet(addr, 0b1111000000000000));
        REQUIRE(MAVSubnet("255.16/5") == MAVSubnet(addr, 0b1111100000000000));
        REQUIRE(MAVSubnet("255.16/6") == MAVSubnet(addr, 0b1111110000000000));
        REQUIRE(MAVSubnet("255.16/7") == MAVSubnet(addr, 0b1111111000000000));
        REQUIRE(MAVSubnet("255.16/8") == MAVSubnet(addr, 0b1111111100000000));
        REQUIRE(MAVSubnet("255.16/9") == MAVSubnet(addr, 0b1111111110000000));
        REQUIRE(MAVSubnet("255.16/10") == MAVSubnet(addr, 0b1111111111000000));
        REQUIRE(MAVSubnet("255.16/11") == MAVSubnet(addr, 0b1111111111100000));
        REQUIRE(MAVSubnet("255.16/12") == MAVSubnet(addr, 0b1111111111110000));
        REQUIRE(MAVSubnet("255.16/13") == MAVSubnet(addr, 0b1111111111111000));
        REQUIRE(MAVSubnet("255.16/14") == MAVSubnet(addr, 0b1111111111111100));
        REQUIRE(MAVSubnet("255.16/15") == MAVSubnet(addr, 0b1111111111111110));
        REQUIRE(MAVSubnet("255.16/16") == MAVSubnet(addr, 0b1111111111111111));
    }
    SECTION("Using backslash notation.")
    {
        REQUIRE(MAVSubnet("255.16\\0") == MAVSubnet(addr, 0b0000000000000000));
        REQUIRE(MAVSubnet("255.16\\1") == MAVSubnet(addr, 0b0000000010000000));
        REQUIRE(MAVSubnet("255.16\\2") == MAVSubnet(addr, 0b0000000011000000));
        REQUIRE(MAVSubnet("255.16\\3") == MAVSubnet(addr, 0b0000000011100000));
        REQUIRE(MAVSubnet("255.16\\4") == MAVSubnet(addr, 0b0000000011110000));
        REQUIRE(MAVSubnet("255.16\\5") == MAVSubnet(addr, 0b0000000011111000));
        REQUIRE(MAVSubnet("255.16\\6") == MAVSubnet(addr, 0b0000000011111100));
        REQUIRE(MAVSubnet("255.16\\7") == MAVSubnet(addr, 0b0000000011111110));
        REQUIRE(MAVSubnet("255.16\\8") == MAVSubnet(addr, 0b0000000011111111));
    }
    SECTION("And ensures the subnet string is valid.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVSubnet("255.16 255.256"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16-256.255"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16+256.255"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1."), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1.2."), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1.2.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:a.2.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1.b.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:1.2.c"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:+1.0"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:0.+1"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:-1.0"), std::invalid_argument);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:0.-1"), std::invalid_argument);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16 255.256"),
            "Invalid MAVLink subnet: \"255.16 255.256\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16-256.255"),
            "Invalid MAVLink subnet: \"255.16-256.255\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16+256.255"),
            "Invalid MAVLink subnet: \"255.16+256.255\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1"), "Invalid MAVLink subnet: \"255.16:1\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1."), "Invalid MAVLink subnet: \"255.16:1.\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1.2."),
            "Invalid MAVLink subnet: \"255.16:1.2.\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1.2.3"),
            "Invalid MAVLink subnet: \"255.16:1.2.3\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:a.2.3"),
            "Invalid MAVLink subnet: \"255.16:a.2.3\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1.b.3"),
            "Invalid MAVLink subnet: \"255.16:1.b.3\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:1.2.c"),
            "Invalid MAVLink subnet: \"255.16:1.2.c\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:+1.0"),
            "Invalid MAVLink subnet: \"255.16:+1.0\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:0.+1"),
            "Invalid MAVLink subnet: \"255.16:0.+1\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:-1.0"),
            "Invalid MAVLink subnet: \"255.16:-1.0\".");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:0.-1"),
            "Invalid MAVLink subnet: \"255.16:0.-1\".");
    }
    SECTION("And ensures mask is within range.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVSubnet("255.16:256.255"), std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet("255.16:255.256"), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:256.255"),
            "System ID (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16:255.256"),
            "Component ID (256) is outside of the allowed range (0 - 255).");
    }
    SECTION("And ensures forward slash mask is within range.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVSubnet("255.16/-1"), std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet("255.16/17"), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16/17"),
            "Forward slash mask (17) is outside of allowed range (0 - 16).");
    }
    SECTION("And ensures backslash mask is within range.")
    {
        // Errors
        REQUIRE_THROWS_AS(MAVSubnet("255.16/-1"), std::out_of_range);
        REQUIRE_THROWS_AS(MAVSubnet("255.16\\-1"), std::out_of_range);
        // Error messages.
        REQUIRE_THROWS_WITH(
            MAVSubnet("255.16\\9"),
            "Backslash mask (9) is outside of allowed range (0 - 8).");
    }
}


TEST_CASE("MAVSubnet's are copyable.", "[MAVSubnet]")
{
    MAVSubnet a(MAVAddress(0x0000), 0);
    MAVSubnet b(MAVAddress(0xFFFF), 255);
    MAVSubnet a_copy = a;
    MAVSubnet b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a == a_copy);
    REQUIRE(&b != &b_copy);
    REQUIRE(b == b_copy);
}


TEST_CASE("MAVSubnet's are assignable.", "[MAVSubnet]")
{
    MAVSubnet a(MAVAddress(0x0000), 0, 0);
    REQUIRE(a == MAVSubnet(MAVAddress(0x0000), 0, 0));
    a = MAVSubnet(MAVAddress(0xFFFF), 255, 255);
    REQUIRE(a == MAVSubnet(MAVAddress(0xFFFF), 255, 255));
}


TEST_CASE("MAVSubnet's are printable", "[MAVSubnet]")
{
    auto addr = MAVAddress(255, 16);
    REQUIRE(str(MAVSubnet(addr, 123, 234)) == "255.16:123.234");
    REQUIRE(str(MAVSubnet(addr, 128, 240)) == "255.16:128.240");
    REQUIRE(str(MAVSubnet(addr, 0b0000000000000000)) == "255.16/0");
    REQUIRE(str(MAVSubnet(addr, 0b1000000000000000)) == "255.16/1");
    REQUIRE(str(MAVSubnet(addr, 0b1100000000000000)) == "255.16/2");
    REQUIRE(str(MAVSubnet(addr, 0b1110000000000000)) == "255.16/3");
    REQUIRE(str(MAVSubnet(addr, 0b1111000000000000)) == "255.16/4");
    REQUIRE(str(MAVSubnet(addr, 0b1111100000000000)) == "255.16/5");
    REQUIRE(str(MAVSubnet(addr, 0b1111110000000000)) == "255.16/6");
    REQUIRE(str(MAVSubnet(addr, 0b1111111000000000)) == "255.16/7");
    REQUIRE(str(MAVSubnet(addr, 0b1111111100000000)) == "255.16/8");
    REQUIRE(str(MAVSubnet(addr, 0b1111111110000000)) == "255.16/9");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111000000)) == "255.16/10");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111100000)) == "255.16/11");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111110000)) == "255.16/12");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111111000)) == "255.16/13");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111111100)) == "255.16/14");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111111110)) == "255.16/15");
    REQUIRE(str(MAVSubnet(addr, 0b1111111111111111)) == "255.16/16");
    REQUIRE(str(MAVSubnet(addr, 0b0000000010000000)) == "255.16\\1");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011000000)) == "255.16\\2");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011100000)) == "255.16\\3");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011110000)) == "255.16\\4");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011111000)) == "255.16\\5");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011111100)) == "255.16\\6");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011111110)) == "255.16\\7");
    REQUIRE(str(MAVSubnet(addr, 0b0000000011111111)) == "255.16\\8");
}


TEST_CASE("The 'contains' method determines if a MAVLink address "
          "is in the subnet.",
          "[MAVSubnet]")
{
    REQUIRE(MAVSubnet("0.0:0.0").contains(MAVAddress("0.0")));
    REQUIRE(MAVSubnet("0.0:0.0").contains(MAVAddress("255.255")));
    REQUIRE(MAVSubnet("0.0:255.255").contains(MAVAddress("0.0")));
    REQUIRE_FALSE(MAVSubnet("0.0:255.255").contains(MAVAddress("1.1")));
    REQUIRE_FALSE(MAVSubnet("0.0:255.255").contains(MAVAddress("255.255")));
    SECTION("With subnet 192.0/14")
    {
        MAVSubnet subnet("192.0/14");
        REQUIRE(subnet.contains(MAVAddress("192.0")));
        REQUIRE(subnet.contains(MAVAddress("192.1")));
        REQUIRE(subnet.contains(MAVAddress("192.2")));
        REQUIRE(subnet.contains(MAVAddress("192.3")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.4")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.5")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.255")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("191.0")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("193.1")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("0.2")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("255.3")));
    }
    SECTION("With subnet 192.0\\6")
    {
        MAVSubnet subnet("192.0\\6");
        REQUIRE(subnet.contains(MAVAddress("192.0")));
        REQUIRE(subnet.contains(MAVAddress("192.1")));
        REQUIRE(subnet.contains(MAVAddress("192.2")));
        REQUIRE(subnet.contains(MAVAddress("192.3")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.4")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.5")));
        REQUIRE_FALSE(subnet.contains(MAVAddress("192.255")));
        REQUIRE(subnet.contains(MAVAddress("191.0")));
        REQUIRE(subnet.contains(MAVAddress("193.1")));
        REQUIRE(subnet.contains(MAVAddress("0.2")));
        REQUIRE(subnet.contains(MAVAddress("255.3")));
    }
}
