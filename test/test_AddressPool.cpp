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


#include <algorithm>
#include <chrono>

#include <catch.hpp>
#include <fake_clock.hh>

#include <AddressPool.hpp>
#include <MAVAddress.hpp>

#include "util.hpp"


using namespace std::chrono_literals;
using namespace testing;


TEST_CASE("AddressPool's can be constructed.", "[AddressPool]")
{
    REQUIRE_NOTHROW(AddressPool<>());
    REQUIRE_NOTHROW(AddressPool<>(10s));
}


TEST_CASE("AddressPool's 'add' method adds an address to the pool.",
          "[AddressPool]")
{
    AddressPool<fake_clock> pool;
    pool.add(MAVAddress("192.168"));
    pool.add(MAVAddress("172.16"));
    pool.add(MAVAddress("10.10"));
    auto addr = pool.addresses();
    std::sort(addr.begin(), addr.end(), std::greater<MAVAddress>());
    REQUIRE(addr.size() == 3);
    std::vector<MAVAddress> compare =
    {
        MAVAddress("192.168"),
        MAVAddress("172.16"),
        MAVAddress("10.10")
    };
    REQUIRE(addr == compare);
}


TEST_CASE("AddressPool's 'contains' method determines whether an address is "
          "in the pool or not.", "[AddressPool]")
{
    AddressPool<fake_clock> pool;
    pool.add(MAVAddress("192.168"));
    pool.add(MAVAddress("172.16"));
    pool.add(MAVAddress("10.10"));
    REQUIRE(pool.contains(MAVAddress("192.168")));
    REQUIRE(pool.contains(MAVAddress("172.16")));
    REQUIRE(pool.contains(MAVAddress("10.10")));
    REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
}


TEST_CASE("AddressPool removes expired addresses.", "[AddressPool]")
{
    SECTION("When using the 'contains' method (and default timeout of 2 min).")
    {
        AddressPool<fake_clock> pool;
        pool.add(MAVAddress("0.0"));
        fake_clock::advance(1s); // 00:00:01
        pool.add(MAVAddress("1.1"));
        fake_clock::advance(1s); // 00:00:02
        pool.add(MAVAddress("2.2"));
        fake_clock::advance(1s); // 00:00:03
        pool.add(MAVAddress("3.3"));
        REQUIRE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(117s); // 00:02:00
        REQUIRE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 00:02:01
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 00:02:02
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 00:02:03
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE_FALSE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 00:02:04
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE_FALSE(pool.contains(MAVAddress("2.2")));
        REQUIRE_FALSE(pool.contains(MAVAddress("3.3")));
    }
    SECTION("When using the 'contains' method (and a custom timeout).")
    {
        AddressPool<fake_clock> pool(1h);
        pool.add(MAVAddress("0.0"));
        fake_clock::advance(1s); // 00:00:01
        pool.add(MAVAddress("1.1"));
        fake_clock::advance(1s); // 00:00:02
        pool.add(MAVAddress("2.2"));
        fake_clock::advance(1s); // 00:00:03
        pool.add(MAVAddress("3.3"));
        REQUIRE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(3597s); // 01:00:00
        REQUIRE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 01:00:01
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 01:00:02
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 01:00:03
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE_FALSE(pool.contains(MAVAddress("2.2")));
        REQUIRE(pool.contains(MAVAddress("3.3")));
        fake_clock::advance(1s); // 01:00:04
        REQUIRE_FALSE(pool.contains(MAVAddress("0.0")));
        REQUIRE_FALSE(pool.contains(MAVAddress("1.1")));
        REQUIRE_FALSE(pool.contains(MAVAddress("2.2")));
        REQUIRE_FALSE(pool.contains(MAVAddress("3.3")));
    }
    SECTION("When using the 'addresses' method (and default timeout of 2 min).")
    {
        AddressPool<fake_clock> pool;
        pool.add(MAVAddress("0.0"));
        fake_clock::advance(1s); // 00:00:01
        pool.add(MAVAddress("1.1"));
        fake_clock::advance(1s); // 00:00:02
        pool.add(MAVAddress("2.2"));
        fake_clock::advance(1s); // 00:00:03
        pool.add(MAVAddress("3.3"));
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("0.0"),
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(117s); // 00:02:00
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("0.0"),
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 00:02:01
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 00:02:02
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 00:02:03
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 00:02:04
        {
            std::vector<MAVAddress> vec;
            REQUIRE(pool.addresses().empty());
        }
    }
    SECTION("When using the 'addresses' method (and a custom timeout).")
    {
        AddressPool<fake_clock> pool(1h);
        pool.add(MAVAddress("0.0"));
        fake_clock::advance(1s); // 00:00:01
        pool.add(MAVAddress("1.1"));
        fake_clock::advance(1s); // 00:00:02
        pool.add(MAVAddress("2.2"));
        fake_clock::advance(1s); // 00:00:03
        pool.add(MAVAddress("3.3"));
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("0.0"),
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(3597s); // 01:00:00
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("0.0"),
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 01:00:01
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("1.1"),
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 01:00:02
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("2.2"),
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 01:00:03
        {
            std::vector<MAVAddress> vec =
            {
                MAVAddress("3.3"),
            };
            auto addr = pool.addresses();
            std::sort(addr.begin(), addr.end());
            REQUIRE(vec == addr);
        }
        fake_clock::advance(1s); // 01:00:04
        {
            std::vector<MAVAddress> vec;
            REQUIRE(pool.addresses().empty());
        }
    }
}
