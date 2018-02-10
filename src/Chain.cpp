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


/** Construct a new filter chain.
 *
 *  \note No rule in the chain may contain a \ref GoTo or \ref Call that would
 *  directly or indirectly result in returning to this chain.
 *
 *  \param name_ The name of the filter chain.  This is only used when printing
 *      the chain.  The name cannot contain whitespace.
 *  \param rules A vector of the rules used in the filter chain.
 *  \throws std::invalid_argument if the name contains whitespace.
 */
Chain::Chain(
    std::string name_, std::vector<std::unique_ptr<const Rule>> rules)
    : name(std::move(name_)), rules_(std::move(rules))
{
    if (name.find_first_of("\t\n ") != std::string::npos)
    {
        throw std::invalid_argument("Chain names cannot contain whitespace.");
    }
}


/** Decide what to do with a \ref Packet.
 *
 *  Determine what action to take with the given \p packet sent to the
 *  given \p address.  The possible actions are documented in the \ref
 *  Action class.
 *
 *  The filter chain will loop through all the rules and the first one that
 *  matches and returns something other than the continue action will be taken
 *  as the result.
 *
 *  \note An error will be thrown if any \ref Call or \ref GoTo rule matches
 *      that directly or indirectly loops back to this chain.
 *
 *  \param packet The packet to determine whether to allow or not.
 *  \param address The address the \p packet will be sent out on if the
 *      action allows it.
 *  \returns The action to take with the packet.  If this is the accept
 *      object, it may also contain a priority for the packet.
 *  \throws RecursionError if a rule loops back to this chain.
 */
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


/** Append a new rule to the filter chain.
 *
 *  \param rule A new filter rule to append to the chain.
 */
void Chain::append(std::unique_ptr<const Rule> rule)
{
    rules_.push_back(std::move(rule));
}


/** Print the given filter chain to to the given output stream.
 *
 *  An example is:
 *  ```
 *  chain ap-in {
 *      accept if GPS_STATUS to 255.200;
 *      accept if GLOBAL_POSITION_INT to 255.200;
 *      reject if to 255.100;
 *      accept;
 *  }
 *  ```
 */
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
