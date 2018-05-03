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


#include <cstddef>
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
#include "RecursionGuard.hpp"


/** Copy constructor.
 *
 *  \param other Chain to copy from.
 */
Chain::Chain(const Chain &other)
    : name_(other.name_)
{
    for (auto &rule : other.rules_)
    {
        rules_.push_back(rule->clone());
    }
}


/** Construct a new filter chain.
 *
 *  \note No rule in the chain may contain a \ref GoTo or \ref Call that would
 *  directly or indirectly result in returning to this chain.
 *
 *  \param name The name of the filter chain.  This is only used when printing
 *      the chain.  The name cannot contain whitespace.
 *  \param rules A vector of the rules used in the filter chain.  This must be
 *      moved from since the vector is made up of std::unique_ptr's.
 *  \throws std::invalid_argument if the \p name contains whitespace.
 */
Chain::Chain(
    std::string name, std::vector<std::unique_ptr<Rule>> &&rules)
    : name_(std::move(name)), rules_(std::move(rules))
{
    if (name_.find_first_of("\t\n ") != std::string::npos)
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
 *  matches and returns something other than the continue \ref Action will be
 *  taken as the result.
 *
 *  \note An error will be thrown if any \ref Call or \ref GoTo rule matches
 *      that directly or indirectly loops back to this chain.
 *
 *  \param packet The packet to determine whether to allow or not.
 *  \param address The address the \p packet will be sent out on if the
 *      action allows it.
 *  \returns The action to take with the packet.  %If this is the accept \ref
 *      Action object, it may also contain a priority for the packet.
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
        if (result.action() != Action::CONTINUE)
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
void Chain::append(std::unique_ptr<Rule> rule)
{
    rules_.push_back(std::move(rule));
}


/** Return the name of the chain.
 *
 *  \note This is only used when printing the chain.
 *
 *  \returns The chain's name.
 */
const std::string &Chain::name() const
{
    return name_;
}


/** Assignment operator.
 *
 * \param other Chain to copy from.
 */
Chain &Chain::operator=(const Chain &other)
{
    name_ = other.name_;
    rules_.clear();

    for (auto &rule : other.rules_)
    {
        rules_.push_back(rule->clone());
    }

    recursion_data_ = other.recursion_data_;
    return *this;
}


/** Equality comparison.
 *
 *  Compares the chain name and each \ref Rule in the chain.
 *
 *  \relates Chain
 *  \param lhs The left hand side filter chain.
 *  \param rhs The right hand side filter chain.
 *  \retval true if \p lhs is the same as rhs.
 *  \retval false if \p lhs is not the same as rhs.
 */
bool operator==(const Chain &lhs, const Chain &rhs)
{
    // Compare names.
    if (lhs.name() != rhs.name())
    {
        return false;
    }

    // Compare number of rules.
    if (lhs.rules_.size() != rhs.rules_.size())
    {
        return false;
    }

    // Compare rules one by one.
    for (size_t i = 0; i < lhs.rules_.size(); ++i)
    {
        if (*(lhs.rules_[i]) != *(rhs.rules_[i]))
        {
            return false;
        }
    }

    return true;
}


/** Inequality comparison.
 *
 *  Compares the chain name and each \ref Rule in the chain.
 *
 *  \relates Chain
 *  \param lhs The left hand side action.
 *  \param rhs The right hand side action.
 *  \retval true if \p lhs is not the same as rhs.
 *  \retval false if \p lhs is the same as rhs.
 */
bool operator!=(const Chain &lhs, const Chain &rhs)
{
    return !(lhs == rhs);
}


/** Print the given filter chain to to the given output stream.
 *
 *  An example is:
 *  ```
 *  chain default {
 *      reject if HEARTBEAT from 10.10;
 *      accept with priority -3 if GPS_STATUS to 172.0/8;
 *      accept if GLOBAL_POSITION_INT to 172.0/8;
 *      goto ap-in with priority 3 if from 192.168;
 *      call ap-out if to 192.168;
 *      reject;
 *  }
 *  ```
 *
 *  \relates Chain
 *  \param os The output stream to print to.
 *  \param chain The filter chain to print.
 *  \returns The output stream.
 */
std::ostream &operator<<(std::ostream &os, const Chain &chain)
{
    os << "chain " << chain.name() << " {" << std::endl;

    for (auto const &rule : chain.rules_)
    {
        os << "    " << *rule << ";" << std::endl;
    }

    os << "}";
    return os;
}
