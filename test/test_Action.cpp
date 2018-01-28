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


#include <sstream>

#include <catch.hpp>

#include "util.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Action.hpp"

#include "common_Packet.hpp"


namespace
{

    class ActionTestClass : public Action
    {
        protected:
            virtual std::ostream &print_(std::ostream &os) const
            {
                os << "test";
                return os;
            }

        public:
            virtual bool action(const Packet &packet, const MAVAddress &address)
            {
                return (packet.name() == "PING") &&
                    MAVSubnet("192.0/14").contains(address);
            }
    };

}


TEST_CASE("Action's can be constructed.", "[Action]")
{
    REQUIRE_NOTHROW(ActionTestClass());
}


TEST_CASE("Action's determine what to do with a packet with respect to a "
          "destination address.", "[Action]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    auto heartbeat = packet_v1::Packet(to_vector(HeartbeatV1()), conn);
    ActionTestClass action;
    REQUIRE(action.action(ping, MAVAddress("192.0")));
    REQUIRE(action.action(ping, MAVAddress("192.1")));
    REQUIRE(action.action(ping, MAVAddress("192.2")));
    REQUIRE(action.action(ping, MAVAddress("192.3")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.4")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.5")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.6")));
    REQUIRE_FALSE(action.action(ping, MAVAddress("192.7")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.0")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.1")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.2")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.3")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.4")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.5")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.6")));
    REQUIRE_FALSE(action.action(heartbeat, MAVAddress("192.7")));
}


TEST_CASE("Action's are printable.")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto ping = packet_v2::Packet(to_vector(PingV2()), conn);
    ActionTestClass action;
    Action &polymophic_action = action;
    SECTION("By direct type.")
    {
        REQUIRE(str(action) == "test");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(polymophic_action) == "test");
    }
}
