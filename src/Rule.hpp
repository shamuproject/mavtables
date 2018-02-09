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


#ifndef RULE_HPP_
#define RULE_HPP_


#include <memory>
#include <optional>
#include <ostream>

#include "Action.hpp"
#include "If.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "RecursionChecker.hpp"


/** Base class of all rules, used in filter \ref Chain's.
 *
 *  Rules are used to determine an \ref Action to take with a packet based on
 *  its type, source address, and destination address.  The are very much like
 *  the rules found in typical software defined firewalls.
 */
class Rule
{
    protected:
        std::optional<If> condition_;
        /** Print the rule to the given output stream.
         *
         *  \param os The output stream to print to.
         *  \return The output stream.
         */
        virtual std::ostream &print_(std::ostream &os) const = 0;

    public:
        Rule(std::optional<If> condition = {});
        virtual ~Rule();  // Clang does not like pure virtual destructors.
        /** Decide what to do with a \ref Packet.
         *
         *  Determine what action to take with the given \p packet sent to the
         *  given \p address.  The possible actions are documented in the \ref
         *  Action class.  The continue object is always returned if the
         *  condition was set and does not match the packet/address combination.
         *
         *  \param packet The packet to determine whether to allow or not.
         *  \param address The address the \p packet will be sent out on if the
         *      action allows it.
         *  \param recursion_checker A recursion checker used to ensure infinite
         *      recursion does not occur.
         *  \returns The action to take with the packet.  If this is the accept
         *      object, it may also contain a priority for the packet.
         */
        virtual Action action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const = 0;
        /** Return a copy of the Rule polymorphically.
         *
         *  This allows Rule's to be copied without knowing the derived type.
         *
         *  \returns A pointer to a new object with base type \ref Rule which
         *      is an exact copy of this one.
         */
        virtual std::unique_ptr<Rule> clone() const = 0;
        /** Equality comparison.
         *
         *  \param other The other rule to compare this to.
         *  \retval true if this rule is the same as \p other.
         *  \retval false if this rule is not the same as \p other.
         */
        virtual bool operator==(const Rule &other) const = 0;
        /** Inequality comparison.
         *
         *  \param other The other rule to compare this to.
         *  \retval true if this rule is not the same as \p other.
         *  \retval false if this rule is the same as \p other.
         */
        virtual bool operator!=(const Rule &other) const = 0;

        friend std::ostream &operator<<(std::ostream &os, const Rule &action);
};


std::ostream &operator<<(std::ostream &os, const Rule &action);


#endif // RULE_HPP_
