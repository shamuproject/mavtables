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
#include <stdexcept>

#include <catch.hpp>

#include "Action.hpp"
#include "ActionResult.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "util.hpp"

#include "ChainTestClass.hpp"
#include "common_Packet.hpp"


TEST_CASE("Call's can be constructed.", "[Call]")
{
    REQUIRE_NOTHROW(Call(std::make_shared<ChainTestClass>("test_chain")));
    REQUIRE_NOTHROW(Call(std::make_shared<ChainTestClass>("test_chain"), 3));
    SECTION("Ensures the shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(Call(nullptr), std::invalid_argument);
        REQUIRE_THROWS_AS(Call(nullptr, 3), std::invalid_argument);
        REQUIRE_THROWS_WITH(Call(nullptr), "Given Chain pointer is null.");
        REQUIRE_THROWS_WITH(Call(nullptr, 3), "Given Chain pointer is null.");
    }
}


TEST_CASE("Call's are comparable.", "[Call]")
{
    auto chain1 = std::make_shared<ChainTestClass>("test_chain_1");
    auto chain2 = std::make_shared<ChainTestClass>("test_chain_2");
    SECTION("with ==")
    {
        REQUIRE(Call(chain1) == Call(chain1));
        REQUIRE(Call(chain2) == Call(chain2));
        REQUIRE(Call(chain1, 3) == Call(chain1, 3));
        REQUIRE_FALSE(Call(chain1) == Call(chain2));
        REQUIRE_FALSE(Call(chain1) == Call(chain1, 3));
    }
    SECTION("with !=")
    {
        REQUIRE(Call(chain1) != Call(chain2));
        REQUIRE(Call(chain1) != Call(chain1, 3));
        REQUIRE_FALSE(Call(chain1) != Call(chain1));
        REQUIRE_FALSE(Call(chain2) != Call(chain2));
        REQUIRE_FALSE(Call(chain1, 3) != Call(chain1, 3));
    }
}


TEST_CASE("Call's 'action' method delegates the decision to the Chain it "
          "contains.", "[Call]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    SECTION("Without a priority.")
    {
        Call call(std::make_shared<ChainTestClass>("test_chain"));
        REQUIRE(
            call.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            call.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            call.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            call.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            call.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_continue());
    }
    SECTION("With a priority.")
    {
        Call call(std::make_shared<ChainTestClass>("test_chain"), 3);
        REQUIRE(
            call.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            call.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            call.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            call.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            call.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_continue());
        REQUIRE(
            call.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_continue());
    }
}


TEST_CASE("Call's are printable.", "[Call]")
{
    SECTION("Without priority.")
    {
        Call call(std::make_shared<ChainTestClass>("test_chain"));
        Action &action = call;
        REQUIRE(str(call) == "call test_chain");
        REQUIRE(str(action) == "call test_chain");
    }
    SECTION("With priority.")
    {
        Call call(std::make_shared<ChainTestClass>("test_chain"), -3);
        Action &action = call;
        REQUIRE(str(call) == "call test_chain with priority -3");
        REQUIRE(str(action) == "call test_chain with priority -3");
    }
}


TEST_CASE("Call's 'clone' method returns a polymorphic copy.", "[Call]")
{
    Call call(std::make_shared<ChainTestClass>("test_chain"));
    Action &action = call;
    std::unique_ptr<Action> polymorphic_copy = action.clone();
    REQUIRE(call == *polymorphic_copy);
}


// // Required for complete function coverage.
// TEST_CASE("Run dynamic destructors (Call).", "[Call]")
// {
//     ChainTestClass *chain = nullptr;
//     REQUIRE_NOTHROW(chain = new ChainTestClass("test_chain"));
//     REQUIRE_NOTHROW(delete chain);
//     Call *call = nullptr;
//     REQUIRE_NOTHROW(
//         call = new Call(std::make_shared<ChainTestClass>("test_chain")));
//     REQUIRE_NOTHROW(delete call);
// }
