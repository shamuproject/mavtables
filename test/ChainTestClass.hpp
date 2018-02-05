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


#include "Action.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"
#include "Packet.hpp"
#include "RecursionChecker.hpp"


namespace
{

    class ChainTestClass : public Chain
    {
        public:
            using Chain::Chain;
            virtual Action::Option action(
                Packet &packet, const MAVAddress &address,
                RecursionChecker &recursion_checker) const
            {
                (void)recursion_checker;

                if (packet.id() == 4)
                {
                    if (MAVSubnet("192.0/14").contains(address))
                    {
                        return Action::ACCEPT;
                    }

                    return Action::REJECT;
                }

                return Action::CONTINUE;
            }
    };

}
