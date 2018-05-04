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


#ifndef CHAIN_HPP_
#define CHAIN_HPP_


#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Action.hpp"
#include "config.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "RecursionGuard.hpp"
#include "Rule.hpp"


/** A filter chain, containing a list of rules to check packets against.
 */
class Chain
{
    public:
        Chain(const Chain &other);
        /** Move constructor.
         *
         *  \param other Chain to move from.
         */
        Chain(Chain &&other) = default;
        Chain(std::string name_,
              std::vector<std::unique_ptr<Rule>> &&rules = {});
        TEST_VIRTUAL ~Chain() = default;
        TEST_VIRTUAL Action action(
            const Packet &packet, const MAVAddress &address);
        void append(std::unique_ptr<Rule> rule);
        const std::string &name() const;
        Chain &operator=(const Chain &other);
        /** Assignment operator (by move semantics).
         *
         * \param other Chain to move from.
         */
        Chain &operator=(Chain &&other) = default;

        friend bool operator==(const Chain &lhs, const Chain &rhs);
        friend bool operator!=(const Chain &lhs, const Chain &rhs);
        friend std::ostream &operator<<(std::ostream &os, const Chain &chain);

    private:
        std::string name_;
        std::vector<std::unique_ptr<Rule>> rules_;
        RecursionData recursion_data_;
};


bool operator==(const Chain &lhs, const Chain &rhs);
bool operator!=(const Chain &lhs, const Chain &rhs);
std::ostream &operator<<(std::ostream &os, const Chain &chain);


#endif // CHAIN_HPP_
