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
#include <utility>

#include "Action.hpp"
#include "Chain.hpp"
#include "Filter.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"


/** Construct a new packet filter.
 *
 *  \param default_chain The \ref Chain that all filtering begins with.
 *  \param accept_by_default Whether to accept (true) or reject (false) packets
 *      that don't match any rules in the default chain or any chains called by
 *      the default chain.  The default value is false and thus to reject
 *      unmatched packets.
 */
Filter::Filter(Chain default_chain, bool accept_by_default)
    : default_chain_(std::move(default_chain)),
      accept_by_default_(accept_by_default)
{
}


/** Determine whether to accept or reject a packet/address combination.
 *
 *  \param packet The packet to determine whether to allow or not.
 *  \param address The address the \p packet will be sent out on if the
 *      action allows it.
 *  \returns A pair (tuple) with the first value being whether to accept hte
 *      packet or not and the second being the priority to use when sending the
 *      packet.  The second value is only defined if the first value is true
 *      (accept).
 */
std::pair<bool, int>
Filter::will_accept(const Packet &packet, const MAVAddress &address)
{
    Action result = default_chain_.action(packet, address);

    switch (result.action())
    {
        case Action::ACCEPT:
            return {true, result.priority()};
        case Action::REJECT:
            return {false, 0};
        case Action::CONTINUE:
            break;

        case Action::DEFAULT:
            break;
    }

    return {accept_by_default_, 0};
}


/** Equality comparison.
 *
 *  \relates Filter
 *  \param lhs The left hand side packet filter.
 *  \param rhs The right hand side packet filter.
 *  \retval true if \p lhs is the same as rhs.
 *  \retval false if \p lhs is not the same as rhs.
 */
bool operator==(const Filter &lhs, const Filter &rhs)
{
    return (lhs.default_chain_ == rhs.default_chain_) &&
           (lhs.accept_by_default_ == rhs.accept_by_default_);
}


/** Inequality comparison.
 *
 *  \relates Filter
 *  \param lhs The left hand side packet filter.
 *  \param rhs The right hand side packet filter.
 *  \retval true if \p lhs is not the same as rhs.
 *  \retval false if \p lhs is the same as rhs.
 */
bool operator!=(const Filter &lhs, const Filter &rhs)
{
    return (lhs.default_chain_ != rhs.default_chain_) ||
           (lhs.accept_by_default_ != rhs.accept_by_default_);
}
