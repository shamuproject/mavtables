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
#include <stdexcept>
#include <utility>

#include "Action.hpp"
#include "ActionResult.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"


/** Construct a call action given a chain to delegate to.
 *
 *  \param chain The chain to delegate decisions of whether to accept or reject
 *      a packet to.
 *  \param priority The priority to accept packets with, the default is no
 *      priority.
 *  \throws std::invalid_argument if the given pointer is nullptr.
 */
Call::Call(std::shared_ptr<Chain> chain, std::optional<int> priority)
    : chain_(std::move(chain)), priority_(std::move(priority))
{
    if (chain_ == nullptr)
    {
        throw std::invalid_argument("Given Chain pointer is null.");
    }
}


/** \copydoc Action::print_(std::ostream &os) const
 *
 *  Prints `"call <Chain Name>"` or `"call <Chain Name> with priority
 *  <priority>"` if the priority is given.
 */
std::ostream &Call::print_(std::ostream &os) const
{
    os << "call " << chain_->name;
    if (priority_)
    {
        os << " with priority " << priority_.value();
    }
    return os;
}


std::unique_ptr<Action> Call::clone() const
{
    return std::make_unique<Call>(chain_);
}


/** \copydoc Action::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  The Call class delegates the action choice to the contained \ref Chain.  If
 *  that action is a continue then \ref Rule evaluation should continue on the
 *  next \ref Rule in the current \ref Chain.
 */
ActionResult Call::action(
    Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    ActionResult result = chain_->action(packet, address, recursion_checker);
    if (priority_)
    {
        result.priority(priority_.value());
    }
    return result;
}


/** \copydoc Action::operator==(const Action &) const
 *
 *  Compares the chain associated with the call as well.
 */
bool Call::operator==(const Action &other) const
{
    return typeid(*this) == typeid(other) &&
        chain_ == static_cast<const Call &>(other).chain_ &&
        priority_ == static_cast<const Call &>(other).priority_;
}


/** \copydoc Action::operator!=(const Action &) const
 *
 *  Compares the chain associated with the call as well.  If a priority was
 *  given it will also be compared.
 */
bool Call::operator!=(const Action &other) const
{
    return typeid(*this) != typeid(other) ||
        chain_ != static_cast<const Call &>(other).chain_ ||
        priority_ != static_cast<const Call &>(other).priority_;
}
