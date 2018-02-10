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
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "Action.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include <RecursionGuard.hpp>




Chain::Chain(
    std::string name_, std::vector<std::unique_ptr<const Rule>> rules)
    : name(std::move(name_)), rules_(std::move(rules))
{
    if (name.find_first_of("\t\n ") != std::string::npos)
    {
        throw std::invalid_argument("Chain names cannot contain whitespace.");
    }
}


Action Chain::action(
    const Packet &packet, const MAVAddress &address)
{
    // Prevent recursion.
    RecursionGuard recursion_guard(recursion_data_);

    // Loop throught the rules.
    for (auto const &rule : rules_)
    {
        auto result = rule->action(packet, address);

        // Return rule result if not CONTINUE.
        if (result.action != Action::CONTINUE)
        {
            return result;
        }
    }

    return Action::make_continue();
}


void Chain::append(std::unique_ptr<const Rule> rule)
{
    rules_.push_back(std::move(rule));
}


std::ostream &operator<<(std::ostream &os, const Chain &chain)
{
    os << "chain " << chain.name << " {" << std::endl;

    for (auto const &rule : chain.rules_)
    {
        os << *rule << ";" << std::endl;
    }

    os << ";" << std::endl;
    return os;
}
