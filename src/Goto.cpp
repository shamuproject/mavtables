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


#include <ostream>

#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "Action.hpp"
#include "Chain.hpp"
#include "Goto.hpp"


/** Construct a goto action given a chain to delegate to.
 *
 *  \param chain The chain to delegate decisions of whether to accept or reject
 *      a packet to.
 */
Goto::Goto(std::shared_ptr<Chain> chain)
    : chain_(std::move(chain))
{
}


/** \copydoc Action::print_(std::ostream &os) const
 *
 *  Prints `"goto <Chain Name>"`.
 */
std::ostream &Goto::print_(std::ostream &os) const
{
    os << "goto " << chain_->name;
    return os;
}


/** \copydoc Action::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  The Goto class delegates the action choice to the contained \ref Chain.  If
 *  the \ref Chain decides on the \ref Action::CONTINUE action this method will
 *  return \ref Action::DEFAULT instead since final decision for a \ref Goto
 *  should be with the contained \ref Chain or with the default action.  In
 *  other words, once a rule with a \ref Goto matches, no further rule in the
 *  chain should ever be ran.
 */
Action::Option Goto::action(
    const Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    Action::Option option = chain_->action(packet, address, recursion_checker);
    if (option == Action::CONTINUE)
    {
        return Action::DEFAULT;
    }
    return option;
}
