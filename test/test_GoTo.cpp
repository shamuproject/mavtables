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
#include "GoTo.hpp"

#include "common_Packet.hpp"


namespace
{

    class ChainTestClass : public Chain
    {
        public:
            using Chain::Chain;
            virtual Action::Option action(
                const Packet &packet, const MAVAddress &address,
                RecursionChecker &recursion_checker) const
            {
                (void)recursion_checker;

                if (packet.name() == "PING")
                {
                    if (MAVSubnet("192.0/14").contains(address))
                    {
                        return Action::ACCEPT;
                    }

                    return Action::REJECT;
                }

                return Action::CONTINUE;
            }
    };

}


TEST_CASE("GoTo's can be constructed.", "[GoTo]")
{
    REQUIRE_NOTHROW(GoTo(std::make_shared<ChainTestClass>("test_chain")));
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
