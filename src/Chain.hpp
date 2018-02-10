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
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "Action.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include <RecursionGuard.hpp>
#include "Rule.hpp"


/** A filter chain, containing a list of rules to check packets against.
 */
class Chain
{
    public:
        /** The name of the chain.
         *
         *  \note This is only used when printing the chain.
         */
        const std::string name;

    private:
        std::vector<std::unique_ptr<const Rule>> rules_;
        RecursionData recursion_data_;

    public:
        Chain(std::string name_,
              std::vector<std::unique_ptr<const Rule>> rules = {});
        virtual ~Chain() = default;
        virtual Action action(
            const Packet &packet, const MAVAddress &address);
        void append(std::unique_ptr<const Rule> rule);

        friend std::ostream &operator<<(std::ostream &os, const Chain &chain);
};


std::ostream &operator<<(std::ostream &os, const Chain &chain);


#endif // CHAIN_HPP_
