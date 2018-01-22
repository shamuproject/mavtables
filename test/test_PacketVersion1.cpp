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
#include <utility>
#include <cstdint>
#include <optional>
#include <stdexcept>

#include "PacketVersion1.hpp"
#include "MAVAddress.hpp"
#include "Connection.hpp"
#include "mavlink.h"
#include "macros.hpp"
#include "util.hpp"


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Subclass of Packet used for testing the abstract class Connection.
    class ConnectionTestClass : public Connection
    {
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif


    // HEARTBEAT structure for testing packets without target system/component.
    struct PACKED Heartbeat
    {
        struct PACKED payload
        {
            uint8_t type = 1;
            uint8_t autopilot = 2;
            uint8_t base_mode = 3;
            uint32_t custom_mode = 4;
            uint8_t system_status = 5;
            uint8_t mavlink_version = 6;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0;
        uint8_t sysid = 1;
        uint8_t compid = 0;
        uint8_t msgid = 0;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // PING structure for testing target system/compoent.
    struct PACKED Ping
    {
        struct PACKED payload
        {
            uint64_t time_usec = 295128000000000;
            uint32_t seq = 0xBA5EBA11;
            uint8_t target_system = 255;
            uint8_t target_component = 23;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0;
        uint8_t sysid = 60;
        uint8_t compid = 40;
        uint8_t msgid = 4;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // SET_MODE structure for testing target system only.
    struct PACKED SetMode
    {
        struct PACKED payload
        {
            uint32_t custom_mode = 2;
            uint8_t target_system = 123;
            uint8_t base_mode = 1;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0;
        uint8_t sysid = 70;
        uint8_t compid = 30;
        uint8_t msgid = 11;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // ENCAPSULATED_DATA structure for testing maximum length packets.
    struct PACKED EncapsulatedData
    {
        struct PACKED payload
        {
            uint16_t seqnr = 0;
            uint8_t data[253];
        };
        uint8_t magic = 0xFE;
        uint8_t len = 255;
        uint8_t seq = 0;
        uint8_t sysid = 255;
        uint8_t compid = 1;
        uint8_t msgid = 131;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // Convert a MAVLink packet structure to a vector of bytes.
    template <class T>
    static std::vector<uint8_t> to_vector(T packet)
    {
        std::vector<uint8_t> data;
        data.assign(reinterpret_cast<uint8_t *>(&packet),
                    reinterpret_cast<uint8_t *>(&packet) + sizeof(packet));
        return data;
    }

}


TEST_CASE("PacketVersion1's can be constructed.", "[PacketVersion1]")
{
    Heartbeat heartbeat;
    Ping ping;
    SetMode set_mode;
    EncapsulatedData encapsulated_data;
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("With proper arguments.")
    {
        // HEARTBEAT
        REQUIRE_NOTHROW(PacketVersion1(to_vector(heartbeat), conn));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(heartbeat), conn, -10));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(heartbeat), conn, +10));
        // PING
        REQUIRE_NOTHROW(PacketVersion1(to_vector(ping), conn));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(ping), conn, -10));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(ping), conn, +10));
        // SET_MODE
        REQUIRE_NOTHROW(PacketVersion1(to_vector(set_mode), conn));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(set_mode), conn, -10));
        REQUIRE_NOTHROW(PacketVersion1(to_vector(set_mode), conn, +10));
        // ENCAPSULATED_DATA
        REQUIRE_NOTHROW(PacketVersion1(to_vector(encapsulated_data), conn));
        REQUIRE_NOTHROW(
            PacketVersion1(to_vector(encapsulated_data), conn, -10));
        REQUIRE_NOTHROW(
            PacketVersion1(to_vector(encapsulated_data), conn, +10));
    }
    SECTION("And ensures a complete header is given.")
    {
        REQUIRE_THROWS_AS(PacketVersion1({}, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1({}, conn),
            "Packet (0 bytes) is shorter than a v1.0 header (6 bytes).");
        REQUIRE_THROWS_AS(PacketVersion1({1, 2, 3, 4, 5}, conn),
                          std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1({1, 2, 3, 4, 5}, conn),
            "Packet (5 bytes) is shorter than a v1.0 header (6 bytes).");
    }
    SECTION("And ensures packets begins with the magic byte (0xFE).")
    {
        heartbeat.magic = 0xBC;
        REQUIRE_THROWS_AS(PacketVersion1(to_vector(heartbeat), conn),
                          std::invalid_argument);
        REQUIRE_THROWS_WITH(
            PacketVersion1(to_vector(heartbeat), conn),
            "Invalid packet starting byte (0xBC), "
            "v1.0 packets should start with 0xFE.");
        ping.magic = 0xAD;
        REQUIRE_THROWS_AS(PacketVersion1(to_vector(ping), conn),
                          std::invalid_argument);
        REQUIRE_THROWS_WITH(
            PacketVersion1(to_vector(ping), conn),
            "Invalid packet starting byte (0xAD), "
            "v1.0 packets should start with 0xFE.");
    }
    SECTION("And ensures the message ID is valid.")
    {
        ping.msgid = 255; // ID 255 is not currently valid.
        REQUIRE_THROWS_AS(
            PacketVersion1(to_vector(ping), conn), std::runtime_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1(to_vector(ping), conn), "Invalid packet ID (#255).");
    }
    SECTION("And ensures the packet is the correct length.")
    {
        // HEARTBEAT (no target system/component).
        auto heartbeat_data = to_vector(heartbeat);
        heartbeat_data.pop_back();
        REQUIRE_THROWS_AS(
            PacketVersion1(heartbeat_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1(heartbeat_data, conn),
            "Packet is 16 bytes, should be 17 bytes.");
        // PING (with target system/component).
        auto ping_data = to_vector(ping);
        ping_data.push_back(0x00);
        REQUIRE_THROWS_AS(PacketVersion1(ping_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1(ping_data, conn),
            "Packet is 23 bytes, should be 22 bytes.");
        // SET_MODE (target system only, no target component).
        auto set_mode_data = to_vector(set_mode);
        set_mode_data.pop_back();
        REQUIRE_THROWS_AS(
            PacketVersion1(set_mode_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1(set_mode_data, conn),
            "Packet is 13 bytes, should be 14 bytes.");
        // ENCAPSULATED_DATA (longest packet).
        auto encapsulated_data_data = to_vector(encapsulated_data);
        encapsulated_data_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            PacketVersion1(encapsulated_data_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            PacketVersion1(encapsulated_data_data, conn),
            "Packet is 264 bytes, should be 263 bytes.");
    }
}


TEST_CASE("PacketVersion1's contain raw packet data and make it accessible.",
          "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(PacketVersion1(heartbeat, conn).data() == heartbeat);
    REQUIRE(PacketVersion1(ping, conn).data() == ping);
    REQUIRE(PacketVersion1(set_mode, conn).data() == set_mode);
    REQUIRE(PacketVersion1(encapsulated_data, conn).data() ==
            encapsulated_data);
}


TEST_CASE("PacketVersion1's contain a weak_ptr to a connection.",
          "[PacketVersion1]")
{
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    std::weak_ptr<ConnectionTestClass> empty_conn; // empty weak pointer
    REQUIRE(PacketVersion1(ping, conn1).connection().lock() == conn1);
    REQUIRE(PacketVersion1(ping, conn2).connection().lock() == conn2);
    REQUIRE(PacketVersion1(ping, empty_conn).connection().lock() ==
            empty_conn.lock());
    REQUIRE(PacketVersion1(ping, empty_conn).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketVersion1(ping, conn1).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketVersion1(ping, conn2).connection().lock() == nullptr);
    REQUIRE_FALSE(PacketVersion1(ping, conn1).connection().lock() == conn2);
    REQUIRE_FALSE(PacketVersion1(ping, conn2).connection().lock() == conn1);
    REQUIRE_FALSE(
        PacketVersion1(ping, empty_conn).connection().lock() == conn1);
    REQUIRE_FALSE(
        PacketVersion1(ping, empty_conn).connection().lock() == conn2);
}


TEST_CASE("PacketVersion1's have a priority.", "[PacketVersion1]")
{
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("Which has a default value of 0.")
    {
        REQUIRE(PacketVersion1(ping, conn).priority() == 0);
    }
    SECTION("That can be set during construction.")
    {
        REQUIRE(PacketVersion1(ping, conn, -32768).priority() == -32768);
        REQUIRE(PacketVersion1(ping, conn, 0).priority() == 0);
        REQUIRE(PacketVersion1(ping, conn, 32767).priority() == 32767);

        // Loop over every 1000 priority values.
        for (auto i : boost::irange(-32768, 32768, 1000))
        {
            REQUIRE(PacketVersion1(ping, conn, i).priority() == i);
        }
    }
    SECTION("That can be set after construction.")
    {
        PacketVersion1 packet(ping, conn);
        REQUIRE(packet.priority() == 0);
        // -32768
        REQUIRE(packet.priority(-32768) == -32768);
        REQUIRE(packet.priority() == -32768);
        // 0
        REQUIRE(packet.priority(0) == 0);
        REQUIRE(packet.priority() == 0);
        // 32767
        REQUIRE(packet.priority(32767) == 32767);
        REQUIRE(packet.priority() == 32767);

        // Loop over every 1000 priority values.
        for (auto i : boost::irange(-32768, 32768, 1000))
        {
            REQUIRE(packet.priority(i) == i);
            REQUIRE(packet.priority() == i);
        }
    }
}


TEST_CASE("PacketVersion1's are copyable.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    PacketVersion1 a(heartbeat, conn1, 1);
    PacketVersion1 b(ping, conn2, 2);
    PacketVersion1 a_copy = a;
    PacketVersion1 b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a.data() == a_copy.data());
    REQUIRE(a.connection().lock() == a_copy.connection().lock());
    REQUIRE(a.priority() == a_copy.priority());
    REQUIRE(&b != &b_copy);
    REQUIRE(b.data() == b_copy.data());
    REQUIRE(b.connection().lock() == b_copy.connection().lock());
    REQUIRE(b.priority() == b_copy.priority());
}


TEST_CASE("PacketVersion1's are movable.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    PacketVersion1 a(heartbeat, conn1, 1);
    PacketVersion1 b(ping, conn2, 2);
    PacketVersion1 a_moved = std::move(a);
    PacketVersion1 b_moved(std::move(b));
    REQUIRE(a_moved.data() == heartbeat);
    REQUIRE(a_moved.connection().lock() == conn1);
    REQUIRE(a_moved.priority() == 1);
    REQUIRE(b_moved.data() == ping);
    REQUIRE(b_moved.connection().lock() == conn2);
    REQUIRE(b_moved.priority() == 2);
}


TEST_CASE("PacketVersion1's are assignable.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketVersion1 packet(heartbeat, conn, -10);
    PacketVersion1 packet_to_copy(ping, std::weak_ptr<ConnectionTestClass>(), 10);
    REQUIRE(packet.data() == heartbeat);
    REQUIRE(packet.connection().lock() == conn);
    REQUIRE(packet.priority() == -10);
    packet = packet_to_copy;
    REQUIRE(packet.data() == ping);
    REQUIRE(packet.connection().lock() == nullptr);
    REQUIRE(packet.priority() == 10);
}


TEST_CASE("PacketVersion1's are assignable (with move semantics).",
          "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketVersion1 packet(heartbeat, conn, -10);
    PacketVersion1 packet_to_move(ping, std::weak_ptr<ConnectionTestClass>(), 10);
    REQUIRE(packet.data() == heartbeat);
    REQUIRE(packet.connection().lock() == conn);
    REQUIRE(packet.priority() == -10);
    packet = std::move(packet_to_move);
    REQUIRE(packet.data() == ping);
    REQUIRE(packet.connection().lock() == nullptr);
    REQUIRE(packet.priority() == 10);
}


TEST_CASE("PacketVersion1's have a version.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    // All should read 0x0100 for v1.0.
    REQUIRE(PacketVersion1(heartbeat, conn).version() == 0x0100);
    REQUIRE(PacketVersion1(ping, conn).version() == 0x0100);
    REQUIRE(PacketVersion1(set_mode, conn).version() == 0x0100);
    REQUIRE(PacketVersion1(encapsulated_data, conn).version() == 0x0100);
}


TEST_CASE("PacketVersion1's have an ID.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(PacketVersion1(heartbeat, conn).id() == 0);
    REQUIRE(PacketVersion1(ping, conn).id() == 4);
    REQUIRE(PacketVersion1(set_mode, conn).id() == 11);
    REQUIRE(PacketVersion1(encapsulated_data, conn).id() == 131);
}


TEST_CASE("PacketVersion1's have a name.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(PacketVersion1(heartbeat, conn).name() == "HEARTBEAT");
    REQUIRE(PacketVersion1(ping, conn).name() == "PING");
    REQUIRE(PacketVersion1(set_mode, conn).name() == "SET_MODE");
    REQUIRE(PacketVersion1(encapsulated_data, conn).name() ==
            "ENCAPSULATED_DATA");
}


TEST_CASE("PacketVersion1's have a source address.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(PacketVersion1(heartbeat, conn).source() == MAVAddress("1.0"));
    REQUIRE(PacketVersion1(ping, conn).source() == MAVAddress("60.40"));
    REQUIRE(PacketVersion1(set_mode, conn).source() == MAVAddress("70.30"));
    REQUIRE(PacketVersion1(encapsulated_data, conn).source() ==
            MAVAddress("255.1"));
}


TEST_CASE("PacketVersion1's optionally have a destination address.",
          "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE_THROWS_AS(
        PacketVersion1(heartbeat, conn).dest().value(),
        std::bad_optional_access);
    REQUIRE(PacketVersion1(ping, conn).dest().value() == MAVAddress("255.23"));
    REQUIRE(PacketVersion1(set_mode, conn).dest().value() ==
            MAVAddress("123.0"));
    REQUIRE_THROWS_AS(
        PacketVersion1(encapsulated_data, conn).dest().value(),
        std::bad_optional_access);
}


TEST_CASE("PacketVersion1's are printable.", "[PacketVersion1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(str(PacketVersion1(heartbeat, conn)) ==
            "HEARTBEAT (#0) from 1.0 (v1.0)");
    REQUIRE(str(PacketVersion1(ping, conn)) ==
            "PING (#4) from 60.40 to 255.23 (v1.0)");
    REQUIRE(str(PacketVersion1(set_mode, conn)) ==
            "SET_MODE (#11) from 70.30 to 123.0 (v1.0)");
    REQUIRE(str(PacketVersion1(encapsulated_data, conn)) ==
            "ENCAPSULATED_DATA (#131) from 255.1 (v1.0)");
}
