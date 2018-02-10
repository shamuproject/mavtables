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


#include <vector>
#include <memory>
#include <utility>

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
#include "Reject.hpp"
#include "Rule.hpp"

#include "common_Packet.hpp"


TEST_CASE("Chain's are constructable.", "[Chain]")
{
    SECTION("Without rules.")
    {
        REQUIRE_NOTHROW(Chain("test_chain"));
    }
    SECTION("With rules.")
    {
        std::vector<std::unique_ptr<const Rule>> rules;
        rules.push_back(std::make_unique<Accept>(If().type("PING")));
        rules.push_back(std::make_unique<Reject>());
        REQUIRE_NOTHROW(Chain("test_chain", std::move(rules)));
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
        std::vector<std::unique_ptr<const Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<const Rule>> rules2;
        rules2.push_back(std::make_unique<Accept>(If().to("192.168")));
        Chain chain2("test_chain", std::move(rules2));
        REQUIRE_FALSE(chain1 == chain2);
        REQUIRE(chain1 != chain2);
    }
    SECTION("When the rules are the same.")
    {
        std::vector<std::unique_ptr<const Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<const Rule>> rules2;
        rules2.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules2.push_back(std::make_unique<Reject>());
        Chain chain2("test_chain", std::move(rules2));
        REQUIRE(chain1 == chain2);
        REQUIRE_FALSE(chain1 != chain2);
    }
    SECTION("When the rules are not the same.")
    {
        std::vector<std::unique_ptr<const Rule>> rules1;
        rules1.push_back(std::make_unique<Accept>(If().to("192.168")));
        rules1.push_back(std::make_unique<Reject>());
        Chain chain1("test_chain", std::move(rules1));
        std::vector<std::unique_ptr<const Rule>> rules2;
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
    std::vector<std::unique_ptr<const Rule>> rules;
    rules.push_back(std::make_unique<Accept>(If().to("192.168")));
    rules.push_back(std::make_unique<Reject>());
    Chain chain1("test_chain", std::move(rules));
    Chain chain2("test_chain");
    chain2.append(std::make_unique<Accept>(If().to("192.168")));
    chain2.append(std::make_unique<Reject>());
    REQUIRE(chain1 == chain2);
}


TEST_CASE("Chain's are copyable.", "[Chain]")
{
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
