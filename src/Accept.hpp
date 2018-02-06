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


#ifndef ACCEPT_HPP_
#define ACCEPT_HPP_


#include <memory>
#include <optional>
#include <ostream>

#include "Action.hpp"
#include "ActionResult.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"


/** Action to accept a packet.
 */
class Accept : public Action
{
    private:
        std::optional<int> priority_;

    protected:
        virtual std::ostream &print_(std::ostream &os) const;

    public:
        Accept(std::optional<int> priority = {});
        virtual std::unique_ptr<Action> clone() const;
        virtual ActionResult action(
            Packet &packet, const MAVAddress &address,
            RecursionChecker &recusion_checker) const;
        virtual bool operator==(const Action &other) const;
        virtual bool operator!=(const Action &other) const;
};


#endif // ACCEPT_HPP_
