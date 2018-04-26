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

#include "IPAddress.hpp"
#include "utility.hpp"


TEST_CASE("IPAddress's store an address and a port number.", "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    REQUIRE(a.address() == 0x00000000);
    REQUIRE(a.port() == 0);
    REQUIRE(b.address() == 0xFFFFFFFF);
    REQUIRE(b.port() == 65535);
}


TEST_CASE("IPAddress's are comparable.", "[IPAddress]")
{
    SECTION("with ==")
    {
        REQUIRE(IPAddress(0x00000000, 0) == IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x88888888, 8) == IPAddress(0x88888888, 8));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) == IPAddress(0x00000000, 1));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) == IPAddress(0x00000001, 0));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFE, 65535));
    }
    SECTION("with !=")
    {
        REQUIRE(IPAddress(0x00000000, 0) != IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0x00000000, 0) != IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFF, 65534));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFE, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) != IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) != IPAddress(0x88888888, 8));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFF, 65535));
    }
    SECTION("with <")
    {
        REQUIRE(IPAddress(0x00000000, 0) < IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0x00000000, 0) < IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0x00000000, 1) < IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0xFFFFFFFF, 65534) < IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0xFFFFFFFE, 65535) < IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0x00000000, 0) < IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 1) < IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000001, 0) < IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000001, 0) < IPAddress(0x00000000, 1));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFE, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) < IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) < IPAddress(0x88888888, 8));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) < IPAddress(0x00000000, 0));
    }
    SECTION("with >")
    {
        REQUIRE(IPAddress(0x00000000, 1) > IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x00000001, 0) > IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x00000001, 0) > IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) > IPAddress(0xFFFFFFFF, 65534));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) > IPAddress(0xFFFFFFFE, 65535));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) > IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) > IPAddress(0x00000000, 1));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) > IPAddress(0x00000001, 0));
        REQUIRE_FALSE(IPAddress(0x00000000, 1) > IPAddress(0x00000001, 0));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65534) > IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFE, 65535) > IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) > IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) > IPAddress(0x88888888, 8));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) > IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) > IPAddress(0xFFFFFFFF, 65535));
    }
    SECTION("with <=")
    {
        REQUIRE(IPAddress(0x00000000, 0) <= IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x88888888, 8) <= IPAddress(0x88888888, 8));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0x00000000, 0) <= IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0x00000000, 0) <= IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0x00000000, 1) <= IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0xFFFFFFFF, 65534) <= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0xFFFFFFFE, 65535) <= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0x00000000, 0) <= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 1) <= IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000001, 0) <= IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000001, 0) <= IPAddress(0x00000000, 1));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0xFFFFFFFE, 65535));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0x00000000, 0));
    }
    SECTION("with >=")
    {
        REQUIRE(IPAddress(0x00000000, 0) >= IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x88888888, 8) >= IPAddress(0x88888888, 8));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) >= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE(IPAddress(0x00000000, 1) >= IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x00000001, 0) >= IPAddress(0x00000000, 0));
        REQUIRE(IPAddress(0x00000001, 0) >= IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) >= IPAddress(0xFFFFFFFF, 65534));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) >= IPAddress(0xFFFFFFFE, 65535));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) >= IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) >= IPAddress(0x00000000, 1));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) >= IPAddress(0x00000001, 0));
        REQUIRE_FALSE(IPAddress(0x00000000, 1) >= IPAddress(0x00000001, 0));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFF, 65534) >= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(
            IPAddress(0xFFFFFFFE, 65535) >= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) >= IPAddress(0xFFFFFFFF, 65535));
    }
}


TEST_CASE("IPAddress's can be constructed from an address and port.",
          "[IPAddress]")
{
    REQUIRE(IPAddress(0x00000000, 0) == IPAddress(0x00000000, 0));
    REQUIRE(IPAddress(0x12345678, 123) == IPAddress(0x12345678, 123));
    REQUIRE(IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFF, 65535));
    SECTION("And ensures address and port are within range.")
    {
        // Fails on ARMv7.
        // REQUIRE_THROWS_AS(
        //     IPAddress(static_cast<unsigned long>(-1), 0), std::out_of_range);
        REQUIRE_THROWS_AS(
            IPAddress(0, static_cast<unsigned int>(-1)), std::out_of_range);
        // This causes a compilation error on ARMv7.
        // REQUIRE_THROWS_AS(IPAddress(0x1FFFFFFFF, 65535), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress(0xFFFFFFFF, 65536), std::out_of_range);
    }
}


TEST_CASE("IPAddress's can be constructed from strings.", "[IPAddress]")
{
    SECTION("Without a port number.")
    {
        REQUIRE(IPAddress("192.168.32.128") == IPAddress(0xC0A82080));
    }
    SECTION("With a port number.")
    {
        REQUIRE(IPAddress("192.168.32.128:443") == IPAddress(0xC0A82080, 443));
    }
    SECTION("And will raise std::invalid_argument if it cannot be parsed.")
    {
        // Errors.
        REQUIRE_THROWS_AS(IPAddress("-1.2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.-2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.-3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.-4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("+1.2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.+2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.+3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.+4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1."), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2."), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3."), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.4."), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.4.5"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("a.2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.b.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.c.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.d"), std::invalid_argument);
        REQUIRE_THROWS_AS(
            IPAddress("192.168.32.128.443"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("192.168.128:443"), std::invalid_argument);
        REQUIRE_THROWS_AS(
            IPAddress("192:168:32:128:443"), std::invalid_argument);
        REQUIRE_THROWS_AS(
            IPAddress("192.1b8.32.128:443"), std::invalid_argument);
        // Check error string.
        REQUIRE_THROWS_WITH(
            IPAddress("-1.2.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.-2.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.-3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.-4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("+1.2.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.+2.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.+3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.+4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1."), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2."), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3."), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.4."), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.4.5"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("a.2.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.b.3.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.c.4"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.d"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("192.168.32.128.443"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("192.168.128:443"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("192:168:32:128:443"), "Invalid IP address string.");
        REQUIRE_THROWS_WITH(
            IPAddress("192.1b8.32.128:443"), "Invalid IP address string.");
    }
    SECTION("And ensures address and port are within range.")
    {
        // Errors
        // The following 2 tests had to be removed because they do not work on
        // systems that use 16 bit integers.
        // REQUIRE_THROWS_AS(IPAddress("1.2.3.4:-1"), std::out_of_range);
        // REQUIRE_THROWS_AS(IPAddress("1.2.3.4:65536"), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress("256.2.3.4"), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress("1.256.3.4"), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress("1.2.256.4"), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress("1.2.3.256"), std::out_of_range);
        // Check error string.
        REQUIRE_THROWS_WITH(
            IPAddress("256.2.3.4"),
            "Address octet (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            IPAddress("1.256.3.4"),
            "Address octet (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.256.4"),
            "Address octet (256) is outside of the allowed range (0 - 255).");
        REQUIRE_THROWS_WITH(
            IPAddress("1.2.3.256"),
            "Address octet (256) is outside of the allowed range (0 - 255).");
    }
}


TEST_CASE("IPAddress's are copyable.", "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    IPAddress a_copy = a;
    IPAddress b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a == a_copy);
    REQUIRE(&b != &b_copy);
    REQUIRE(b == b_copy);
}


TEST_CASE("The port of an IP address can be changed during a copy.",
          "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    IPAddress a_copy(a, 65535);
    IPAddress b_copy(b, 0);
    REQUIRE(&a != &a_copy);
    REQUIRE(a.address() == a_copy.address());
    REQUIRE(a.port() == 0);
    REQUIRE(a_copy.port() == 65535);
    REQUIRE(&b != &b_copy);
    REQUIRE(b.address() == b_copy.address());
    REQUIRE(b.port() == 65535);
    REQUIRE(b_copy.port() == 0);
    SECTION("And ensures port number is within range.")
    {
        REQUIRE_THROWS_AS(
            IPAddress(a, static_cast<unsigned int>(-1)), std::out_of_range);
        REQUIRE_THROWS_AS(
            IPAddress(b, static_cast<unsigned int>(-1)), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress(a, 65536), std::out_of_range);
        REQUIRE_THROWS_AS(IPAddress(b, 65536), std::out_of_range);
    }
}


TEST_CASE("IPAddress's are movable.", "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    IPAddress a_moved = std::move(a);
    IPAddress b_moved(std::move(b));
    REQUIRE(a_moved == IPAddress(0x00000000, 0));
    REQUIRE(b_moved == IPAddress(0xFFFFFFFF, 65535));
}


TEST_CASE("IPAddress's are assignable.", "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    REQUIRE(a == IPAddress(0x00000000, 0));
    a = b;
    REQUIRE(a == IPAddress(0xFFFFFFFF, 65535));
}


TEST_CASE("IPAddress's are assignable (by move semantics).", "[IPAddress]")
{
    IPAddress a(0x00000000, 0);
    IPAddress b(0xFFFFFFFF, 65535);
    REQUIRE(a == IPAddress(0x00000000, 0));
    a = std::move(b);
    REQUIRE(a == IPAddress(0xFFFFFFFF, 65535));
}


TEST_CASE("IPAddress's are printable", "[IPAddress]")
{
    SECTION("Without a port number.")
    {
        REQUIRE(str(IPAddress("192.168.32.128")) == "192.168.32.128");
    }
    SECTION("With a port number.")
    {
        REQUIRE(str(IPAddress("192.168.32.128:443")) == "192.168.32.128:443");
    }
}


TEST_CASE("dnslookup finds IPAddress's by domain name.", "[IPAddress]")
{
    SECTION("When they are available.")
    {
        REQUIRE(dnslookup("localhost") == IPAddress("127.0.0.1"));
    }
    SECTION("And throws DNSLookupError otherwise.")
    {
        REQUIRE_THROWS_AS(dnslookup("abc.efg"), DNSLookupError);
    }
}
