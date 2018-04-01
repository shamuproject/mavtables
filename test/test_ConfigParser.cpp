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


#include <iostream>

#include <catch.hpp>
#include <fakeit.hpp>
#include <pegtl.hpp>

#include "parse_tree.hpp"
#include "config_grammar.hpp"
#include "ConfigParser.hpp"
#include "PacketVersion2.hpp"
#include "util.hpp"
#include "MAVAddress.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


TEST_CASE("'init_chains' initializes a map of chain names to empty chains"
          "[ConfigParser]")
{
    SECTION("default chain listed first")
    {
        tao::pegtl::string_input<> in(
            "chain default {}\n"
            "chain first_chain {}\n"
            "chain second_chain {}\n"
            "chain third_chain {}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto chains = init_chains(*root);
        REQUIRE(chains.count("default") == 0);
        REQUIRE(chains.count("first_chain") == 1);
        REQUIRE(chains.count("second_chain") == 1);
        REQUIRE(chains.count("third_chain") == 1);
        REQUIRE(chains["first_chain"] != nullptr);
        REQUIRE(chains["second_chain"] != nullptr);
        REQUIRE(chains["third_chain"] != nullptr);
        REQUIRE(
            str(*chains["first_chain"]) ==
            "chain first_chain {\n"
            "}");
        REQUIRE(
            str(*chains["second_chain"]) ==
            "chain second_chain {\n"
            "}");
        REQUIRE(
            str(*chains["third_chain"]) ==
            "chain third_chain {\n"
            "}");
    }
    SECTION("default chain listed last")
    {
        tao::pegtl::string_input<> in(
            "chain first_chain {}\n"
            "chain second_chain {}\n"
            "chain third_chain {}\n"
            "chain default {}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto chains = init_chains(*root);
        REQUIRE(chains.count("default") == 0);
        REQUIRE(chains.count("first_chain") == 1);
        REQUIRE(chains.count("second_chain") == 1);
        REQUIRE(chains.count("third_chain") == 1);
        REQUIRE(chains["first_chain"] != nullptr);
        REQUIRE(chains["second_chain"] != nullptr);
        REQUIRE(chains["third_chain"] != nullptr);
        REQUIRE(
            str(*chains["first_chain"]) ==
            "chain first_chain {\n"
            "}");
        REQUIRE(
            str(*chains["second_chain"]) ==
            "chain second_chain {\n"
            "}");
        REQUIRE(
            str(*chains["third_chain"]) ==
            "chain third_chain {\n"
            "}");
    }
}


TEST_CASE("'parse_action' parses an action from the given AST node.", 
          "[ConfigParser]")
{
    std::map<std::string, std::shared_ptr<Chain>> chains;
    chains["some_chain"] = std::make_shared<Chain>("some_chain");
    SECTION("Accept action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {}, {}, chains);
        REQUIRE(str(*action) == "accept");
    }
    SECTION("Accept action, with priority.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority -1;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1, {}, chains);
        REQUIRE(str(*action) == "accept with priority -1");
    }
    SECTION("Accept action, with condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {},
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(str(*action) == "accept if PING from 192.168 to 127.0/8");
    }
    SECTION("Accept action, with priority and condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority -1 if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1,
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) ==
            "accept with priority -1 if PING from 192.168 to 127.0/8");
    }
    SECTION("Reject action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {}, {}, chains);
        REQUIRE(str(*action) == "reject");
    }
    SECTION("Reject action, with priority (priority ignored for reject).")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority -1;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1, {}, chains);
        REQUIRE(str(*action) == "reject");
    }
    SECTION("Reject action, with condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {},
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(str(*action) == "reject if PING from 192.168 to 127.0/8");
    }
    SECTION("Reject action, with priority and condition "
            "(priority ignored for reject).")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1,
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) ==
            "reject if PING from 192.168 to 127.0/8");
    }
    SECTION("Call action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_chain;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {}, {}, chains);
        REQUIRE(str(*action) == "call some_chain");
    }
    SECTION("Call action, throws and error if calling the default chain.")
    {
        tao::pegtl::string_input<> in(
            "chain some_chain {\n"
            "    call default;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_THROWS_AS(
            parse_action(*root->children[0]->children[0], {}, {}, chains),
            std::invalid_argument);
        REQUIRE_THROWS_WITH(
            parse_action(*root->children[0]->children[0], {}, {}, chains),
            "cannot 'call' the default chain");
    }
    SECTION("Call action, with priority.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_chain with priority -1;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1, {}, chains);
        REQUIRE(str(*action) == "call some_chain with priority -1");
    }
    SECTION("Call action, with condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_chain if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {},
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) == "call some_chain if PING from 192.168 to 127.0/8");
    }
    SECTION("Call action, with priority and condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_chain with priority -1 "
            "if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1,
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) ==
            "call some_chain with priority -1 if PING from 192.168 to 127.0/8");
    }
    SECTION("GoTo action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_chain;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {}, {}, chains);
        REQUIRE(str(*action) == "goto some_chain");
    }
    SECTION("GoTo action, throws and error if going to the default chain.")
    {
        tao::pegtl::string_input<> in(
            "chain some_chain {\n"
            "    goto default;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_THROWS_AS(
            parse_action(*root->children[0]->children[0], {}, {}, chains),
            std::invalid_argument);
        REQUIRE_THROWS_WITH(
            parse_action(*root->children[0]->children[0], {}, {}, chains),
            "cannot 'goto' the default chain");
    }
    SECTION("GoTo action, with priority.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_chain with priority -1;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1, {}, chains);
        REQUIRE(str(*action) == "goto some_chain with priority -1");
    }
    SECTION("GoTo action, with condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_chain if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], {},
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) == "goto some_chain if PING from 192.168 to 127.0/8");
    }
    SECTION("GoTo action, with priority and condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_chain with priority -1 "
            "if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        auto action = parse_action(
            *root->children[0]->children[0], -1,
            If().type("PING").from("192.168").to("127.0/8"), chains);
        REQUIRE(
            str(*action) ==
            "goto some_chain with priority -1 if PING from 192.168 to 127.0/8");
    }
}


TEST_CASE("'parse_condition' parses a condition from the given AST node.",
          "[ConfigParser]")
{
    SECTION("Condition with type.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if PING");
    }
    SECTION("Condition source.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.168;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if from 192.168");
    }
    SECTION("Condition destination.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 128.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if to 128.0/8");
    }
    SECTION("Condition source and destination.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if from 192.168 to 127.0/8");
    }
    SECTION("Condition type and source.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING from 192.168;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if PING from 192.168");
    }
    SECTION("Condition type and destination.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if PING to 127.0/8");
    }
    SECTION("Condition type, source, and destination.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0] != nullptr);
        REQUIRE_FALSE(root->children[0]->children[0]->children.empty());
        REQUIRE(root->children[0]->children[0]->children[0] != nullptr);
        auto condition = parse_condition(
            *root->children[0]->children[0]->children[0]);
        REQUIRE(str(condition) == "if PING from 192.168 to 127.0/8");
    }
}


TEST_CASE("'parse_chain' parses an action from the given AST node.", 
          "[ConfigParser]")
{
    Chain default_chain("default");
    std::map<std::string, std::shared_ptr<Chain>> chains;
    chains["some_chain"] = std::make_shared<Chain>("some_chain");
    SECTION("Accept action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept;\n"
            "    accept with priority -1;\n"
            "    accept if PING from 192.168 to 127.0/8;\n"
            "    accept with priority -1 if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        parse_chain(default_chain, *root->children[0], chains);
        REQUIRE(str(default_chain) ==
            "chain default {\n"
            "    accept;\n"
            "    accept with priority -1;\n"
            "    accept if PING from 192.168 to 127.0/8;\n"
            "    accept with priority -1 if PING from 192.168 to 127.0/8;\n"
            "}");
    }
    SECTION("Reject action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject;\n"
            "    reject if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        parse_chain(default_chain, *root->children[0], chains);
        REQUIRE(str(default_chain) ==
            "chain default {\n"
            "    reject;\n"
            "    reject if PING from 192.168 to 127.0/8;\n"
            "}");
    }
    SECTION("Call action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_chain;\n"
            "    call some_chain with priority -1;\n"
            "    call some_chain if PING from 192.168 to 127.0/8;\n"
            "    call some_chain with priority -1"
            "if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        parse_chain(default_chain, *root->children[0], chains);
        REQUIRE(str(default_chain) ==
            "chain default {\n"
            "    call some_chain;\n"
            "    call some_chain with priority -1;\n"
            "    call some_chain if PING from 192.168 to 127.0/8;\n"
            "    call some_chain with priority -1 "
            "if PING from 192.168 to 127.0/8;\n"
            "}");
    }
    SECTION("GoTo action.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_chain;\n"
            "    goto some_chain with priority -1;\n"
            "    goto some_chain if PING from 192.168 to 127.0/8;\n"
            "    goto some_chain with priority -1"
            "if PING from 192.168 to 127.0/8;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        parse_chain(default_chain, *root->children[0], chains);
        REQUIRE(str(default_chain) ==
            "chain default {\n"
            "    goto some_chain;\n"
            "    goto some_chain with priority -1;\n"
            "    goto some_chain if PING from 192.168 to 127.0/8;\n"
            "    goto some_chain with priority -1 "
            "if PING from 192.168 to 127.0/8;\n"
            "}");
    }
}


TEST_CASE("'parse_filter' parses the filter from the given AST root node.", 
          "[ConfigParser]")
{
    auto ping = packet_v2::Packet(to_vector(PingV2()));
    auto heartbeat = packet_v2::Packet(to_vector(HeartbeatV2()));
    auto encapsulated_data =
        packet_v2::Packet(to_vector(EncapsulatedDataV2()));
    SECTION("Successful parse.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call first_chain if PING;\n"
            "    goto second_chain if HEARTBEAT;\n"
            "    reject;\n"
            "}\n"
            "\n"
            "chain first_chain {\n"
            "    accept with priority 1;\n"
            "}\n"
            "\n"
            "chain second_chain {\n"
            "    accept with priority 2;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto filter = parse_filter(*root);
        REQUIRE(filter != nullptr);
        auto ping_result = filter->will_accept(ping, MAVAddress("127.1"));
        REQUIRE(ping_result.first == true);
        REQUIRE(ping_result.second == 1);
        auto heartbeat_result =
            filter->will_accept(heartbeat, MAVAddress("127.1"));
        REQUIRE(heartbeat_result.first == true);
        REQUIRE(heartbeat_result.second == 2);
        auto encapsulated_result =
            filter->will_accept(encapsulated_data, MAVAddress("127.1"));
        REQUIRE(encapsulated_result.first == false);
    }
    SECTION("Default filter action is 'accept'")
    {
        tao::pegtl::string_input<> in(
            "default_action accept;\n"
            "chain default {\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto filter = parse_filter(*root);
        REQUIRE(filter != nullptr);
        auto result = filter->will_accept(ping, MAVAddress("127.1"));
        REQUIRE(result.first == true);
        REQUIRE(result.second == 0);
    }
    SECTION("Default filter action is 'reject'")
    {
        tao::pegtl::string_input<> in(
            "default_action reject;\n"
            "chain default {\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto filter = parse_filter(*root);
        REQUIRE(filter != nullptr);
        auto result = filter->will_accept(ping, MAVAddress("127.1"));
        REQUIRE(result.first == false);
    }
    SECTION("The default, default filter action is 'reject'")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        auto filter = parse_filter(*root);
        REQUIRE(filter != nullptr);
        auto result = filter->will_accept(ping, MAVAddress("127.1"));
        REQUIRE(result.first == false);
    }
}


TEST_CASE("'parse_serial' parses a serial interface from a serial interface "
          "AST node.", "[ConfigParser]")
{
    SECTION("With flow control.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "    flow_control yes;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto serial_port =
            parse_serial(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*serial_port) ==
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "    flow_control yes;\n"
            "}");
    }
    SECTION("Without flow control.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "    flow_control no;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto serial_port =
            parse_serial(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*serial_port) ==
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "    flow_control no;\n"
            "}");
    }
    SECTION("Default flow control is off.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto serial_port =
            parse_serial(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*serial_port) ==
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 115200;\n"
            "    flow_control no;\n"
            "}");
    }
    SECTION("Default baud rate is 9600.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device ./ttyS0;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto serial_port =
            parse_serial(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*serial_port) ==
            "serial {\n"
            "    device ./ttyS0;\n"
            "    baudrate 9600;\n"
            "    flow_control no;\n"
            "}");
    }
    SECTION("Throw error if device string is missing.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        REQUIRE_THROWS_AS(
            parse_serial(*root->children[0], filter, connection_pool),
            std::invalid_argument);
        REQUIRE_THROWS_WITH(
            parse_serial(*root->children[0], filter, connection_pool),
            "missing device string");
    }
}


TEST_CASE("'parse_udp' parses a UDP interface from a UDP interface AST node.",
          "[ConfigParser]")
{
    SECTION("Without specific IP address.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port 14500;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto udp_socket =
            parse_udp(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*udp_socket) ==
            "udp {\n"
            "    port 14500;\n"
            "}");
    }
    SECTION("With specific IP address.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port 14500;\n"
            "    address 127.0.0.1;\n"
            "}\n", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE_FALSE(root->children.empty());
        REQUIRE(root->children[0] != nullptr);
        auto filter = std::make_shared<Filter>(Chain("default"));
        auto connection_pool = std::make_shared<ConnectionPool>();
        auto udp_socket =
            parse_udp(*root->children[0], filter, connection_pool);
        REQUIRE(
            str(*udp_socket) ==
            "udp {\n"
            "    port 14500;\n"
            "    address 127.0.0.1;\n"
            "}");
    }
}


TEST_CASE("'parse_interfaces' parses serial port and UDP interfaces from "
          "the root node.", "[ConfigParser]")
{
    tao::pegtl::string_input<> in(
        "serial {\n"
        "    device ./ttyS0;\n"
        "    baudrate 115200;\n"
        "    flow_control yes;\n"
        "}\n"
        "\n"
        "serial {\n"
        "    device ./ttyS1;\n"
        "    baudrate 300;\n"
        "    flow_control no;\n"
        "}\n"
        "\n"
        "udp {\n"
        "    port 14500;\n"
        "}\n"
        "\n"
        "udp {\n"
        "    port 8000;\n"
        "    address 127.0.0.1;\n"
        "}\n", "");
    auto root = config::parse(in);
    REQUIRE(root != nullptr);
    auto filter = std::make_unique<Filter>(Chain("default"));
    auto interfaces = parse_interfaces(*root, std::move(filter));
    REQUIRE(interfaces.size() == 4);
    REQUIRE(interfaces[0] != nullptr);
    REQUIRE(
        str(*interfaces[0]) ==
        "serial {\n"
        "    device ./ttyS0;\n"
        "    baudrate 115200;\n"
        "    flow_control yes;\n"
        "}");
    REQUIRE(interfaces[1] != nullptr);
    REQUIRE(
        str(*interfaces[1]) ==
        "serial {\n"
        "    device ./ttyS1;\n"
        "    baudrate 300;\n"
        "    flow_control no;\n"
        "}");
    REQUIRE(interfaces[2] != nullptr);
    REQUIRE(
        str(*interfaces[2]) ==
        "udp {\n"
        "    port 14500;\n"
        "}");
    REQUIRE(interfaces[3] != nullptr);
    REQUIRE(
        str(*interfaces[3]) ==
        "udp {\n"
        "    port 8000;\n"
        "    address 127.0.0.1;\n"
        "}");
}


TEST_CASE("ConfigParser can parse a file.", "[ConfigParser]")
{
    SECTION("When the file exists and is valid.")
    {
        REQUIRE_NOTHROW(ConfigParser("examples/test.conf"));
    }
    SECTION("Throws an error if the file does not exist.")
    {
        REQUIRE_THROWS(
            ConfigParser("examples/file_that_does_not_exist.conf"));
    }
}


TEST_CASE("ConfigParser are printable.", "[ConfigParser]")
{
    ConfigParser config("examples/test.conf");
    REQUIRE(
        str(config) ==
        "examples/test.conf:001:  default_action\n"
        "examples/test.conf:001:  |  accept\n"
        "examples/test.conf:004:  udp\n"
        "examples/test.conf:005:  |  port 14500\n"
        "examples/test.conf:006:  |  address 127.0.0.1\n"
        "examples/test.conf:010:  serial\n"
        "examples/test.conf:011:  |  device ./ttyS0\n"
        "examples/test.conf:012:  |  baudrate 115200\n"
        "examples/test.conf:013:  |  flow_control yes\n"
        "examples/test.conf:017:  chain default\n"
        "examples/test.conf:019:  |  call some_chain10\n"
        "examples/test.conf:019:  |  |  condition\n"
        "examples/test.conf:019:  |  |  |  source 127.1\n"
        "examples/test.conf:019:  |  |  |  dest 192.0\n"
        "examples/test.conf:020:  |  reject\n"
        "examples/test.conf:024:  chain some_chain10\n"
        "examples/test.conf:026:  |  accept\n"
        "examples/test.conf:026:  |  |  priority 99\n"
        "examples/test.conf:026:  |  |  condition\n"
        "examples/test.conf:026:  |  |  |  dest 192.0\n"
        "examples/test.conf:027:  |  accept\n"
        "examples/test.conf:027:  |  |  condition\n"
        "examples/test.conf:027:  |  |  |  packet_type PING\n"
        "examples/test.conf:028:  |  accept\n");
}

TEST_CASE("ConfigParser's 'make_app' method returns an application object.",
          "[ConfigParser]")
{
    ConfigParser config("examples/test.conf");
    std::unique_ptr<App> app;
    REQUIRE_NOTHROW(app = config.make_app());
    REQUIRE(app != nullptr);
}
