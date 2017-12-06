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


using namespace fakeit;


TEST_CASE("to_bytes: Convert numeric types to bytes.", "[to_bytes]")
{
    SECTION("char's can be converted to at least 1 bytes")
    {
        auto bytes = to_bytes(static_cast<char>(0x89));
        REQUIRE(bytes.size() >= 1);
        REQUIRE(bytes[0] == 0X89);
    }
    SECTION("unsigned char's can be converted to at least 1 byte")
    {
        auto bytes = to_bytes(static_cast<unsigned char>(0x89u));
        REQUIRE(bytes.size() >= 1);
        REQUIRE(bytes[0] == 0X89);
    }
    SECTION("short's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<short>(0xFACE));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0XCE);
        REQUIRE(bytes[1] == 0XFA);
    }
    SECTION("unsigned short's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned short>(0xFACEu));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0XCE);
        REQUIRE(bytes[1] == 0XFA);
    }
    SECTION("int's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<int>(0xFACE));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0XCE);
        REQUIRE(bytes[1] == 0XFA);
    }
    SECTION("unsigned int's can be converted to at least 2 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned int>(0xFACEu));
        REQUIRE(bytes.size() >= 2);
        REQUIRE(bytes[0] == 0XCE);
        REQUIRE(bytes[1] == 0XFA);
    }
    SECTION("long's can be converted to at least 4 bytes")
    {
        auto bytes = to_bytes(static_cast<long>(0xBA5EBA11));
        REQUIRE(bytes.size() >= 4);
        REQUIRE(bytes[0] == 0X11);
        REQUIRE(bytes[1] == 0XBA);
        REQUIRE(bytes[2] == 0X5E);
        REQUIRE(bytes[3] == 0XBA);
    }
    SECTION("unsigned long's can be converted to at least 4 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned long>(0xBA5EBA11u));
        REQUIRE(bytes.size() >= 4);
        REQUIRE(bytes[0] == 0X11);
        REQUIRE(bytes[1] == 0XBA);
        REQUIRE(bytes[2] == 0X5E);
        REQUIRE(bytes[3] == 0XBA);
    }
    SECTION("long long's can be converted to at least 8 bytes")
    {
        auto bytes = to_bytes(static_cast<long>(0x0123456789ABCDEF));
        REQUIRE(bytes.size() >= 8);
        REQUIRE(bytes[0] == 0XEF);
        REQUIRE(bytes[1] == 0XCD);
        REQUIRE(bytes[2] == 0XAB);
        REQUIRE(bytes[3] == 0X89);
        REQUIRE(bytes[4] == 0X67);
        REQUIRE(bytes[5] == 0X45);
        REQUIRE(bytes[6] == 0X23);
        REQUIRE(bytes[7] == 0X01);
    }
    SECTION("unsigned long long's can be converted to at least 8 bytes")
    {
        auto bytes = to_bytes(static_cast<unsigned long>(0x0123456789ABCDEFu));
        REQUIRE(bytes.size() >= 8);
        REQUIRE(bytes[0] == 0XEF);
        REQUIRE(bytes[1] == 0XCD);
        REQUIRE(bytes[2] == 0XAB);
        REQUIRE(bytes[3] == 0X89);
        REQUIRE(bytes[4] == 0X67);
        REQUIRE(bytes[5] == 0X45);
        REQUIRE(bytes[6] == 0X23);
        REQUIRE(bytes[7] == 0X01);
    }
}


TEST_CASE("str: Convert printable types to strings.", "[to_string]")
{
    REQUIRE(str(256) == "256");
    REQUIRE(str(3.14159) == "3.14159");
    REQUIRE(str("Hello world") == "Hello world");
}


TEST_CASE("capital_case capitalizes first character of a string.",
          "[captal_case]")
{
    REQUIRE(capital_case("the quick brown fox.") == "The quick brown fox.");
}


TEST_CASE("fakeit example")
{
    // Sample interface.
    struct SomeInterface
    {
        virtual int is_positive(int) = 0;
        virtual int capital_case(std::string) = 0;
        virtual ~SomeInterface() = 0;
    };
    Mock<SomeInterface> mock;
    When(Method(mock, is_positive).Using(Gt(0))).AlwaysReturn(true);
    When(Method(mock, is_positive).Using(Le(0))).AlwaysReturn(false);
    When(Method(mock, capital_case)).Return(0, 1, 2);
    // Get rerference to the interface and test.
    SomeInterface &i = mock.get();
    REQUIRE(i.is_positive(-1) == false);
    REQUIRE(i.is_positive(0) == false);
    REQUIRE(i.is_positive(1) == true);
    i.capital_case("hello world");
    // Verify methods were called.
    Verify(Method(mock, is_positive)).Exactly(3);
    Verify(Method(mock, capital_case));
}
