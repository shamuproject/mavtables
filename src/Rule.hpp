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
#include "Packet.hpp"
#include "Conditional.hpp"


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
        bool apply_priority_ = false;
        int priority_;

    public:
        Rule();
        Rule(std::unique_ptr<Action> action,
             std::optional<Conditional> condition = {});
        Rule &accept();
        Rule &reject();
        Rule &call(std::shared_ptr<Chain> chain);
        Rule &goto_(std::shared_ptr<Chain> chain);
        Rule &with_priority(int priority);
        Conditional &if_();
        Action::Option action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const;

        friend std::ostream &operator<<(std::ostream &os, const Rule &rule);

};


std::ostream &operator<<(std::ostream &os, const Rule &rule);


#endif // RULE_HPP_
