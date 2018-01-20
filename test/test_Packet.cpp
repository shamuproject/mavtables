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
#include "util.hpp"


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

// Subclass of Packet used for testing the abstract class Connection.
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
            return 0x030E; // 3.14
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


TEST_CASE("Packet's contain raw packet data and make it accessible.",
          "[Packet]")
{
    std::vector<uint8_t> data = {1, 3, 3, 7};
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(PacketTestClass(data, conn).data() == std::vector<uint8_t>({1, 3, 3, 7}));
    REQUIRE_FALSE(PacketTestClass(data, conn).data() == std::vector<uint8_t>({1, 0, 5, 3}));
}


TEST_CASE("Packet's contain a weak_ptr to a connection.", "[Packet]")
{
    std::vector<uint8_t> data = {1, 3, 3, 7};
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    std::weak_ptr<ConnectionTestClass> empty_conn; // empty weak pointer
    REQUIRE(PacketTestClass(data, conn1).connection().lock() == conn1);
    REQUIRE(PacketTestClass(data, conn2).connection().lock() == conn2);
    REQUIRE(PacketTestClass(data,
                            empty_conn).connection().lock() == empty_conn.lock());
    REQUIRE(PacketTestClass(data, empty_conn).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketTestClass(data, conn1).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketTestClass(data, conn2).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketTestClass(data, conn1).connection().lock() == conn2);
    REQUIRE_FALSE(PacketTestClass(data, conn2).connection().lock() == conn1);
    REQUIRE_FALSE(PacketTestClass(data, empty_conn).connection().lock() == conn1);
    REQUIRE_FALSE(PacketTestClass(data, empty_conn).connection().lock() == conn2);
}


TEST_CASE("Packets's are copyable.", "[Packet]")
{
    std::vector<uint8_t> data1 = {1, 3, 3, 7};
    std::vector<uint8_t> data2 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    PacketTestClass a(data1, conn1, 1);
    PacketTestClass b(data2, conn2, 2);
    PacketTestClass a_copy = a;
    PacketTestClass b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a.data() == a_copy.data());
    REQUIRE(a.connection().lock() == a_copy.connection().lock());
    REQUIRE(a.priority() == a_copy.priority());
    REQUIRE(&b != &b_copy);
    REQUIRE(b.data() == b_copy.data());
    REQUIRE(b.connection().lock() == b_copy.connection().lock());
    REQUIRE(b.priority() == b_copy.priority());
}


TEST_CASE("Packet's are assignable.", "[Packet]")
{
    std::vector<uint8_t> data = {1, 3, 3, 7};
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketTestClass packet(data, conn, -10);
    REQUIRE(packet.data() == data);
    REQUIRE(packet.connection().lock() == conn);
    REQUIRE(packet.priority() == -10);
    packet = PacketTestClass({}, std::weak_ptr<ConnectionTestClass>(), 10);
    REQUIRE(packet.data() == std::vector<uint8_t>());
    REQUIRE(packet.connection().lock() == nullptr);
    REQUIRE(packet.priority() == 10);
}


TEST_CASE("Packet's have a version.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(PacketTestClass({}, conn).version() == 0x030E /* 3.14 */);
}


TEST_CASE("Packet's have an ID.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(PacketTestClass({}, conn).id() == 42);
}


TEST_CASE("Packet's have a name.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(PacketTestClass({}, conn).name() == "MISSION_CURRENT");
}


TEST_CASE("Packet's have a source address.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(PacketTestClass({}, conn).source() == MAVAddress("3.14"));
}


TEST_CASE("Packet's optionally have a destination address.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(PacketTestClass({}, conn).dest().value() == MAVAddress("2.71"));
}


TEST_CASE("Packet's have a priority.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    SECTION("Which has a default value of 0.")
    {
        REQUIRE(PacketTestClass({}, conn).priority() == 0);
    }
    SECTION("That can be set during construction.")
    {
        REQUIRE(PacketTestClass({}, conn, -32768).priority() == -32768);
        REQUIRE(PacketTestClass({}, conn, -100).priority() == -100);
        REQUIRE(PacketTestClass({}, conn, -10).priority() == -10);
        REQUIRE(PacketTestClass({}, conn, -5).priority() == -5);
        REQUIRE(PacketTestClass({}, conn, -1).priority() == -1);
        REQUIRE(PacketTestClass({}, conn, 0).priority() == 0);
        REQUIRE(PacketTestClass({}, conn, 1).priority() == 1);
        REQUIRE(PacketTestClass({}, conn, 5).priority() == 5);
        REQUIRE(PacketTestClass({}, conn, 10).priority() == 10);
        REQUIRE(PacketTestClass({}, conn, 100).priority() == 100);
        REQUIRE(PacketTestClass({}, conn, 32767).priority() == 32767);
    }
    SECTION("That can be set after construction.")
    {
        PacketTestClass packet({}, conn);
        REQUIRE(packet.priority() == 0);
        // -32768
        REQUIRE(packet.priority(-32768) == -32768);
        REQUIRE(packet.priority() == -32768);
        // -100
        REQUIRE(packet.priority(-100) == -100);
        REQUIRE(packet.priority() == -100);
        // -10
        REQUIRE(packet.priority(-10) == -10);
        REQUIRE(packet.priority() == -10);
        // -5
        REQUIRE(packet.priority(-5) == -5);
        REQUIRE(packet.priority() == -5);
        // -1
        REQUIRE(packet.priority(-1) == -1);
        REQUIRE(packet.priority() == -1);
        // 0
        REQUIRE(packet.priority(0) == 0);
        REQUIRE(packet.priority() == 0);
        // 1
        REQUIRE(packet.priority(1) == 1);
        REQUIRE(packet.priority() == 1);
        // 5
        REQUIRE(packet.priority(5) == 5);
        REQUIRE(packet.priority() == 5);
        // 10
        REQUIRE(packet.priority(10) == 10);
        REQUIRE(packet.priority() == 10);
        // 100
        REQUIRE(packet.priority(100) == 100);
        REQUIRE(packet.priority() == 100);
        // 32767
        REQUIRE(packet.priority(32767) == 32767);
        REQUIRE(packet.priority() == 32767);
    }
}


TEST_CASE("Packet's are printable.", "[Packet]")
{
    std::weak_ptr<ConnectionTestClass> conn;
    REQUIRE(str(PacketTestClass({}, conn)) ==
            "MISSION_CURRENT (#42) from 3.14 to 2.71 (v3.14)");
    REQUIRE(str(PacketTestClass({}, conn,
                                0)) == "MISSION_CURRENT (#42) from 3.14 to 2.71 (v3.14)");
    REQUIRE(str(PacketTestClass({}, conn,
                                -32768)) ==
            "MISSION_CURRENT (#42) from 3.14 to 2.71 with priority -32768 (v3.14)");
    REQUIRE(str(PacketTestClass({}, conn,
                                32767)) ==
            "MISSION_CURRENT (#42) from 3.14 to 2.71 with priority 32767 (v3.14)");
}
