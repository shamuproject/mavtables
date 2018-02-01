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

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "RecursionChecker.hpp"
#include "Chain.hpp"
#include "Action.hpp"
#include "GoTo.hpp"

#include "ChainTestClass.hpp"
#include "common_Packet.hpp"


TEST_CASE("GoTo's can be constructed.", "[GoTo]")
{
    REQUIRE_NOTHROW(GoTo(std::make_shared<ChainTestClass>("test_chain")));
    SECTION("Ensures the shared pointer is not null.")
    {
        REQUIRE_THROWS_AS(GoTo(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(GoTo(nullptr), "Given Chain pointer is null.");
    }
}


TEST_CASE("GoTo's 'action' method delegates the decision to the Chain it "
          "contains.", "[GoTo]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto hb = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
    REQUIRE(goto_.action(ping, MAVAddress("192.0"), rc) == Action::ACCEPT);
    REQUIRE(goto_.action(ping, MAVAddress("192.1"), rc) == Action::ACCEPT);
    REQUIRE(goto_.action(ping, MAVAddress("192.2"), rc) == Action::ACCEPT);
    REQUIRE(goto_.action(ping, MAVAddress("192.3"), rc) == Action::ACCEPT);
    REQUIRE(goto_.action(ping, MAVAddress("192.4"), rc) == Action::REJECT);
    REQUIRE(goto_.action(ping, MAVAddress("192.5"), rc) == Action::REJECT);
    REQUIRE(goto_.action(ping, MAVAddress("192.6"), rc) == Action::REJECT);
    REQUIRE(goto_.action(ping, MAVAddress("192.7"), rc) == Action::REJECT);
    REQUIRE(goto_.action(hb, MAVAddress("192.0"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.1"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.2"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.3"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.4"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.5"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.6"), rc) == Action::DEFAULT);
    REQUIRE(goto_.action(hb, MAVAddress("192.7"), rc) == Action::DEFAULT);
}


TEST_CASE("GoTo's are printable.", "[GoTo]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
    Action &action = goto_;
    SECTION("By direct type.")
    {
        REQUIRE(str(goto_) == "goto test_chain");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(action) == "goto test_chain");
    }
}


TEST_CASE("GoTo's 'clone' method returns a polymorphic copy.", "[GoTo]")
{
    // Note: String comparisons are used because Action's are not comparable.
    GoTo goto_(std::make_shared<ChainTestClass>("test_chain"));
    Action &action = goto_;
    std::unique_ptr<Action> polymorphic_copy = action.clone();
    REQUIRE(str(goto_) == str(*polymorphic_copy));
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (GoTo).", "[GoTo]")
{
    ChainTestClass *chain = nullptr;
    REQUIRE_NOTHROW(chain = new ChainTestClass("test_chain"));
    REQUIRE_NOTHROW(delete chain);
    GoTo *goto_ = nullptr;
    REQUIRE_NOTHROW(
        goto_ = new GoTo(std::make_shared<ChainTestClass>("test_chain")));
    REQUIRE_NOTHROW(delete goto_);
}
