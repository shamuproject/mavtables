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

#include "Accept.hpp"
#include "Action.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "Rule.hpp"


/** Construct an accept rule, without a priority.
 *
 *  An accept rule is used to accept packet/address combinations that match the
 *  condition of the rule.
 *
 *  \param condition The condition used to determine the rule matches a
 *      particular packet/address combination given to the \ref action method.
 *      The default is {} which indicates the rule matches any packet/address
 *      combination.
 *  \sa action
 */
Accept::Accept(std::optional<If> condition)
    : Rule(std::move(condition))
{
}


/** Construct an accept rule, with a priority.
 *
 *  An accept rule is used to accept packet/address combinations that match the
 *  condition of the rule.
 *
 *  \param condition The condition used to determine the rule matches a
 *      particular packet/address combination given to the \ref action method.
 *      The default is {} which indicates the rule matches any packet/address
 *      combination.
 *  \param priority The priority to accept packets with.  A higher number is
 *      more important and will be routed first.
 *  \sa action
 */
Accept::Accept(int priority, std::optional<If> condition)
    : Rule(std::move(condition)), priority_(priority)
{
}


/** \copydoc Rule::print_(std::ostream &os)const
 *
 *  Prints `"reject <Priority> <If Statement>"` where the priority and if
 *  statement are only printed if the priority or condition is set,
 *  respectively.
 */
std::ostream &Accept::print_(std::ostream &os) const
{
    os << "accept";

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


/** \copydoc Rule::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  If the condition has not been set or it matches the given packet/address
 *  combination then it will return the accept object (with optional priority),
 *  otherwise it will return the continue object.
 */
Action Accept::action(
    const Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    (void)recursion_checker;

    if (!condition_ || condition_->check(packet, address))
    {
        return Action::make_accept(priority_);
    }

    return Action::make_continue();
}


std::unique_ptr<Rule> Accept::clone() const
{
    if (priority_)
    {
        return std::make_unique<Accept>(priority_.value(), condition_);
    }
    return std::make_unique<Accept>(condition_);
}


/** \copydoc Rule::operator==(const Rule&)const
 *
 *  Compares the priority (if set) associated with the rule as well.
 */
bool Accept::operator==(const Rule &other) const
{
    return typeid(*this) == typeid(other) &&
           priority_ == static_cast<const Accept &>(other).priority_ &&
           condition_ == static_cast<const Accept &>(other).condition_;
}


/** \copydoc Rule::operator!=(const Rule&)const
 *
 *  Compares the priority (if set) associated with the rule as well.
 */
bool Accept::operator!=(const Rule &other) const
{
    return typeid(*this) != typeid(other) ||
           priority_ != static_cast<const Accept &>(other).priority_ ||
           condition_ != static_cast<const Accept &>(other).condition_;
}
