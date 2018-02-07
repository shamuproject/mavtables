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


#include <ostream>

#include <catch.hpp>

#include "Action.hpp"
#include "ActionResult.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "util.hpp"

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
            virtual std::unique_ptr<Action> clone() const
            {
                return std::make_unique<ActionTestClass>();
            }
            virtual ActionResult action(
                Packet &packet, const MAVAddress &address,
                RecursionChecker &recursion_checker) const
            {
                (void)recursion_checker;

                if (packet.name() == "PING")
                {
                    if (MAVSubnet("192.0/14").contains(address))
                    {
                        return ActionResult::make_accept();
                    }

                    return ActionResult::make_reject();
                }

                return ActionResult::make_continue();
            }
            virtual bool operator==(const Action &other) const
            {
                (void)other;
                return true;
            }
            virtual bool operator!=(const Action &other) const
            {
                (void)other;
                return false;
            }
    };

}


TEST_CASE("Action's can be constructed.", "[Action]")
{
    REQUIRE_NOTHROW(ActionTestClass());
}


TEST_CASE("Action's are comparable.", "[Action]")
{
    REQUIRE(ActionTestClass() == ActionTestClass());
    REQUIRE_FALSE(ActionTestClass() != ActionTestClass());
}


TEST_CASE("Action's determine what to do with a packet with respect to a "
          "destination address.", "[Action]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    ActionTestClass action;
    REQUIRE(
        action.action(ping, MAVAddress("192.0"), rc) ==
        ActionResult::make_accept());
    REQUIRE(
        action.action(ping, MAVAddress("192.1"), rc) ==
        ActionResult::make_accept());
    REQUIRE(
        action.action(ping, MAVAddress("192.2"), rc) ==
        ActionResult::make_accept());
    REQUIRE(
        action.action(ping, MAVAddress("192.3"), rc) ==
        ActionResult::make_accept());
    REQUIRE(
        action.action(ping, MAVAddress("192.4"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        action.action(ping, MAVAddress("192.5"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        action.action(ping, MAVAddress("192.6"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        action.action(ping, MAVAddress("192.7"), rc) ==
        ActionResult::make_reject());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.0"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.1"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.2"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.3"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.4"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.5"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.6"), rc) ==
        ActionResult::make_continue());
    REQUIRE(
        action.action(set_mode, MAVAddress("192.7"), rc) ==
        ActionResult::make_continue());
}


TEST_CASE("Action's are printable.", "[Action]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    ActionTestClass action;
    Action &polymorphic = action;
    SECTION("By direct type.")
    {
        REQUIRE(str(action) == "test");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(polymorphic) == "test");
    }
}


TEST_CASE("Action's 'clone' method returns a polymorphic copy.", "[Action]")
{
    ActionTestClass action;
    Action &polymorphic = action;
    std::unique_ptr<Action> polymorphic_copy = polymorphic.clone();
    REQUIRE(action == *polymorphic_copy);
}
