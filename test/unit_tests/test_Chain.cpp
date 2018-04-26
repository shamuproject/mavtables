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
#include <utility>
#include <vector>

#include <catch.hpp>

#include "Accept.hpp"
#include "Action.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "GoTo.hpp"
#include "If.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "RecursionError.hpp"
#include "Reject.hpp"
#include "utility.hpp"

#include "common_Packet.hpp"


TEST_CASE("Chain's are constructable.", "[Chain]")
{
    SECTION("Without rules.")
    {
        REQUIRE_NOTHROW(Chain("test_chain"));
    }
    SECTION("With rules.")
    {
        std::vector<std::unique_ptr<Rule>> rules;
        rules.push_back(std::make_unique<Accept>(If().type("PING")));
        rules.push_back(std::make_unique<Reject>());
        REQUIRE_NOTHROW(Chain("test_chain", std::move(rules)));
    }
    SECTION("Ensures the chain name does not contain whitespace.")
    {
        // Spaces
        REQUIRE_THROWS_AS(Chain("test chain"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Chain("test chain"), "Chain names cannot contain whitespace.");
        // Tabs
        REQUIRE_THROWS_AS(Chain("test\tchain"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Chain("test\tchain"), "Chain names cannot contain whitespace.");
        // Newlines
        REQUIRE_THROWS_AS(Chain("test\nchain"), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            Chain("test\nchain"), "Chain names cannot contain whitespace.");
    }
}


TEST_CASE("Chain's are comparable.", "[Chain]")
{
    SECTION("When there are no rules.")
    {
        REQUIRE(Chain("test_chain") == Chain("test_chain"));
        REQUIRE_FALSE(Chain("test_chain") != Chain("test_chain"));
        REQUIRE_FALSE(Chain("test_chain1") == Chain("test_chain2"));
        REQUIRE(Chain("test_chain1") != Chain("test_chain2"));
    }
    SECTION("When the number of rules are not the same.")
    {
        std::vector<std::unique_ptr<Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<Rule>> rules2;
        rules2.push_back(std::make_unique<Accept>(If().to("192.168")));
        Chain chain2("test_chain", std::move(rules2));
        REQUIRE_FALSE(chain1 == chain2);
        REQUIRE(chain1 != chain2);
    }
    SECTION("When the rules are the same.")
    {
        std::vector<std::unique_ptr<Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<Rule>> rules2;
        rules2.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules2.push_back(std::make_unique<Reject>());
        Chain chain2("test_chain", std::move(rules2));
        REQUIRE(chain1 == chain2);
        REQUIRE_FALSE(chain1 != chain2);
    }
    SECTION("When the rules are not the same.")
    {
        std::vector<std::unique_ptr<Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<Rule>> rules2;
        rules2.push_back(std::make_unique<Accept>(If().from("172.16")));
        rules2.push_back(std::make_unique<Reject>());
        Chain chain2("test_chain", std::move(rules2));
        REQUIRE_FALSE(chain1 == chain2);
        REQUIRE(chain1 != chain2);
    }
}


TEST_CASE("Chain's 'append' method appends a new rule to the filter chain.",
          "[Chain]")
{
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<Accept>(If().to("192.168")));
    rules.push_back(std::make_unique<Reject>());
    Chain chain1("test_chain", std::move(rules));
    Chain chain2("test_chain");
    chain2.append(std::make_unique<Accept>(If().to("192.168")));
    chain2.append(std::make_unique<Reject>());
    REQUIRE(chain1 == chain2);
}


TEST_CASE("Chain's 'name' method returns the name of the chain.", "[Chain]")
{
    REQUIRE(Chain("crazy_chain_name").name() == "crazy_chain_name");
}


TEST_CASE("Chain's are copyable.", "[Chain]")
{
    Chain original("test_chain");
    original.append(std::make_unique<Accept>(If().to("192.168")));
    original.append(std::make_unique<Reject>());
    Chain for_comparison("test_chain");
    for_comparison.append(std::make_unique<Accept>(If().to("192.168")));
    for_comparison.append(std::make_unique<Reject>());
    Chain copy(original);
    REQUIRE(copy == for_comparison);
}


TEST_CASE("Chain's are movable.", "[Chain]")
{
    Chain original("test_chain");
    original.append(std::make_unique<Accept>(If().to("192.168")));
    original.append(std::make_unique<Reject>());
    Chain for_comparison("test_chain");
    for_comparison.append(std::make_unique<Accept>(If().to("192.168")));
    for_comparison.append(std::make_unique<Reject>());
    Chain moved(std::move(original));
    REQUIRE(moved == for_comparison);
}


TEST_CASE("Chain's are assignable.", "[Chain]")
{
    Chain a("test_chain_a");
    a.append(std::make_unique<Accept>(If().to("192.168")));
    a.append(std::make_unique<Reject>());
    Chain a_compare("test_chain_a");
    a_compare.append(std::make_unique<Accept>(If().to("192.168")));
    a_compare.append(std::make_unique<Reject>());
    Chain b("test_chain_b");
    b.append(std::make_unique<Reject>());
    Chain b_compare("test_chain_b");
    b_compare.append(std::make_unique<Reject>());
    REQUIRE(a == a_compare);
    a = b;
    REQUIRE(a == b_compare);
}


TEST_CASE("Chain's are assignable (by move semantics).", "[Chain]")
{
    Chain a("test_chain_a");
    a.append(std::make_unique<Accept>(If().to("192.168")));
    a.append(std::make_unique<Reject>());
    Chain a_compare("test_chain_a");
    a_compare.append(std::make_unique<Accept>(If().to("192.168")));
    a_compare.append(std::make_unique<Reject>());
    Chain b("test_chain_b");
    b.append(std::make_unique<Reject>());
    Chain b_compare("test_chain_b");
    b_compare.append(std::make_unique<Reject>());
    REQUIRE(a == a_compare);
    a = std::move(b);
    REQUIRE(a == b_compare);
}


TEST_CASE("Chain's 'action' method determines what to do with a packet with "
          " respect to a destination address.", "[Rule]")
{
    auto heartbeat = packet_v2::Packet(to_vector(HeartbeatV2()));
    auto ping = packet_v1::Packet(to_vector(PingV1()));
    auto set_mode = packet_v2::Packet(to_vector(SetModeV2()));
    SECTION("When the chain is well formed.")
    {
        auto chain = std::make_shared<Chain>("main_chain");
        auto subchain = std::make_shared<Chain>("sub_chain");
        chain->append(std::make_unique<Accept>(If().to("192.168")));
        chain->append(std::make_unique<Accept>(If().type("HEARTBEAT")));
        chain->append(std::make_unique<Call>(subchain, If().to("172.0/8")));
        chain->append(std::make_unique<Reject>(If().to("10.10")));
        subchain->append(std::make_unique<Accept>(If().type("SET_MODE")));
        subchain->append(std::make_unique<Accept>(If().to("172.16")));
        REQUIRE(
            chain->action(ping, MAVAddress("192.168")) ==
            Action::make_accept());
        REQUIRE(
            chain->action(ping, MAVAddress("192.168")) ==
            Action::make_accept());
        REQUIRE(
            chain->action(heartbeat, MAVAddress("192.0")) ==
            Action::make_accept());
        REQUIRE(
            chain->action(set_mode, MAVAddress("172.0")) ==
            Action::make_accept());
        REQUIRE(
            chain->action(ping, MAVAddress("172.16")) ==
            Action::make_accept());
        REQUIRE(
            chain->action(ping, MAVAddress("10.10")) ==
            Action::make_reject());
        REQUIRE(
            chain->action(ping, MAVAddress("172.0")) ==
            Action::make_continue());
    }
    SECTION("And throws an error when chain recursion is detected.")
    {
        auto chain = std::make_shared<Chain>("main_chain");
        auto subchain = std::make_shared<Chain>("sub_chain");
        chain->append(std::make_unique<Call>(subchain));
        subchain->append(std::make_unique<Call>(chain));
        REQUIRE_THROWS_AS(
            chain->action(ping, MAVAddress("192.168")), RecursionError);
        REQUIRE_THROWS_WITH(
            chain->action(ping, MAVAddress("192.168")), "Recursion detected.");
    }
}


TEST_CASE("Chain's are printable.", "[Chain]")
{
    auto chain = std::make_shared<Chain>("default");
    auto ap_in = std::make_shared<Chain>("ap-in");
    auto ap_out = std::make_shared<Chain>("ap-out");
    chain->append(
        std::make_unique<Reject>(If().type("HEARTBEAT").from("10.10")));
    chain->append(
        std::make_unique<Accept>(-3, If().type("GPS_STATUS").to("172.0/8")));
    chain->append(
        std::make_unique<Accept>(
            If().type("GLOBAL_POSITION_INT").to("172.0/8")));
    chain->append(
        std::make_unique<GoTo>(ap_in, 3, If().from("192.168")));
    chain->append(std::make_unique<Call>(ap_out, If().to("192.168")));
    chain->append(std::make_unique<Reject>());
    REQUIRE(
        str(*chain) ==
        "chain default {\n"
        "    reject if HEARTBEAT from 10.10;\n"
        "    accept with priority -3 if GPS_STATUS to 172.0/8;\n"
        "    accept if GLOBAL_POSITION_INT to 172.0/8;\n"
        "    goto ap-in with priority 3 if from 192.168;\n"
        "    call ap-out if to 192.168;\n"
        "    reject;\n"
        "}");
}


// Required for complete function coverage.
TEST_CASE("Run dynamic destructors (Chain).", "[Chain]")
{
    Chain *chain = nullptr;
    REQUIRE_NOTHROW(chain = new Chain("chain"));
    REQUIRE_NOTHROW(delete chain);
}
