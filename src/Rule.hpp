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

#include "Action.hpp"
#include "Chain.hpp"
#include "Conditional.hpp"
#include "Packet.hpp"


/** A rule used in filter \ref Chains.
 *
 * Rules are used to determine an \ref Action to take with a packet based on its
 * type, source address, and destination address.  The are very much like the
 * rules found in typical software defined firewalls.
 */
class Rule
{
    private:
        std::unique_ptr<Action> action_;
        std::optional<Conditional> condition_;
        std::optional<int> priority_;

    public:
        Rule();
        Rule(const Rule &other);
        /** Move constructor.
         *
         * \param other Rule to move from.
         */
        Rule(Rule &&other) = default;
        Rule(std::unique_ptr<Action> action,
             std::optional<Conditional> condition = {});
        Rule &accept();
        Rule &reject();
        Rule &call(std::shared_ptr<Chain> chain);
        Rule &goto_(std::shared_ptr<Chain> chain);
        Rule &with_priority(int priority);
        Conditional &if_();
        Action::Option action(
            Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const;
        Rule &operator=(const Rule &other);
        /** Assignment operator (by move semantics).
         *
         * \param other Rule to move from.
         */
        Rule &operator=(Rule &&other) = default;

        friend bool operator==(const Rule &lhs, const Rule &rhs);
        friend bool operator!=(const Rule &lhs, const Rule &rhs);
        friend std::ostream &operator<<(std::ostream &os, const Rule &rule);

};


bool operator==(const Rule &lhs, const Rule &rhs);
bool operator!=(const Rule &lhs, const Rule &rhs);
std::ostream &operator<<(std::ostream &os, const Rule &rule);


#endif // RULE_HPP_
