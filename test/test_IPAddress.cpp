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


#include <catch.hpp>
#include "IPAddress.hpp"
#include <iostream>
#include <util.hpp>


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
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) == IPAddress(0xFFFFFFFE, 65535));
    }
    SECTION("with !=")
    {
        REQUIRE(IPAddress(0x00000000, 0) != IPAddress(0x00000000, 1));
        REQUIRE(IPAddress(0x00000000, 0) != IPAddress(0x00000001, 0));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFF, 65534));
        REQUIRE(IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFE, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) != IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) != IPAddress(0x88888888, 8));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) != IPAddress(0xFFFFFFFF, 65535));
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
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFE, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) < IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) < IPAddress(0x88888888, 8));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) < IPAddress(0xFFFFFFFF, 65535));
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
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65534) > IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFE, 65535) > IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) > IPAddress(0x00000000, 0));
        REQUIRE_FALSE(IPAddress(0x88888888, 8) > IPAddress(0x88888888, 8));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) > IPAddress(0xFFFFFFFF, 65535));
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
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0xFFFFFFFF, 65534));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65535) <= IPAddress(0xFFFFFFFE, 65535));
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
        REQUIRE_FALSE(IPAddress(0xFFFFFFFF, 65534) >= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0xFFFFFFFE, 65535) >= IPAddress(0xFFFFFFFF, 65535));
        REQUIRE_FALSE(IPAddress(0x00000000, 0) >= IPAddress(0xFFFFFFFF, 65535));
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
        REQUIRE_THROWS_AS(IPAddress("192.168.32.128.443"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("192.168.128:443"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("192:168:32:128:443"), std::invalid_argument);
        REQUIRE_THROWS_AS(IPAddress("192.1b8.32.128:443"), std::invalid_argument);
    }
}


TEST_CASE("IPAddress's are printable", "[IPAddress]")
{
    std::ostringstream oss;
    SECTION("Without a port number.")
    {
        oss << IPAddress("192.168.32.128");
        REQUIRE(oss.str() == "192.168.32.128");
    }
    SECTION("With a port number.")
    {
        oss << IPAddress("192.168.32.128:443");
        REQUIRE(oss.str() == "192.168.32.128:443");
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
