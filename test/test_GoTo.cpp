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
#include "Chain.hpp"
#include "GoTo.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "util.hpp"

#include "ChainTestClass.hpp"
#include "common_Packet.hpp"


TEST_CASE("GoTo's can be constructed.", "[GoTo]")
{
    REQUIRE_NOTHROW(GoTo(std::make_shared<ChainTestClass>("test_chain")));
    REQUIRE_NOTHROW(GoTo(std::make_shared<ChainTestClass>("test_chain"), 3));
    SECTION("Ensures the shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(GoTo(nullptr), std::invalid_argument);
        REQUIRE_THROWS_AS(GoTo(nullptr, 3), std::invalid_argument);
        REQUIRE_THROWS_WITH(GoTo(nullptr), "Given Chain pointer is null.");
        REQUIRE_THROWS_WITH(GoTo(nullptr, 3), "Given Chain pointer is null.");
    }
}


TEST_CASE("GoTo's are comparable.", "[GoTo]")
{
    auto chain1 = std::make_shared<ChainTestClass>("test_chain_1");
    auto chain2 = std::make_shared<ChainTestClass>("test_chain_2");
    SECTION("with ==")
    {
        REQUIRE(GoTo(chain1) == GoTo(chain1));
        REQUIRE(GoTo(chain2) == GoTo(chain2));
        REQUIRE(GoTo(chain1, 3) == GoTo(chain1, 3));
        REQUIRE_FALSE(GoTo(chain1) == GoTo(chain2));
        REQUIRE_FALSE(GoTo(chain1) == GoTo(chain1, 3));
    }
    SECTION("with !=")
    {
        REQUIRE(GoTo(chain1) != GoTo(chain2));
        REQUIRE(GoTo(chain1) != GoTo(chain1, 3));
        REQUIRE_FALSE(GoTo(chain1) != GoTo(chain1));
        REQUIRE_FALSE(GoTo(chain2) != GoTo(chain2));
        REQUIRE_FALSE(GoTo(chain1, 3) != GoTo(chain1, 3));
    }
}


TEST_CASE("GoTo's 'action' method delegates the decision to the Chain it "
          "contains.", "[GoTo]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    SECTION("Without a priority.")
    {
        GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
        REQUIRE(
            goto_.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_default());
    }
    SECTION("With a priority.")
    {
        GoTo goto_(std::make_shared<ChainTestClass>("test_chain"), 3);
        REQUIRE(
            goto_.action(ping, MAVAddress("192.0"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            goto_.action(ping, MAVAddress("192.1"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            goto_.action(ping, MAVAddress("192.2"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            goto_.action(ping, MAVAddress("192.3"), rc) ==
            ActionResult::make_accept(3));
        REQUIRE(
            goto_.action(ping, MAVAddress("192.4"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.5"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.6"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(ping, MAVAddress("192.7"), rc) ==
            ActionResult::make_reject());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.0"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.1"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.2"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.3"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.4"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.5"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.6"), rc) ==
            ActionResult::make_default());
        REQUIRE(
            goto_.action(set_mode, MAVAddress("192.7"), rc) ==
            ActionResult::make_default());
    }
}


TEST_CASE("GoTo's are printable.", "[GoTo]")
{
    SECTION("Without priority.")
    {
        GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
        Action &action = goto_;
        REQUIRE(str(goto_) == "goto test_chain");
        REQUIRE(str(action) == "goto test_chain");
    }
    SECTION("With priority.")
    {
        GoTo goto_(std::make_shared<ChainTestClass>("test_chain"), -3);
        Action &action = goto_;
        REQUIRE(str(goto_) == "goto test_chain with priority -3");
        REQUIRE(str(action) == "goto test_chain with priority -3");
    }
}


TEST_CASE("GoTo's 'clone' method returns a polymorphic copy.", "[GoTo]")
{
    GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
    Action &action = goto_;
    std::unique_ptr<Action> polymorphic_copy = action.clone();
    REQUIRE(goto_ == *polymorphic_copy);
}


// // Required for complete function coverage.
// TEST_CASE("Run dynamic destructors (GoTo).", "[GoTo]")
// {
//     ChainTestClass *chain = nullptr;
//     REQUIRE_NOTHROW(chain = new ChainTestClass("test_chain"));
//     REQUIRE_NOTHROW(delete chain);
//     GoTo *goto_ = nullptr;
//     REQUIRE_NOTHROW(
//         goto_ = new GoTo(std::make_shared<ChainTestClass>("test_chain")));
//     REQUIRE_NOTHROW(delete goto_);
// }
