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


TEST_CASE("packet_v1::Packet's can be constructed.", "[packet_v1::Packet]")
{
    Heartbeat heartbeat;
    Ping ping;
    SetMode set_mode;
    EncapsulatedData encapsulated_data;
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("With proper arguments.")
    {
        // HEARTBEAT
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(heartbeat), conn));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(heartbeat), conn, -10));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(heartbeat), conn, +10));
        // PING
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(ping), conn));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(ping), conn, -10));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(ping), conn, +10));
        // SET_MODE
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(set_mode), conn));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(set_mode), conn, -10));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(set_mode), conn, +10));
        // ENCAPSULATED_DATA
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(encapsulated_data), conn));
        REQUIRE_NOTHROW(
            packet_v1::Packet(to_vector(encapsulated_data), conn, -10));
        REQUIRE_NOTHROW(
            packet_v1::Packet(to_vector(encapsulated_data), conn, +10));
    }
    SECTION("And ensures a complete header is given.")
    {
        REQUIRE_THROWS_AS(packet_v1::Packet({}, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet({}, conn),
            "Packet is empty.");
        REQUIRE_THROWS_AS(packet_v1::Packet({0xFE, 2, 3, 4, 5}, conn),
                          std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet({0xFE, 2, 3, 4, 5}, conn),
            "Packet (5 bytes) is shorter than a v1.0 header (6 bytes).");
    }
    SECTION("And ensures packets begins with the magic byte (0xFE).")
    {
        heartbeat.magic = 0xBC;
        REQUIRE_THROWS_AS(packet_v1::Packet(to_vector(heartbeat), conn),
                          std::invalid_argument);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(to_vector(heartbeat), conn),
            "Invalid packet starting byte (0xBC), "
            "v1.0 packets should start with 0xFE.");
        ping.magic = 0xAD;
        REQUIRE_THROWS_AS(packet_v1::Packet(to_vector(ping), conn),
                          std::invalid_argument);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(to_vector(ping), conn),
            "Invalid packet starting byte (0xAD), "
            "v1.0 packets should start with 0xFE.");
    }
    SECTION("And ensures the message ID is valid.")
    {
        ping.msgid = 255; // ID 255 is not currently valid.
        REQUIRE_THROWS_AS(
            packet_v1::Packet(to_vector(ping), conn), std::runtime_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(to_vector(ping), conn),
            "Invalid packet ID (#255).");
    }
    SECTION("And ensures the packet is the correct length.")
    {
        // HEARTBEAT (no target system/component).
        auto heartbeat_data = to_vector(heartbeat);
        heartbeat_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v1::Packet(heartbeat_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(heartbeat_data, conn),
            "Packet is 16 bytes, should be 17 bytes.");
        // PING (with target system/component).
        auto ping_data = to_vector(ping);
        ping_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            packet_v1::Packet(ping_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(ping_data, conn),
            "Packet is 23 bytes, should be 22 bytes.");
        // SET_MODE (target system only, no target component).
        auto set_mode_data = to_vector(set_mode);
        set_mode_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v1::Packet(set_mode_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(set_mode_data, conn),
            "Packet is 13 bytes, should be 14 bytes.");
        // ENCAPSULATED_DATA (longest packet).
        auto encapsulated_data_data = to_vector(encapsulated_data);
        encapsulated_data_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            packet_v1::Packet(encapsulated_data_data, conn), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(encapsulated_data_data, conn),
            "Packet is 264 bytes, should be 263 bytes.");
    }
}


TEST_CASE("packet_v1::Packet's contain raw packet data and make it accessible.",
          "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(packet_v1::Packet(heartbeat, conn).data() == heartbeat);
    REQUIRE(packet_v1::Packet(ping, conn).data() == ping);
    REQUIRE(packet_v1::Packet(set_mode, conn).data() == set_mode);
    REQUIRE(packet_v1::Packet(encapsulated_data, conn).data() ==
            encapsulated_data);
}


TEST_CASE("packet_v1::Packet's contain a weak_ptr to a connection.",
          "[packet_v1::Packet]")
{
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    std::weak_ptr<ConnectionTestClass> empty_conn; // empty weak pointer
    REQUIRE(packet_v1::Packet(ping, conn1).connection().lock() == conn1);
    REQUIRE(packet_v1::Packet(ping, conn2).connection().lock() == conn2);
    REQUIRE(packet_v1::Packet(ping, empty_conn).connection().lock() ==
            empty_conn.lock());
    REQUIRE(packet_v1::Packet(ping, empty_conn).connection().lock() == nullptr);
    REQUIRE_FALSE(
        packet_v1::Packet(ping, conn1).connection().lock() == nullptr);
    REQUIRE_FALSE(
        packet_v1::Packet(ping, conn2).connection().lock() == nullptr);
    REQUIRE_FALSE(packet_v1::Packet(ping, conn1).connection().lock() == conn2);
    REQUIRE_FALSE(packet_v1::Packet(ping, conn2).connection().lock() == conn1);
    REQUIRE_FALSE(
        packet_v1::Packet(ping, empty_conn).connection().lock() == conn1);
    REQUIRE_FALSE(
        packet_v1::Packet(ping, empty_conn).connection().lock() == conn2);
}


TEST_CASE("packet_v1::Packet's have a priority.", "[packet_v1::Packet]")
{
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("Which has a default value of 0.")
    {
        REQUIRE(packet_v1::Packet(ping, conn).priority() == 0);
    }
    SECTION("That can be set during construction.")
    {
        REQUIRE(packet_v1::Packet(ping, conn, -32768).priority() == -32768);
        REQUIRE(packet_v1::Packet(ping, conn, 0).priority() == 0);
        REQUIRE(packet_v1::Packet(ping, conn, 32767).priority() == 32767);

        // Loop over every 1000 priority values.
        for (auto i : boost::irange(-32768, 32768, 1000))
        {
            REQUIRE(packet_v1::Packet(ping, conn, i).priority() == i);
        }
    }
    SECTION("That can be set after construction.")
    {
        packet_v1::Packet packet(ping, conn);
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


TEST_CASE("packet_v1::Packet's are copyable.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    packet_v1::Packet a(heartbeat, conn1, 1);
    packet_v1::Packet b(ping, conn2, 2);
    packet_v1::Packet a_copy = a;
    packet_v1::Packet b_copy(b);
    REQUIRE(&a != &a_copy);
    REQUIRE(a.data() == a_copy.data());
    REQUIRE(a.connection().lock() == a_copy.connection().lock());
    REQUIRE(a.priority() == a_copy.priority());
    REQUIRE(&b != &b_copy);
    REQUIRE(b.data() == b_copy.data());
    REQUIRE(b.connection().lock() == b_copy.connection().lock());
    REQUIRE(b.priority() == b_copy.priority());
}


TEST_CASE("packet_v1::Packet's are movable.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    packet_v1::Packet a(heartbeat, conn1, 1);
    packet_v1::Packet b(ping, conn2, 2);
    packet_v1::Packet a_moved = std::move(a);
    packet_v1::Packet b_moved(std::move(b));
    REQUIRE(a_moved.data() == heartbeat);
    REQUIRE(a_moved.connection().lock() == conn1);
    REQUIRE(a_moved.priority() == 1);
    REQUIRE(b_moved.data() == ping);
    REQUIRE(b_moved.connection().lock() == conn2);
    REQUIRE(b_moved.priority() == 2);
}


TEST_CASE("packet_v1::Packet's are assignable.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    packet_v1::Packet packet(heartbeat, conn, -10);
    packet_v1::Packet packet_to_copy(
        ping, std::weak_ptr<ConnectionTestClass>(), 10);
    REQUIRE(packet.data() == heartbeat);
    REQUIRE(packet.connection().lock() == conn);
    REQUIRE(packet.priority() == -10);
    packet = packet_to_copy;
    REQUIRE(packet.data() == ping);
    REQUIRE(packet.connection().lock() == nullptr);
    REQUIRE(packet.priority() == 10);
}


TEST_CASE("packet_v1::Packet's are assignable (with move semantics).",
          "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto conn = std::make_shared<ConnectionTestClass>();
    packet_v1::Packet packet(heartbeat, conn, -10);
    packet_v1::Packet packet_to_move(
        ping, std::weak_ptr<ConnectionTestClass>(), 10);
    REQUIRE(packet.data() == heartbeat);
    REQUIRE(packet.connection().lock() == conn);
    REQUIRE(packet.priority() == -10);
    packet = std::move(packet_to_move);
    REQUIRE(packet.data() == ping);
    REQUIRE(packet.connection().lock() == nullptr);
    REQUIRE(packet.priority() == 10);
}


TEST_CASE("packet_v1::Packet's have a version.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    // All should read 0x0100 for v1.0.
    REQUIRE(packet_v1::Packet(heartbeat, conn).version() == 0x0100);
    REQUIRE(packet_v1::Packet(ping, conn).version() == 0x0100);
    REQUIRE(packet_v1::Packet(set_mode, conn).version() == 0x0100);
    REQUIRE(packet_v1::Packet(encapsulated_data, conn).version() == 0x0100);
}


TEST_CASE("packet_v1::Packet's have an ID.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(packet_v1::Packet(heartbeat, conn).id() == 0);
    REQUIRE(packet_v1::Packet(ping, conn).id() == 4);
    REQUIRE(packet_v1::Packet(set_mode, conn).id() == 11);
    REQUIRE(packet_v1::Packet(encapsulated_data, conn).id() == 131);
}


TEST_CASE("packet_v1::Packet's have a name.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(packet_v1::Packet(heartbeat, conn).name() == "HEARTBEAT");
    REQUIRE(packet_v1::Packet(ping, conn).name() == "PING");
    REQUIRE(packet_v1::Packet(set_mode, conn).name() == "SET_MODE");
    REQUIRE(packet_v1::Packet(encapsulated_data, conn).name() ==
            "ENCAPSULATED_DATA");
}


TEST_CASE("packet_v1::Packet's have a source address.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(packet_v1::Packet(heartbeat, conn).source() == MAVAddress("1.0"));
    REQUIRE(packet_v1::Packet(ping, conn).source() == MAVAddress("60.40"));
    REQUIRE(packet_v1::Packet(set_mode, conn).source() == MAVAddress("70.30"));
    REQUIRE(packet_v1::Packet(encapsulated_data, conn).source() ==
            MAVAddress("255.1"));
}


TEST_CASE("packet_v1::Packet's optionally have a destination address.",
          "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE_THROWS_AS(
        packet_v1::Packet(heartbeat, conn).dest().value(),
        std::bad_optional_access);
    REQUIRE(packet_v1::Packet(ping, conn).dest().value() ==
            MAVAddress("255.23"));
    REQUIRE(packet_v1::Packet(set_mode, conn).dest().value() ==
            MAVAddress("123.0"));
    REQUIRE_THROWS_AS(
        packet_v1::Packet(encapsulated_data, conn).dest().value(),
        std::bad_optional_access);
}


TEST_CASE("packet_v1::Packet's are printable.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE(str(packet_v1::Packet(heartbeat, conn)) ==
            "HEARTBEAT (#0) from 1.0 (v1.0)");
    REQUIRE(str(packet_v1::Packet(ping, conn)) ==
            "PING (#4) from 60.40 to 255.23 (v1.0)");
    REQUIRE(str(packet_v1::Packet(set_mode, conn)) ==
            "SET_MODE (#11) from 70.30 to 123.0 (v1.0)");
    REQUIRE(str(packet_v1::Packet(encapsulated_data, conn)) ==
            "ENCAPSULATED_DATA (#131) from 255.1 (v1.0)");
}


TEST_CASE("'is_magic' determines if a byte is the v1.0 packet magic byte.",
        "[packet_v1]")
{
    REQUIRE_FALSE(packet_v1::is_magic(0xAD));
    REQUIRE_FALSE(packet_v1::is_magic(0xBC));
    REQUIRE(packet_v1::is_magic(0xFE));
    REQUIRE_FALSE(packet_v1::is_magic(0xFD));
}


TEST_CASE("'header_complete' determines wheather the given bytes "
          "at least represent a complete header.", "[packet_v1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("Returns true when at least a complete header is given.")
    {
        heartbeat.resize(6);
        ping.resize(10);
        set_mode.resize(15);
        REQUIRE(packet_v1::header_complete(heartbeat));
        REQUIRE(packet_v1::header_complete(ping));
        REQUIRE(packet_v1::header_complete(set_mode));
        REQUIRE(packet_v1::header_complete(encapsulated_data));
    }
    SECTION("Returns false when an incomplete header is given.")
    {
        heartbeat.resize(5);
        ping.resize(4);
        set_mode.resize(3);
        encapsulated_data.resize(0);
        REQUIRE_FALSE(packet_v1::header_complete(heartbeat));
        REQUIRE_FALSE(packet_v1::header_complete(ping));
        REQUIRE_FALSE(packet_v1::header_complete(set_mode));
        REQUIRE_FALSE(packet_v1::header_complete(encapsulated_data));
    }
    SECTION("Returns false when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFD;
        encapsulated_data.front() = 0xFD;
        REQUIRE_FALSE(packet_v1::header_complete(heartbeat));
        REQUIRE_FALSE(packet_v1::header_complete(ping));
        REQUIRE_FALSE(packet_v1::header_complete(set_mode));
        REQUIRE_FALSE(packet_v1::header_complete(encapsulated_data));
    }
}


TEST_CASE("'packet_complete' determines wheather the given bytes represent a "
          "complete packet.", "[packet_v1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("Returns true when a complete packet is given.")
    {
        REQUIRE(packet_v1::packet_complete(heartbeat));
        REQUIRE(packet_v1::packet_complete(ping));
        REQUIRE(packet_v1::packet_complete(set_mode));
        REQUIRE(packet_v1::packet_complete(encapsulated_data));
    }
    SECTION("Returns false when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFD;
        encapsulated_data.front() = 0xFD;
        REQUIRE_FALSE(packet_v1::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v1::packet_complete(ping));
        REQUIRE_FALSE(packet_v1::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v1::packet_complete(encapsulated_data));
    }
    SECTION("Returns false when the packet is too short.")
    {
        heartbeat.pop_back();
        ping.pop_back();
        set_mode.pop_back();
        encapsulated_data.pop_back();
        REQUIRE_FALSE(packet_v1::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v1::packet_complete(ping));
        REQUIRE_FALSE(packet_v1::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v1::packet_complete(encapsulated_data));
    }
    SECTION("Returns false when the packet is too long.")
    {
        heartbeat.push_back(0x00);
        ping.push_back(0x00);
        set_mode.push_back(0x00);
        encapsulated_data.push_back(0x00);
        REQUIRE_FALSE(packet_v1::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v1::packet_complete(ping));
        REQUIRE_FALSE(packet_v1::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v1::packet_complete(encapsulated_data));
    }
}


TEST_CASE("'header' returns a structure pointer to the given header data."
          "[packet_v1]")
{
    auto heartbeat = to_vector(Heartbeat());
    auto ping = to_vector(Ping());
    auto set_mode = to_vector(SetMode());
    auto encapsulated_data = to_vector(EncapsulatedData());
    auto conn = std::make_shared<ConnectionTestClass>();
    SECTION("Header contains a magic value.")
    {
        REQUIRE(packet_v1::header(heartbeat)->magic == 0xFE);
        REQUIRE(packet_v1::header(ping)->magic == 0xFE);
        REQUIRE(packet_v1::header(set_mode)->magic == 0xFE);
        REQUIRE(packet_v1::header(encapsulated_data)->magic == 0xFE);
    }
    SECTION("Header stores the packet length.")
    {
        REQUIRE(packet_v1::header(heartbeat)->len == 9);
        REQUIRE(packet_v1::header(ping)->len == 14);
        REQUIRE(packet_v1::header(set_mode)->len == 6);
        REQUIRE(packet_v1::header(encapsulated_data)->len == 255);
    }
    SECTION("Header has a sequence number.")
    {
        REQUIRE(packet_v1::header(heartbeat)->seq == 0);
        REQUIRE(packet_v1::header(ping)->seq == 0);
        REQUIRE(packet_v1::header(set_mode)->seq == 0);
        REQUIRE(packet_v1::header(encapsulated_data)->seq == 0);
    }
    SECTION("Header has a system ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->sysid == 1);
        REQUIRE(packet_v1::header(ping)->sysid == 60);
        REQUIRE(packet_v1::header(set_mode)->sysid == 70);
        REQUIRE(packet_v1::header(encapsulated_data)->sysid == 255);
    }
    SECTION("Header has a component ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->compid == 0);
        REQUIRE(packet_v1::header(ping)->compid == 40);
        REQUIRE(packet_v1::header(set_mode)->compid == 30);
        REQUIRE(packet_v1::header(encapsulated_data)->compid == 1);
    }
    SECTION("Header has a message ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->msgid == 0);
        REQUIRE(packet_v1::header(ping)->msgid == 4);
        REQUIRE(packet_v1::header(set_mode)->msgid == 11);
        REQUIRE(packet_v1::header(encapsulated_data)->msgid == 131);
    }
    SECTION("Returns false when an incomplete header is given.")
    {
        heartbeat.resize(5);
        ping.resize(4);
        set_mode.resize(3);
        encapsulated_data.resize(0);
        REQUIRE(packet_v1::header(heartbeat) == nullptr);
        REQUIRE(packet_v1::header(ping) == nullptr);
        REQUIRE(packet_v1::header(set_mode) == nullptr);
        REQUIRE(packet_v1::header(encapsulated_data) == nullptr);
    }
    SECTION("Returns false when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFD;
        encapsulated_data.front() = 0xFD;
        REQUIRE(packet_v1::header(heartbeat) == nullptr);
        REQUIRE(packet_v1::header(ping) == nullptr);
        REQUIRE(packet_v1::header(set_mode) == nullptr);
        REQUIRE(packet_v1::header(encapsulated_data) == nullptr);
    }
}
