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
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionChecker.hpp"
#include "Rule.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


namespace
{

    class RuleTestClass : public Rule
    {
        protected:
            virtual std::ostream &print_(std::ostream &os) const
            {
                os << "test_rule";
                return os;
            }

        public:
            virtual std::unique_ptr<Rule> clone() const
            {
                return std::make_unique<RuleTestClass>();
            }
            virtual Action action(
                const Packet &packet, const MAVAddress &address,
                RecursionChecker &recursion_checker) const
            {
                (void)recursion_checker;

                if (packet.name() == "PING")
                {
                    if (MAVSubnet("192.0/14").contains(address))
                    {
                        return Action::make_accept();
                    }

                    return Action::make_reject();
                }

                return Action::make_continue();
            }
            virtual bool operator==(const Rule &other) const
            {
                (void)other;
                return true;
            }
            virtual bool operator!=(const Rule &other) const
            {
                (void)other;
                return false;
            }
    };

}


TEST_CASE("Rule's can be constructed.", "[Rule]")
{
    REQUIRE_NOTHROW(RuleTestClass());
}


TEST_CASE("Rule's are comparable.", "[Rule]")
{
    REQUIRE(RuleTestClass() == RuleTestClass());
    REQUIRE_FALSE(RuleTestClass() != RuleTestClass());
}


TEST_CASE("Rule's 'action' method determine what to do with a packet withu "
          "respect to a destination address.", "[Rule]")
{
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    RecursionChecker rc;
    RuleTestClass rule;
    REQUIRE(
        rule.action(ping, MAVAddress("192.0"), rc) == Action::make_accept());
    REQUIRE(
        rule.action(ping, MAVAddress("192.1"), rc) == Action::make_accept());
    REQUIRE(
        rule.action(ping, MAVAddress("192.2"), rc) == Action::make_accept());
    REQUIRE(
        rule.action(ping, MAVAddress("192.3"), rc) == Action::make_accept());
    REQUIRE(
        rule.action(ping, MAVAddress("192.4"), rc) == Action::make_reject());
    REQUIRE(
        rule.action(ping, MAVAddress("192.5"), rc) == Action::make_reject());
    REQUIRE(
        rule.action(ping, MAVAddress("192.6"), rc) == Action::make_reject());
    REQUIRE(
        rule.action(ping, MAVAddress("192.7"), rc) == Action::make_reject());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.0"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.1"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.2"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.3"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.4"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.5"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.6"), rc) ==
        Action::make_continue());
    REQUIRE(
        rule.action(set_mode, MAVAddress("192.7"), rc) ==
        Action::make_continue());
}


TEST_CASE("Rule's are printable.", "[Rule]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    RuleTestClass rule;
    Rule &polymorphic = rule;
    SECTION("By direct type.")
    {
        REQUIRE(str(rule) == "test_rule");
    }
    SECTION("By polymorphic type.")
    {
        REQUIRE(str(polymorphic) == "test_rule");
    }
}


TEST_CASE("Rule's 'clone' method returns a polymorphic copy.", "[Rule]")
{
    RuleTestClass rule;
    Rule &polymorphic = rule;
    std::unique_ptr<Rule> polymorphic_copy = polymorphic.clone();
    REQUIRE(rule == *polymorphic_copy);
}
