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
#include "ActionResult.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"


/** Construct an accept action.
 *
 *  \param priority The priority to accept packets with, the default is no
 *      priority.
 */
Accept::Accept(std::optional<int> priority)
    : priority_(std::move(priority))
{
}


/** \copydoc Action::print_(std::ostream &os) const
 *
 *  Prints `"accept"` or `"accept with priority <priority>"` if the priority is
 *  given.
 */
std::ostream &Accept::print_(std::ostream &os) const
{
    os << "accept";

    if (priority_)
    {
        os << " with priority " << priority_.value();
    }

    return os;
}


std::unique_ptr<Action> Accept::clone() const
{
    return std::make_unique<Accept>();
}


/** \copydoc Action::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  The Accept class always returns an accept action result.  Therefore it
 *  always indicates that the \p packet should be sent to the given \p address.
 *  If the priorty was set during construction the accept result will have a
 *  priority.
 */
ActionResult Accept::action(
    Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    (void)packet;
    (void)address;
    (void)recursion_checker;
    return ActionResult::make_accept(priority_);
}


bool Accept::operator==(const Action &other) const
{
    return typeid(*this) == typeid(other) &&
           priority_ == static_cast<const Accept &>(other).priority_;
}


bool Accept::operator!=(const Action &other) const
{
    return typeid(*this) != typeid(other) ||
           priority_ != static_cast<const Accept &>(other).priority_;
}
