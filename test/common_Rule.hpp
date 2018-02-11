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
#include "Packet.hpp"


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif
    class TestChain : public Chain
    {
        public:
            TestChain()
                : Chain("test_chain")
            {
            }
            // LCOV_EXCL_START
            ~TestChain() = default;
            // LCOV_EXCL_STOP
            virtual Action action(
                const Packet &packet, const MAVAddress &address)
            {
                (void)packet;
                (void)address;
                return Action::make_accept();
            }
    };
#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}
