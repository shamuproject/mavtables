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


#include <memory>
#include <vector>

#include <catch.hpp>

#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"


#include "common_Packet.hpp"


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Subclass of Interface used for testing the abstract class Interface.
    class InterfaceTestClass : public Interface
    {
        protected:
            virtual void tx_()
            {
                ++tx_count;
            }
            virtual std::unique_ptr<Packet> rx_()
            {
                ++rx_count;
                packets.push_back(
                    std::make_unique<packet_v2::Packet>(to_vector(PingV2())));
                return std::make_unique<packet_v2::Packet>(to_vector(PingV2()));
            }
        public:
            unsigned int tx_count = 0;
            unsigned int rx_count = 0;
            std::vector<std::unique_ptr<const Packet>> packets;
            using Interface::Interface;
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}



TEST_CASE("Interface's can be constructed.", "[Interface]")
{

    REQUIRE_NOTHROW(
        InterfaceTestClass(
            std::make_shared<ConnectionPool>(), Interface::DELAY_START));
    InterfaceTestClass interface(
            std::make_shared<ConnectionPool>());
    // std::thread::
    // interface.shutdown();
    // REQUIRE_NOTHROW(
    //     InterfaceTestClass(
    //         std::make_shared<ConnectionPool>()));
    // InterfaceTestClass interface(std::make_shared<ConnectionPool>());
    // interface.

}
