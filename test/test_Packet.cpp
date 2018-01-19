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


#include <catch.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>


#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "Connection.hpp"


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

// Subclass of Packet use for testing the abstract class Connection.
class ConnectionTestClass : public Connection
{
};

// Subclass of Packet used for testing the abstract class Packet.
class PacketTestClass : public Packet
{
    public:
        PacketTestClass(const PacketTestClass &other) = default;
        PacketTestClass(std::vector<uint8_t> data, std::weak_ptr<Connection> connection,
                        int priority = 0)
            : Packet(std::move(data), std::move(connection), priority)
        {
        }
        virtual unsigned int version() const
        {
            return 0x3E; // 3.14
        }
        virtual unsigned long id() const
        {
            return 42;
        }
        virtual std::string name() const
        {
            return "MISSION_CURRENT";
        }
        virtual MAVAddress source() const
        {
            return MAVAddress("3.14");
        }
        virtual std::optional<MAVAddress> dest() const
        {
            return MAVAddress("2.71");
        }
        PacketTestClass &operator=(const PacketTestClass &other) = default;
};

#ifdef __clang__
    #pragma clang diagnostic pop
#endif


TEST_CASE("Packet's can be constructed.", "[Packet]")
{
    std::vector<uint8_t> data = {1, 3, 3, 7};
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE_NOTHROW(PacketTestClass(data, conn));
    REQUIRE_NOTHROW(PacketTestClass(data, conn, -10));
    REQUIRE_NOTHROW(PacketTestClass(data, conn, +10));
}


