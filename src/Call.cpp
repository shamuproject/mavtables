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

#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "Action.hpp"
#include "Chain.hpp"
#include "Call.hpp"


/** Construct a call action given a chain to delegate to.
 *
 *  \param chain The chain to delegate decisions of whether to accept or reject
 *      a packet to.
 *  \throws std::invalid_argument if the given pointer is nullptr.
 */
Call::Call(std::shared_ptr<Chain> chain)
    : chain_(std::move(chain))
{
    if (chain_ == nullptr)
    {
        throw std::invalid_argument("Given Chain pointer is null.");
    }
}


/** \copydoc Action::print_(std::ostream &os) const
 *
 *  Prints `"call <Chain Name>"`.
 */
std::ostream &Call::print_(std::ostream &os) const
{
    os << "call " << chain_->name;
    return os;
}


//! \copydoc Action::clone() const
std::unique_ptr<Action> Call::clone() const
{
    return std::make_unique<Call>(chain_);
}


/** \copydoc Action::action(const Packet&,const MAVAddress&,RecursionChecker&)const
 *
 *  The Call class delegates the action choice to the contained \ref Chain.  If
 *  that action is \ref Action::CONTINUE then \ref Rule evaluation should
 *  continue on the next \ref Rule in the current \ref Chain.
 */
Action::Option Call::action(
    Packet &packet, const MAVAddress &address,
    RecursionChecker &recursion_checker) const
{
    return chain_->action(packet, address, recursion_checker);
}
