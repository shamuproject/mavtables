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


#ifndef GOTO_HPP_
#define GOTO_HPP_


#include <memory>
#include <optional>
#include <ostream>

#include "Action.hpp"
#include "If.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "Rule.hpp"


// Forward declaration of the chain class.
class Chain;


/** Delegate decision on a packet to another \ref Chain.
 *
 *  Rule to delegate the decision on what to do with a packet to a filter \ref
 *  Chain.  In particular, final decision is given to this \ref Chain.  %If the
 *  \ref Chain cannot decide what to do with the \ref Packet the global default
 *  action should be taken.
 */
class GoTo : public Rule
{
    public:
        GoTo(std::shared_ptr<Chain> chain,
             std::optional<If> condition = {});
        GoTo(std::shared_ptr<Chain> chain, int priority,
             std::optional<If> condition = {});
        virtual Action action(
            const Packet &packet, const MAVAddress &address) const;
        virtual std::unique_ptr<Rule> clone() const;
        virtual bool operator==(const Rule &other) const;
        virtual bool operator!=(const Rule &other) const;

    protected:
        virtual std::ostream &print_(std::ostream &os) const;

    private:
        std::shared_ptr<Chain> chain_;
        std::optional<int> priority_;
};


#endif // GOTO_HPP_
