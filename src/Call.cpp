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
#include <optional>
#include <ostream>
#include <typeinfo>
#include <utility>

#include "Action.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "Rule.hpp"


/** Construct a call rule given a chain to delegate to, without a priority.
 *
 *  A call rule is used to delegate the decision on whether to accept or reject
 *  a packet/address combination to another filter \ref Chain.
 *
 *  \param chain The chain to delegate decisions of whether to accept or reject
 *      a packet/address combination to.  null is not valid.
 *  \param condition The condition used to determine the rule matches a
 *      particular packet/address combination given to the \ref action method.
 *      The default is {} which indicates the rule matches any packet/address
 *      combination.
 *  \throws std::invalid_argument if the given pointer is nullptr.
 *  \sa action
 */
Call::Call(
    std::shared_ptr<Chain> chain, std::optional<If> condition)
    : Rule(std::move(condition)), chain_(std::move(chain))
{
    if (chain_ == nullptr)
    {
        throw std::invalid_argument("Given Chain pointer is null.");
    }
}


/** Construct a call rule given a chain to delegate to, with a priority.
 *
 *  A call rule is used to delegate the decision on whether to accept or reject
 *  a packet/address combination to another filter \ref Chain.
 *
 *  \param chain The chain to delegate decisions of whether to accept or reject
 *      a packet/address combination to. null is not valid.
 *  \param priority The priority to accept packets with.  A higher number is
 *      more important and will be routed first.
 *  \param condition The condition used to determine the rule matches a
 *      particular packet/address combination given to the \ref action method.
 *      The default is {} which indicates the rule matches any packet/address
 *      combination.
 *  \throws std::invalid_argument if the given pointer is nullptr.
 *  \sa action
 */
Call::Call(
    std::shared_ptr<Chain> chain, int priority, std::optional<If> condition)
    : Rule(std::move(condition)), chain_(std::move(chain)), priority_(priority)
{
    if (chain_ == nullptr)
    {
        throw std::invalid_argument("Given Chain pointer is null.");
    }
}


/** \copydoc Rule::print_(std::ostream &os)const
 *
 *  Prints `"call <Chain Name> <If Statement>"` or `"call <Chain Name> with
 *  priority <If Statement> with priority <priority>"` if the priority is
 *  given.
 */
std::ostream &Call::print_(std::ostream &os) const
{
    os << "call " << chain_->name;

    if (priority_)
    {
        os << " with priority " << priority_.value();
    }

    if (condition_)
    {
        os << " " << condition_.value();
    }

    return os;
}


/** \copydoc Rule::action(const Packet&,const MAVAddress&)const
 *
 *  If the condition has not been set or it matches the given packet/address
 *  combination then the choice of \ref Action will be delegated to the
 *  contained \ref Chain.
 *
 *  If the result from the chain is an accept object and no priority has been
 *  set on it but this \ref Rule has a priority then the priority will be set.
 */
Action Call::action(
    const Packet &packet, const MAVAddress &address) const
{
    if (!condition_ || condition_->check(packet, address))
    {
        auto result = chain_->action(packet, address);

        if (priority_)
        {
            // Only has an effect if the action is accept and does not already
            // have a priority.
            result.priority(priority_.value());
        }

        return result;
    }

    return Action::make_continue();
}


std::unique_ptr<Rule> Call::clone() const
{
    if (priority_)
    {
        return std::make_unique<Call>(chain_, priority_.value(), condition_);
    }

    return std::make_unique<Call>(chain_, condition_);
}


/** \copydoc Rule::operator==(const Rule&)const
 *
 *  Compares the chain and priority (if set) associated with the rule as well.
 */
bool Call::operator==(const Rule &other) const
{
    return typeid(*this) == typeid(other) &&
           chain_ == static_cast<const Call &>(other).chain_ &&
           priority_ == static_cast<const Call &>(other).priority_ &&
           condition_ == static_cast<const Call &>(other).condition_;
}


/** \copydoc Rule::operator!=(const Rule&)const
 *
 *  Compares the chain and priority (if set) associated with the rule as well.
 */
bool Call::operator!=(const Rule &other) const
{
    return typeid(*this) != typeid(other) ||
           chain_ != static_cast<const Call &>(other).chain_ ||
           priority_ != static_cast<const Call &>(other).priority_ ||
           condition_ != static_cast<const Call &>(other).condition_;
}
