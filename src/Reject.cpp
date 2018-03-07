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
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "Reject.hpp"
#include "Rule.hpp"


/** Construct a reject rule.
 *
 *  A reject rule is used to reject packet/address combinations that match the
 *  condition of the rule.
 *
 *  \param condition The condition used to determine the rule matches a
 *      particular packet/address combination given to the \ref action method.
 *      The default is {} which indicates the rule matches any packet/address
 *      combination.
 *  \sa action
 */
Reject::Reject(std::optional<If> condition) : Rule(std::move(condition)) {}


/** \copydoc Rule::print_(std::ostream &os) const
 *
 *  Prints `"reject"` or `"reject <If Statement>"` if the rule's condition was
 *  set.
 */
std::ostream &Reject::print_(std::ostream &os) const
{
    os << "reject";

    if (condition_)
    {
        os << " " << condition_.value();
    }

    return os;
}


/** \copydoc Rule::action(const Packet&,const MAVAddress&)const
 *
 *  %If the condition has not been set or it matches the given packet/address
 *  combination then it will return the reject object, otherwise it will return
 *  the continue object.
 */
Action Reject::action(const Packet &packet, const MAVAddress &address) const
{
    if (!condition_ || condition_->check(packet, address))
    {
        return Action::make_reject();
    }

    return Action::make_continue();
}


std::unique_ptr<Rule> Reject::clone() const
{
    return std::make_unique<Reject>(condition_);
}


bool Reject::operator==(const Rule &other) const
{
    return typeid(*this) == typeid(other) &&
           condition_ == static_cast<const Reject &>(other).condition_;
}


bool Reject::operator!=(const Rule &other) const
{
    return typeid(*this) != typeid(other) ||
           condition_ != static_cast<const Reject &>(other).condition_;
}
