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


#include <utility>

#include <catch.hpp>
#include <fakeit.hpp>

#include "Accept.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "Filter.hpp"
#include "GoTo.hpp"
#include "PacketVersion2.hpp"
#include "Reject.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


TEST_CASE("Filter's are constructable.", "[Filter]")
{
    Chain chain("test_chain");
    SECTION("Without default accept.") { REQUIRE_NOTHROW(Filter(chain)); }
    SECTION("With default accept.") { REQUIRE_NOTHROW(Filter(chain, true)); }
}


TEST_CASE("Filter's are comparable.", "[Filter]")
{
    Chain chain1("test_chain_1");
    Chain chain2("test_chain_2");
    SECTION("with ==")
    {
        REQUIRE(Filter(chain1) == Filter(chain1));
        REQUIRE(Filter(chain2) == Filter(chain2));
        // Reject is default.
        REQUIRE(Filter(chain1, false) == Filter(chain1));
        REQUIRE(Filter(chain1, true) == Filter(chain1, true));
        REQUIRE(Filter(chain1, false) == Filter(chain1, false));
        REQUIRE_FALSE(Filter(chain1) == Filter(chain2));
        REQUIRE_FALSE(Filter(chain1, true) == Filter(chain1));
    }
    SECTION("with !=")
    {
        REQUIRE(Filter(chain1) != Filter(chain2));
        REQUIRE(Filter(chain1, true) != Filter(chain1));
        REQUIRE_FALSE(Filter(chain1) != Filter(chain1));
        REQUIRE_FALSE(Filter(chain2) != Filter(chain2));
        REQUIRE_FALSE(Filter(chain1, false) != Filter(chain1));
        REQUIRE_FALSE(Filter(chain1, true) != Filter(chain1, true));
        REQUIRE_FALSE(Filter(chain1, false) != Filter(chain1, false));
    }
}


TEST_CASE("Filter's are copyable.", "[Filter]")
{
    auto original = Filter(Chain("test_chain"));
    auto copy(original);
    REQUIRE(copy == Filter(Chain("test_chain")));
}


TEST_CASE("Filter's are movable.", "[Filter]")
{
    auto original = Filter(Chain("test_chain"));
    auto moved(std::move(original));
    REQUIRE(moved == Filter(Chain("test_chain")));
}


TEST_CASE("Filter's are assignable.", "[Filter]")
{
    auto a = Filter(Chain("test_chain_a"));
    auto b = Filter(Chain("test_chain_b"));
    REQUIRE(a == Filter(Chain("test_chain_a")));
    a = b;
    REQUIRE(a == Filter(Chain("test_chain_b")));
}


TEST_CASE("Filter's are assignable (by move semantics).", "[Filter]")
{
    auto a = Filter(Chain("test_chain_a"));
    auto b = Filter(Chain("test_chain_b"));
    REQUIRE(a == Filter(Chain("test_chain_a")));
    a = std::move(b);
    REQUIRE(a == Filter(Chain("test_chain_b")));
}


TEST_CASE(
    "Filter's 'will_accept' method determines whether to accept or "
    "reject a packet/address combination.",
    "[Filter]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    SECTION("Accept packet, default priority.")
    {
        Chain chain("test_chain");
        chain.append(std::make_unique<Accept>());
        REQUIRE(
            Filter(chain).will_accept(ping, MAVAddress("192.168")) ==
            std::make_pair(true, 0));
    }
    SECTION("Accept packet, with priority.")
    {
        Chain chain("test_chain");
        chain.append(std::make_unique<Accept>(3));
        REQUIRE(
            Filter(chain).will_accept(ping, MAVAddress("192.168")) ==
            std::make_pair(true, 3));
    }
    SECTION("Reject packet.")
    {
        Chain chain("test_chain");
        chain.append(std::make_unique<Reject>());
        REQUIRE_FALSE(
            Filter(chain).will_accept(ping, MAVAddress("192.168")).first);
    }
    SECTION("Default action.")
    {
        auto subchain = std::make_shared<Chain>("test_subchain");
        Chain chain("test_chain");
        chain.append(std::make_unique<GoTo>(subchain));
        REQUIRE_FALSE(
            Filter(chain).will_accept(ping, MAVAddress("192.168")).first);
        REQUIRE(
            Filter(chain, true).will_accept(ping, MAVAddress("192.168")) ==
            std::make_pair(true, 0));
    }
    SECTION("Undecided action.")
    {
        auto subchain = std::make_shared<Chain>("test_subchain");
        Chain chain("test_chain");
        chain.append(std::make_unique<Call>(subchain));
        REQUIRE_FALSE(
            Filter(chain).will_accept(ping, MAVAddress("192.168")).first);
        REQUIRE(
            Filter(chain, true).will_accept(ping, MAVAddress("192.168")) ==
            std::make_pair(true, 0));
    }
}
