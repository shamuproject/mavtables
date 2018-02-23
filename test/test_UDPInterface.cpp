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
#include <stdexcept>

#include <catch.hpp>

#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
#include "UDPInterface.hpp"
#include "UDPSocket.hpp"

#include "common.hpp"


TEST_CASE("UDPInterface's can be constructed.", "[UPDInterface]")
{
    fakeit::Mock<UDPSocket> mock_socket;
    fakeit::Mock<ConnectionPool> mock_pool;
    fakeit::Mock<ConnectionFactory<>> mock_factory;
    auto socket = mock_unique(mock_socket);
    auto pool = mock_shared(mock_pool);
    auto factory = mock_unique(mock_factory);
    SECTION("When all inputs are valid.")
    {
        REQUIRE_NOTHROW(
            UDPInterface(std::move(socket), pool, std::move(factory)));
    }
    SECTION("Ensures the socket pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(nullptr, pool, std::move(factory)),
            std::invalid_argument);
        factory = mock_unique(mock_factory);
        REQUIRE_THROWS_WITH(
            UDPInterface(nullptr, pool, std::move(factory)),
            "Given socket pointer is null.");
    }
    SECTION("Ensures the connection pool pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(std::move(socket), nullptr, std::move(factory)),
            std::invalid_argument);
        socket = mock_unique(mock_socket);
        factory = mock_unique(mock_factory);
        REQUIRE_THROWS_WITH(
            UDPInterface(std::move(socket), nullptr, std::move(factory)),
            "Given connection pool pointer is null.");
    }
    SECTION("Ensures the connection factory pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            UDPInterface(std::move(socket), pool, nullptr),
            std::invalid_argument);
        socket = mock_unique(mock_socket);
        REQUIRE_THROWS_WITH(
            UDPInterface(std::move(socket), pool, nullptr),
            "Given connection factory pointer is null.");
    }
}
