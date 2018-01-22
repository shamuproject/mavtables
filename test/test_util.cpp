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


#include <vector>
#include <catch.hpp>
#include <fakeit.hpp>

#include "util.hpp"


TEST_CASE("to_bytes: Convert numeric types to bytes.", "[to_bytes]")
{
    SECTION("char's can be converted to at least 1 bytes")
    {
        auto bytes = to_bytes(static_cast<char>(0x89));
        REQUIRE(bytes.size() >= 1);
        REQUIRE(bytes[0] == 0x89);
    }
    SECTION("unsigned char's can be converted to at least 1 byte")
    {
        auto bytes = to_bytes(static_cast<unsigned char>(0x89u));
        REQUIRE(bytes.size() >= 1);
        REQUIRE(bytes[0] == 0x89);
    }
    SECTION("short's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<short>(0xFACE));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0xCE);
        REQUIRE(bytes[1] == 0xFA);
    }
    SECTION("unsigned short's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned short>(0xFACEu));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0xCE);
        REQUIRE(bytes[1] == 0xFA);
    }
    SECTION("int's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<int>(0xFACE));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0xCE);
        REQUIRE(bytes[1] == 0xFA);
    }
    SECTION("unsigned int's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned int>(0xFACEu));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0xCE);
        REQUIRE(bytes[1] == 0xFA);
    }
    SECTION("long's can be converted to at least 4 bytes")
    {
        auto bytes = to_bytes(static_cast<long>(0xBA5EBA11));
        REQUIRE(bytes.size() >= 4);
        REQUIRE(bytes[0] == 0x11);
        REQUIRE(bytes[1] == 0xBA);
        REQUIRE(bytes[2] == 0x5E);
        REQUIRE(bytes[3] == 0xBA);
    }
    SECTION("unsigned long's can be converted to at least 4 bytes")
    {
        unsigned long i = 0xBA5EBA11;
        REQUIRE(0xBA5EBA11 == i);
        auto bytes = to_bytes(static_cast<unsigned long>(0xBA5EBA11u));
        REQUIRE(bytes.size() >= 4);
        REQUIRE(bytes[0] == 0x11);
        REQUIRE(bytes[1] == 0xBA);
        REQUIRE(bytes[2] == 0x5E);
        REQUIRE(bytes[3] == 0xBA);
    }
    SECTION("long long's can be converted to at least 8 bytes")
    {
        auto bytes = to_bytes(static_cast<long>(0x0123456789ABCDEF));
        REQUIRE(bytes.size() >= 8);
        REQUIRE(bytes[0] == 0xEF);
        REQUIRE(bytes[1] == 0xCD);
        REQUIRE(bytes[2] == 0xAB);
        REQUIRE(bytes[3] == 0x89);
        REQUIRE(bytes[4] == 0x67);
        REQUIRE(bytes[5] == 0x45);
        REQUIRE(bytes[6] == 0x23);
        REQUIRE(bytes[7] == 0x01);
    }
    SECTION("unsigned long long's can be converted to at least 8 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned long>(0x0123456789ABCDEFu));
        REQUIRE(bytes.size() >= 8);
        REQUIRE(bytes[0] == 0xEF);
        REQUIRE(bytes[1] == 0xCD);
        REQUIRE(bytes[2] == 0xAB);
        REQUIRE(bytes[3] == 0x89);
        REQUIRE(bytes[4] == 0x67);
        REQUIRE(bytes[5] == 0x45);
        REQUIRE(bytes[6] == 0x23);
        REQUIRE(bytes[7] == 0x01);
    }
}


TEST_CASE("str: Convert printable types to strings.", "[to_string]")
{
    REQUIRE(str(256) == "256");
    REQUIRE(str(3.14159) == "3.14159");
    REQUIRE(str("Hello world") == "Hello world");
}
