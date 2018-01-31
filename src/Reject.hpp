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


#ifndef REJECT_HPP_
#define REJECT_HPP_


#include <ostream>

#include "Action.hpp"
#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "Action.hpp"


/** Action to reject a packet.
 */
class Reject : public Action
{
    protected:
        virtual std::ostream &print_(std::ostream &os) const;

    public:
        virtual Action::Option action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recusion_checker) const;
};


#endif // REJECT_HPP_