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
#include <optional>
#include <stdexcept>
#include <utility>

#include <catch.hpp>

#include "MAVAddress.hpp"
#include "PacketVersion1.hpp"
#include "mavlink.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE(
    "'packet_v1::header_complete' determines whether the given bytes "
    "at least represent a complete header.",
    "[packet_v1]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
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


TEST_CASE(
    "'packet_v1::header' returns a structure pointer to the given "
    "header data.",
    "[packet_v1]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    SECTION("Header contains a magic value.")
    {
        REQUIRE(packet_v1::header(heartbeat)->magic == 0xFE);
        REQUIRE(packet_v1::header(ping)->magic == 0xFE);
        REQUIRE(packet_v1::header(set_mode)->magic == 0xFE);
        REQUIRE(packet_v1::header(encapsulated_data)->magic == 0xFE);
    }
    SECTION("Header stores the payload length.")
    {
        REQUIRE(packet_v1::header(heartbeat)->len == 9);
        REQUIRE(packet_v1::header(ping)->len == 14);
        REQUIRE(packet_v1::header(set_mode)->len == 6);
        REQUIRE(packet_v1::header(encapsulated_data)->len == 255);
    }
    SECTION("Header has a sequence number.")
    {
        REQUIRE(packet_v1::header(heartbeat)->seq == 0xFE);
        REQUIRE(packet_v1::header(ping)->seq == 0xFE);
        REQUIRE(packet_v1::header(set_mode)->seq == 0xFE);
        REQUIRE(packet_v1::header(encapsulated_data)->seq == 0xFE);
    }
    SECTION("Header has a system ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->sysid == 127);
        REQUIRE(packet_v1::header(ping)->sysid == 192);
        REQUIRE(packet_v1::header(set_mode)->sysid == 172);
        REQUIRE(packet_v1::header(encapsulated_data)->sysid == 224);
    }
    SECTION("Header has a component ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->compid == 1);
        REQUIRE(packet_v1::header(ping)->compid == 168);
        REQUIRE(packet_v1::header(set_mode)->compid == 0);
        REQUIRE(packet_v1::header(encapsulated_data)->compid == 255);
    }
    SECTION("Header has a message ID.")
    {
        REQUIRE(packet_v1::header(heartbeat)->msgid == 0);
        REQUIRE(packet_v1::header(ping)->msgid == 4);
        REQUIRE(packet_v1::header(set_mode)->msgid == 11);
        REQUIRE(packet_v1::header(encapsulated_data)->msgid == 131);
    }
    SECTION("Returns nullptr when an incomplete header is given.")
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
    SECTION("Returns nullptr when the magic byte is wrong.")
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


TEST_CASE(
    "'packet_v1::packet_complete' determines whether the given bytes "
    "represent a complete packet.",
    "[packet_v1]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
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


TEST_CASE("packet_v1::Packet's can be constructed.", "[packet_v1::Packet]")
{
    HeartbeatV1 heartbeat;
    PingV1 ping;
    SetModeV1 set_mode;
    EncapsulatedDataV1 encapsulated_data;
    SECTION("With proper arguments.")
    {
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(heartbeat)));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(ping)));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(set_mode)));
        REQUIRE_NOTHROW(packet_v1::Packet(to_vector(encapsulated_data)));
    }
    SECTION("And ensures a complete header is given.")
    {
        REQUIRE_THROWS_AS(packet_v1::Packet({}), std::length_error);
        REQUIRE_THROWS_WITH(packet_v1::Packet({}), "Packet is empty.");
        REQUIRE_THROWS_AS(
            packet_v1::Packet({0xFE, 2, 3, 4, 5}), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet({0xFE, 2, 3, 4, 5}),
            "Packet (5 bytes) is shorter than a v1.0 header (6 bytes).");
    }
    SECTION("And ensures packets begins with the magic byte (0xFE).")
    {
        ping.magic = 0xAD;
        REQUIRE_THROWS_AS(
            packet_v1::Packet(to_vector(ping)), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(to_vector(ping)),
            "Invalid packet starting byte (0xAD), "
            "v1.0 packets should start with 0xFE.");
    }
    SECTION("And ensures the message ID is valid.")
    {
        ping.msgid = 255;  // ID 255 is not currently valid.
        REQUIRE_THROWS_AS(
            packet_v1::Packet(to_vector(ping)), std::runtime_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(to_vector(ping)), "Invalid packet ID (#255).");
    }
    SECTION("And ensures the packet is the correct length.")
    {
        // HEARTBEAT (no target system/component).
        auto heartbeat_data = to_vector(heartbeat);
        heartbeat_data.pop_back();
        REQUIRE_THROWS_AS(packet_v1::Packet(heartbeat_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(heartbeat_data),
            "Packet is 16 bytes, should be 17 bytes.");
        // PING (with target system/component).
        auto ping_data = to_vector(ping);
        ping_data.push_back(0x00);
        REQUIRE_THROWS_AS(packet_v1::Packet(ping_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(ping_data),
            "Packet is 23 bytes, should be 22 bytes.");
        // SET_MODE (target system only, no target component).
        auto set_mode_data = to_vector(set_mode);
        set_mode_data.pop_back();
        REQUIRE_THROWS_AS(packet_v1::Packet(set_mode_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(set_mode_data),
            "Packet is 13 bytes, should be 14 bytes.");
        // ENCAPSULATED_DATA (longest packet).
        auto encapsulated_data_data = to_vector(encapsulated_data);
        encapsulated_data_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            packet_v1::Packet(encapsulated_data_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v1::Packet(encapsulated_data_data),
            "Packet is 264 bytes, should be 263 bytes.");
    }
}


TEST_CASE("packet_v1::Packet's are comparable.", "[packet_v1::Packet]")
{
    SECTION("with ==")
    {
        REQUIRE(
            packet_v1::Packet(to_vector(PingV1())) ==
            packet_v1::Packet(to_vector(PingV1())));
        REQUIRE_FALSE(
            packet_v1::Packet(to_vector(PingV1())) ==
            packet_v1::Packet(to_vector(SetModeV1())));
    }
    SECTION("with !=")
    {
        REQUIRE(
            packet_v1::Packet(to_vector(PingV1())) !=
            packet_v1::Packet(to_vector(SetModeV1())));
        REQUIRE_FALSE(
            packet_v1::Packet(to_vector(PingV1())) !=
            packet_v1::Packet(to_vector(PingV1())));
    }
}


TEST_CASE("packet_v1::Packet's are copyable.", "[packet_v1::Packet]")
{
    packet_v1::Packet original(to_vector(PingV1()));
    packet_v1::Packet copy(original);
    REQUIRE(copy == packet_v1::Packet(to_vector(PingV1())));
}


TEST_CASE("packet_v1::Packet's are movable.", "[packet_v1::Packet]")
{
    packet_v1::Packet original(to_vector(PingV1()));
    packet_v1::Packet moved(std::move(original));
    REQUIRE(moved == packet_v1::Packet(to_vector(PingV1())));
}


TEST_CASE("packet_v1::Packet's are assignable.", "[Packet]")
{
    packet_v1::Packet a(to_vector(PingV1()));
    packet_v1::Packet b(to_vector(SetModeV1()));
    REQUIRE(a == packet_v1::Packet(to_vector(PingV1())));
    a = b;
    REQUIRE(b == packet_v1::Packet(to_vector(SetModeV1())));
}


TEST_CASE("packet_v1::Packet's are assignable (by move semantics).", "[Packet]")
{
    packet_v1::Packet a(to_vector(PingV1()));
    packet_v1::Packet b(to_vector(SetModeV1()));
    REQUIRE(a == packet_v1::Packet(to_vector(PingV1())));
    a = b;
    REQUIRE(b == packet_v1::Packet(to_vector(SetModeV1())));
}


TEST_CASE(
    "packet_v1::Packet's contain raw packet data and make it accessible.",
    "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE(packet_v1::Packet(heartbeat).data() == heartbeat);
    REQUIRE(packet_v1::Packet(ping).data() == ping);
    REQUIRE(packet_v1::Packet(set_mode).data() == set_mode);
    REQUIRE(packet_v1::Packet(encapsulated_data).data() == encapsulated_data);
}


TEST_CASE("packet_v1::Packet's have a version.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    // All should read 0x0100 for v1.0.
    REQUIRE(packet_v1::Packet(heartbeat).version() == Packet::V1);
    REQUIRE(packet_v1::Packet(ping).version() == Packet::V1);
    REQUIRE(packet_v1::Packet(set_mode).version() == 0x100);
    REQUIRE(packet_v1::Packet(encapsulated_data).version() == 0x100);
}


TEST_CASE("packet_v1::Packet's have an ID.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE(packet_v1::Packet(heartbeat).id() == 0);
    REQUIRE(packet_v1::Packet(ping).id() == 4);
    REQUIRE(packet_v1::Packet(set_mode).id() == 11);
    REQUIRE(packet_v1::Packet(encapsulated_data).id() == 131);
}


TEST_CASE("packet_v1::Packet's have a name.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE(packet_v1::Packet(heartbeat).name() == "HEARTBEAT");
    REQUIRE(packet_v1::Packet(ping).name() == "PING");
    REQUIRE(packet_v1::Packet(set_mode).name() == "SET_MODE");
    REQUIRE(packet_v1::Packet(encapsulated_data).name() == "ENCAPSULATED_DATA");
}


TEST_CASE("packet_v1::Packet's have a source address.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE(packet_v1::Packet(heartbeat).source() == MAVAddress("127.1"));
    REQUIRE(packet_v1::Packet(ping).source() == MAVAddress("192.168"));
    REQUIRE(packet_v1::Packet(set_mode).source() == MAVAddress("172.0"));
    REQUIRE(
        packet_v1::Packet(encapsulated_data).source() == MAVAddress("224.255"));
}


TEST_CASE(
    "packet_v1::Packet's optionally have a destination address.",
    "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE_THROWS_AS(
        packet_v1::Packet(heartbeat).dest().value(), std::bad_optional_access);
    REQUIRE(packet_v1::Packet(ping).dest().value() == MAVAddress("127.1"));
    REQUIRE(packet_v1::Packet(set_mode).dest().value() == MAVAddress("123.0"));
    REQUIRE_THROWS_AS(
        packet_v1::Packet(encapsulated_data).dest().value(),
        std::bad_optional_access);
}


TEST_CASE("packet_v1::Packet's are printable.", "[packet_v1::Packet]")
{
    auto heartbeat = to_vector(HeartbeatV1());
    auto ping = to_vector(PingV1());
    auto set_mode = to_vector(SetModeV1());
    auto encapsulated_data = to_vector(EncapsulatedDataV1());
    REQUIRE(
        str(packet_v1::Packet(heartbeat)) ==
        "HEARTBEAT (#0) from 127.1 (v1.0)");
    REQUIRE(
        str(packet_v1::Packet(ping)) ==
        "PING (#4) from 192.168 to 127.1 (v1.0)");
    REQUIRE(
        str(packet_v1::Packet(set_mode)) ==
        "SET_MODE (#11) from 172.0 to 123.0 (v1.0)");
    REQUIRE(
        str(packet_v1::Packet(encapsulated_data)) ==
        "ENCAPSULATED_DATA (#131) from 224.255 (v1.0)");
}
