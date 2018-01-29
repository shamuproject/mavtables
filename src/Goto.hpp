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
#include <ostream>

#include "Action.hpp"
#include "Packet.hpp"
#include "MAVAddress.hpp"


class Chain;


class Goto : public Action
{
    private:
        std::shared_ptr<Chain> chain_;

    protected:
        virtual std::ostream &print_(std::ostream &os) const;

    public:
        Goto(std::shared_ptr<Chain> chain);
        virtual Action::Option action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const;
};


#endif // GOTO_HPP_
