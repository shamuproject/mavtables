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


#include <chrono>
#include <memory>

#include <catch.hpp>
#include <fakeit.hpp>

#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"

#include "common.hpp"
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
        private:
            std::shared_ptr<ConnectionPool> connection_pool_;

        public:
            InterfaceTestClass(std::shared_ptr<ConnectionPool> connection_pool)
                : connection_pool_(std::move(connection_pool))
            {
            }
            // LCOV_EXCL_START
            ~InterfaceTestClass() = default;
            // LCOV_EXCL_STOP
            void send_packet(
                const std::chrono::nanoseconds &timeout =
                    std::chrono::nanoseconds(100000)) final
            {
                (void)timeout;
            }
            void receive_packet(
                const std::chrono::nanoseconds &timeout =
                    std::chrono::nanoseconds(100000)) final
            {
                (void)timeout;
                connection_pool_->send(
                    std::make_unique<packet_v2::Packet>(to_vector(PingV2())));
            }
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}


TEST_CASE("Interface's can be constructed.", "[Interface]")
{
    fakeit::Mock<ConnectionPool> mock_pool;
    std::shared_ptr<ConnectionPool> pool = mock_shared(mock_pool);
    REQUIRE_NOTHROW(InterfaceTestClass(pool));
}


TEST_CASE("Interface's 'send_packet' method (included just for coverage)."
          "[Interface]")
{
    fakeit::Mock<ConnectionPool> mock_pool;
    std::shared_ptr<ConnectionPool> pool = mock_shared(mock_pool);
    InterfaceTestClass interface(pool);
    REQUIRE_NOTHROW(interface.send_packet());
}


TEST_CASE("Interface's 'receive_packet' method sends the packet using the "
          "contained ConnectionPool.", "[Interface]")
{
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Fake(Method(mock_pool, send));
    std::shared_ptr<ConnectionPool> pool = mock_shared(mock_pool);
    InterfaceTestClass interface(pool);
    interface.receive_packet();
    fakeit::Verify(Method(mock_pool, send).Matching([](auto &&a)
    {
        return a != nullptr && *a == packet_v2::Packet(to_vector(PingV2()));
    })).Once();
}
