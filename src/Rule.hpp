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


#ifndef ACTION_HPP_
#define ACTION_HPP_


#include <memory>
#include <ostream>

#include "RuleResult.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "RecursionChecker.hpp"


/** Base class of all rule actions.
 *
 *  These are used as the actions of a \ref Rule to determine what to do with a
 *  \ref Packet if it matches the \ref Rule.
 */
class Rule
{
    protected:
        /** Print the action to the given output stream.
         *
         *  \param os The output stream to print to.
         *  \return The output stream.
         */
        virtual std::ostream &print_(std::ostream &os) const = 0;

    public:
        virtual ~Rule();  // Clang does not like pure virtual destructors.
        /** Return a copy of the Rule polymorphically.
         *
         *  This allows Rule's to be copied without knowing the derived type.
         *
         *  \returns A pointer to a new object with base type \ref Rule which
         *      is an exact copy of this one.
         */
        virtual std::unique_ptr<Rule> clone() const = 0;
        /** Decide what to do with a \ref Packet.
         *
         *  Determine what action to take with the given \p packet sent to the
         *  given \p address.  The possible actions are documented in the \ref
         *  RuleResult.
         *
         *  \param packet The packet to determine whether to allow or not.
         *  \param address The address the \p packet will be sent out on if the
         *      action allows it.
         *  \param recursion_checker A recursion checker used to ensure infinite
         *      recursion does not occur.
         *  \returns The action to take with the packet.  If this is the accept
         *      object, it may also contain a priority for the packet.
         */
        virtual RuleResult action(
            Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const = 0;
        /** Equality comparison.
         *
         *  \param other The other action  to compare this to.
         *  \retval true if this action is the same as \p other.
         *  \retval false if this action is not the same as \p other.
         */
        virtual bool operator==(const Rule &other) const = 0;
        /** Inequality comparison.
         *
         *  \param other The other action  to compare this to.
         *  \retval true if this action is not the same as \p other.
         *  \retval false if this action is the same as \p other.
         */
        virtual bool operator!=(const Rule &other) const = 0;

        friend std::ostream &operator<<(std::ostream &os, const Rule &action);
};


std::ostream &operator<<(std::ostream &os, const Rule &action);


#endif // ACTION_HPP_
