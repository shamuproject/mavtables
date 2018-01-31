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


#include <catch.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "RecursionChecker.hpp"
#include "Action.hpp"
#include "Reject.hpp"

#include "common_Packet.hpp"


TEST_CASE("Reject's can be constructed.", "[Reject]")
{
    REQUIRE_NOTHROW(Reject());
}


TEST_CASE("Reject's 'action' method always returns Action::REJECT.", "[Reject]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto hb = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    RecursionChecker rc;
    Reject reject;
    REQUIRE(reject.action(ping, MAVAddress("192.0"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.1"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.2"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.3"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.4"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.5"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.6"), rc) == Action::REJECT);
    REQUIRE(reject.action(ping, MAVAddress("192.7"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.0"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.1"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.2"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.3"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.4"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.5"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.6"), rc) == Action::REJECT);
    REQUIRE(reject.action(hb, MAVAddress("192.7"), rc) == Action::REJECT);
}


TEST_CASE("Reject's are printable.", "[Reject]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    Reject reject;
    Action &action = reject;
    SECTION("By direct type.")
    {
        REQUIRE(str(reject) == "reject");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(reject) == "reject");
    }
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (Reject).", "[Reject]")
{
    Reject *reject = nullptr;
    REQUIRE_NOTHROW(reject = new Reject());
    REQUIRE_NOTHROW(delete reject);
}
