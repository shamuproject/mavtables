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


#include <memory>

#include <catch.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "RecursionChecker.hpp"
#include "Chain.hpp"
#include "Action.hpp"
#include "Call.hpp"

#include "ChainTestClass.hpp"
#include "common_Packet.hpp"


TEST_CASE("Call's can be constructed.", "[Call]")
{
    REQUIRE_NOTHROW(Call(std::make_shared<ChainTestClass>("test_chain")));
    SECTION("Ensures the shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(Call(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(Call(nullptr), "Given Chain pointer is null.");
    }
}


TEST_CASE("Call's 'action' method delegates the decision to the Chain it "
          "contains.", "[Call]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto hb = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    Call call(std::make_shared<ChainTestClass>("test_chain"));
    REQUIRE(call.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
    REQUIRE(call.action(ping, MAVAddress("192.1"), rc) == Action::ACCEPT);
    REQUIRE(call.action(ping, MAVAddress("192.2"), rc) == Action::ACCEPT);
    REQUIRE(call.action(ping, MAVAddress("192.3"), rc) == Action::ACCEPT);
    REQUIRE(call.action(ping, MAVAddress("192.4"), rc) == Action::REJECT);
    REQUIRE(call.action(ping, MAVAddress("192.5"), rc) == Action::REJECT);
    REQUIRE(call.action(ping, MAVAddress("192.6"), rc) == Action::REJECT);
    REQUIRE(call.action(ping, MAVAddress("192.7"), rc) == Action::REJECT);
    REQUIRE(call.action(hb, MAVAddress("192.0"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.1"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.2"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.3"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.4"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.5"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.6"), rc) == Action::CONTINUE);
    REQUIRE(call.action(hb, MAVAddress("192.7"), rc) == Action::CONTINUE);
}


TEST_CASE("Call's are printable.", "[Call]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    Call call(std::make_shared<ChainTestClass>("test_chain"));
    Action &action = call;
    SECTION("By direct type.")
    {
        REQUIRE(str(call) == "call test_chain");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(action) == "call test_chain");
    }
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (Call).", "[Call]")
{
    ChainTestClass *chain = nullptr;
    REQUIRE_NOTHROW(chain = new ChainTestClass("test_chain"));
    REQUIRE_NOTHROW(delete chain);
    Call *call = nullptr;
    REQUIRE_NOTHROW(
        call = new Call(std::make_shared<ChainTestClass>("test_chain")));
    REQUIRE_NOTHROW(delete call);
}
