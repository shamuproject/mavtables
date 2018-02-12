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

#include <catch.hpp>
#include <fakeit.hpp>

#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "PacketVersion2.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


TEST_CASE("ConnectionPool's can be constructed.", "[ConnectionPool]")
{
    REQUIRE_NOTHROW(ConnectionPool());
}


TEST_CASE("ConnectionPool's 'add' method ensures the given connection is not "
          "nullptr.", "[ConnectionPool]")
{
    REQUIRE_THROWS_AS(ConnectionPool().add(nullptr), std::invalid_argument);
    REQUIRE_THROWS_WITH(
        ConnectionPool().add(nullptr), "Given Connection pointer is null.");
}


TEST_CASE("ConnectionPool's can store at least one connection and send a "
          "packet over it.", "[ConnectionPool]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    fakeit::Mock<Connection<>> mock;
    fakeit::Fake(Method(mock, send));
    std::shared_ptr<Connection<>> connection = mock_shared(mock.get());
    ConnectionPool pool;
    pool.add(connection);
    pool.send(packet);
    fakeit::Verify(Method(mock, send).Matching([&](auto a)
    {
        return *a == *packet;
    })).Once();
}


TEST_CASE("ConnectionPool's can store more than one connection and send a "
          "packet over them.", "[ConnectionPool]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    fakeit::Mock<Connection<>> mock1;
    fakeit::Mock<Connection<>> mock2;
    fakeit::Fake(Method(mock1, send));
    fakeit::Fake(Method(mock2, send));
    std::shared_ptr<Connection<>> connection1 = mock_shared(mock1.get());
    std::shared_ptr<Connection<>> connection2 = mock_shared(mock2.get());
    ConnectionPool pool;
    pool.add(connection1);
    pool.add(connection2);
    pool.send(packet);
    fakeit::Verify(Method(mock1, send).Matching([&](auto a)
    {
        return *a == *packet;
    })).Once();
    fakeit::Verify(Method(mock2, send).Matching([&](auto a)
    {
        return *a == *packet;
    })).Once();
}


TEST_CASE("ConnectionPool's 'remove' method removes a connection.",
          "[ConnectionPool]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    fakeit::Mock<Connection<>> mock1;
    fakeit::Mock<Connection<>> mock2;
    fakeit::Fake(Method(mock1, send));
    fakeit::Fake(Method(mock2, send));
    std::shared_ptr<Connection<>> connection1 = mock_shared(mock1.get());
    std::shared_ptr<Connection<>> connection2 = mock_shared(mock2.get());
    ConnectionPool pool;
    pool.add(connection1);
    pool.add(connection2);
    pool.send(packet);
    pool.remove(connection1);
    pool.send(packet);
    fakeit::Verify(Method(mock1, send).Matching([&](auto a)
    {
        return *a == *packet;
    })).Once();
    fakeit::Verify(Method(mock2, send).Matching([&](auto a)
    {
        return *a == *packet;
    })).Exactly(2);
}
