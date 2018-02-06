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

#include "Action.hpp"
#include "ActionResult.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "Reject.hpp"


/** \copydoc Action::print_(std::ostream &os) const
 *
 *  Prints `"reject"`.
 */
std::ostream &Reject::print_(std::ostream &os) const
{
    os << "reject";
    return os;
}


std::unique_ptr<Action> Reject::clone() const
{
    return std::make_unique<Reject>();
}


/** \copydoc Action::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  The Reject class always returns a reject object.  Therefore it always
 *  indicates that the \p packet should not be sent to the given \p address.
 */
ActionResult Reject::action(
    Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    (void)packet;
    (void)address;
    (void)recursion_checker;
    return ActionResult::make_reject();
}


bool Reject::operator==(const Action &other) const
{
    return typeid(*this) == typeid(other);
}


bool Reject::operator!=(const Action &other) const
{
    return typeid(*this) != typeid(other);
}
