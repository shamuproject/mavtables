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
#include "mavlink.hpp"
#include "PacketVersion2.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


TEST_CASE("'packet_v2::header_complete' determines whether the given bytes "
          "at least represent a complete header.", "[packet_v2]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    SECTION("Returns true when at least a complete header is given.")
    {
        heartbeat.resize(10);
        ping.resize(15);
        set_mode.resize(20);
        mission_set_current.resize(25);
        encapsulated_data.resize(30);
        REQUIRE(packet_v2::header_complete(heartbeat));
        REQUIRE(packet_v2::header_complete(ping));
        REQUIRE(packet_v2::header_complete(set_mode));
        REQUIRE(packet_v2::header_complete(mission_set_current));
        REQUIRE(packet_v2::header_complete(encapsulated_data));
        REQUIRE(packet_v2::header_complete(param_ext_request_list));
    }
    SECTION("Returns false when an incomplete header is given.")
    {
        heartbeat.resize(5);
        ping.resize(4);
        set_mode.resize(3);
        mission_set_current.resize(2);
        encapsulated_data.resize(1);
        param_ext_request_list.resize(0);
        REQUIRE_FALSE(packet_v2::header_complete(heartbeat));
        REQUIRE_FALSE(packet_v2::header_complete(ping));
        REQUIRE_FALSE(packet_v2::header_complete(set_mode));
        REQUIRE_FALSE(packet_v2::header_complete(mission_set_current));
        REQUIRE_FALSE(packet_v2::header_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::header_complete(param_ext_request_list));
    }
    SECTION("Returns false when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFE;
        mission_set_current.front() = 0xFE;
        encapsulated_data.front() = 0xFE;
        param_ext_request_list.front() = 0xFE;
        REQUIRE_FALSE(packet_v2::header_complete(heartbeat));
        REQUIRE_FALSE(packet_v2::header_complete(ping));
        REQUIRE_FALSE(packet_v2::header_complete(set_mode));
        REQUIRE_FALSE(packet_v2::header_complete(mission_set_current));
        REQUIRE_FALSE(packet_v2::header_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::header_complete(param_ext_request_list));
    }
}


TEST_CASE("'packet_v2::header' returns a structure pointer to the given "
          "header data.", "[packet_v2]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    SECTION("Header contains a magic value.")
    {
        REQUIRE(packet_v2::header(heartbeat)->magic == 0xFD);
        REQUIRE(packet_v2::header(ping)->magic == 0xFD);
        REQUIRE(packet_v2::header(set_mode)->magic == 0xFD);
        REQUIRE(packet_v2::header(mission_set_current)->magic == 0xFD);
        REQUIRE(packet_v2::header(encapsulated_data)->magic == 0xFD);
        REQUIRE(packet_v2::header(param_ext_request_list)->magic == 0xFD);
    }
    SECTION("Header stores the packet length.")
    {
        REQUIRE(packet_v2::header(heartbeat)->len == 9);
        REQUIRE(packet_v2::header(ping)->len == 14);
        REQUIRE(packet_v2::header(set_mode)->len == 6);
        REQUIRE(packet_v2::header(mission_set_current)->len == 2);
        REQUIRE(packet_v2::header(encapsulated_data)->len == 255);
        REQUIRE(packet_v2::header(param_ext_request_list)->len == 2);
    }
    SECTION("Header has incompatibility flags.")
    {
        REQUIRE((packet_v2::header(heartbeat)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == true);
        REQUIRE((packet_v2::header(ping)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == false);
        REQUIRE((packet_v2::header(set_mode)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == true);
        REQUIRE((packet_v2::header(mission_set_current)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == false);
        REQUIRE((packet_v2::header(encapsulated_data)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == true);
        REQUIRE((packet_v2::header(param_ext_request_list)->incompat_flags &
                 MAVLINK_IFLAG_SIGNED) == false);
    }
    SECTION("Header has compatibility flags.")
    {
        REQUIRE(packet_v2::header(heartbeat)->compat_flags == 0);
        REQUIRE(packet_v2::header(ping)->compat_flags == 0);
        REQUIRE(packet_v2::header(set_mode)->compat_flags == 0);
        REQUIRE(packet_v2::header(mission_set_current)->compat_flags == 0);
        REQUIRE(packet_v2::header(encapsulated_data)->compat_flags == 0);
        REQUIRE(packet_v2::header(param_ext_request_list)->compat_flags == 0);
    }
    SECTION("Header has a sequence number.")
    {
        REQUIRE(packet_v2::header(heartbeat)->seq == 0xFD);
        REQUIRE(packet_v2::header(ping)->seq == 0xFD);
        REQUIRE(packet_v2::header(set_mode)->seq == 0xFD);
        REQUIRE(packet_v2::header(mission_set_current)->seq == 0xFD);
        REQUIRE(packet_v2::header(encapsulated_data)->seq == 0xFD);
        REQUIRE(packet_v2::header(param_ext_request_list)->seq == 0xFD);
    }
    SECTION("Header has a system ID.")
    {
        REQUIRE(packet_v2::header(heartbeat)->sysid == 127);
        REQUIRE(packet_v2::header(ping)->sysid == 192);
        REQUIRE(packet_v2::header(set_mode)->sysid == 172);
        REQUIRE(packet_v2::header(mission_set_current)->sysid == 255);
        REQUIRE(packet_v2::header(encapsulated_data)->sysid == 224);
        REQUIRE(packet_v2::header(param_ext_request_list)->sysid == 0);
    }
    SECTION("Header has a component ID.")
    {
        REQUIRE(packet_v2::header(heartbeat)->compid == 1);
        REQUIRE(packet_v2::header(ping)->compid == 168);
        REQUIRE(packet_v2::header(set_mode)->compid == 0);
        REQUIRE(packet_v2::header(mission_set_current)->compid == 0);
        REQUIRE(packet_v2::header(encapsulated_data)->compid == 255);
        REQUIRE(packet_v2::header(param_ext_request_list)->compid == 255);
    }
    SECTION("Header has a message ID.")
    {
        REQUIRE(packet_v2::header(heartbeat)->msgid == 0);
        REQUIRE(packet_v2::header(ping)->msgid == 4);
        REQUIRE(packet_v2::header(set_mode)->msgid == 11);
        REQUIRE(packet_v2::header(mission_set_current)->msgid == 41);
        REQUIRE(packet_v2::header(encapsulated_data)->msgid == 131);
        REQUIRE(packet_v2::header(param_ext_request_list)->msgid == 321);
    }
    SECTION("Returns nullptr when an incomplete header is given.")
    {
        heartbeat.resize(5);
        ping.resize(4);
        set_mode.resize(3);
        mission_set_current.resize(2);
        encapsulated_data.resize(1);
        param_ext_request_list.resize(0);
        REQUIRE(packet_v2::header(heartbeat) == nullptr);
        REQUIRE(packet_v2::header(ping) == nullptr);
        REQUIRE(packet_v2::header(set_mode) == nullptr);
        REQUIRE(packet_v2::header(mission_set_current) == nullptr);
        REQUIRE(packet_v2::header(encapsulated_data) == nullptr);
        REQUIRE(packet_v2::header(param_ext_request_list) == nullptr);
    }
    SECTION("Returns nullptr when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFE;
        mission_set_current.front() = 0xFE;
        encapsulated_data.front() = 0xFE;
        param_ext_request_list.front() = 0xFE;
        REQUIRE(packet_v2::header(heartbeat) == nullptr);
        REQUIRE(packet_v2::header(ping) == nullptr);
        REQUIRE(packet_v2::header(set_mode) == nullptr);
        REQUIRE(packet_v2::header(mission_set_current) == nullptr);
        REQUIRE(packet_v2::header(encapsulated_data) == nullptr);
        REQUIRE(packet_v2::header(param_ext_request_list) == nullptr);
    }
}


TEST_CASE("'packet_v2::packet_complete' determines whether the given bytes "
          "represent a complete packet.", "[packet_v2]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    SECTION("Returns true when a complete packet is given.")
    {
        REQUIRE(packet_v2::packet_complete(heartbeat));
        REQUIRE(packet_v2::packet_complete(ping));
        REQUIRE(packet_v2::packet_complete(set_mode));
        REQUIRE(packet_v2::packet_complete(mission_set_current));
        REQUIRE(packet_v2::packet_complete(encapsulated_data));
        REQUIRE(packet_v2::packet_complete(param_ext_request_list));
    }
    SECTION("Returns false when the magic byte is wrong.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFE;
        mission_set_current.front() = 0xFE;
        encapsulated_data.front() = 0xFE;
        param_ext_request_list.front() = 0xFE;
        REQUIRE_FALSE(packet_v2::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v2::packet_complete(ping));
        REQUIRE_FALSE(packet_v2::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v2::packet_complete(mission_set_current));
        REQUIRE_FALSE(packet_v2::packet_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::packet_complete(param_ext_request_list));
    }
    SECTION("Returns false when the packet is too short.")
    {
        heartbeat.pop_back();
        ping.pop_back();
        set_mode.pop_back();
        mission_set_current.pop_back();
        encapsulated_data.pop_back();
        param_ext_request_list.pop_back();
        REQUIRE_FALSE(packet_v2::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v2::packet_complete(ping));
        REQUIRE_FALSE(packet_v2::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v2::packet_complete(mission_set_current));
        REQUIRE_FALSE(packet_v2::packet_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::packet_complete(param_ext_request_list));
    }
    SECTION("Returns false when the packet is too long.")
    {
        heartbeat.push_back(0x00);
        ping.push_back(0x00);
        set_mode.push_back(0x00);
        mission_set_current.push_back(0x00);
        encapsulated_data.push_back(0x00);
        param_ext_request_list.push_back(0x00);
        REQUIRE_FALSE(packet_v2::packet_complete(heartbeat));
        REQUIRE_FALSE(packet_v2::packet_complete(ping));
        REQUIRE_FALSE(packet_v2::packet_complete(set_mode));
        REQUIRE_FALSE(packet_v2::packet_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::packet_complete(encapsulated_data));
        REQUIRE_FALSE(packet_v2::packet_complete(param_ext_request_list));
    }
}


TEST_CASE("'packet_v2::is_signed' determines whether the given bytes "
          "represent a signed packet.", "[packet_v2]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    SECTION("Returns true when the packet is signed.")
    {
        REQUIRE(packet_v2::is_signed(heartbeat));
        REQUIRE(packet_v2::is_signed(set_mode));
        REQUIRE(packet_v2::is_signed(encapsulated_data));
    }
    SECTION("Returns false when the packet is not signed.")
    {
        REQUIRE_FALSE(packet_v2::is_signed(ping));
        REQUIRE_FALSE(packet_v2::is_signed(mission_set_current));
        REQUIRE_FALSE(packet_v2::is_signed(param_ext_request_list));
    }
    SECTION("Throws and error when the header is invalid.")
    {
        heartbeat.front() = 0xAD;
        ping.front() = 0xBC;
        set_mode.front() = 0xFE;
        mission_set_current.resize(2);
        encapsulated_data.resize(1);
        param_ext_request_list.resize(0);
        // Errors
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(heartbeat), std::invalid_argument);
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(ping), std::invalid_argument);
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(set_mode), std::invalid_argument);
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(mission_set_current), std::invalid_argument);
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(encapsulated_data), std::invalid_argument);
        REQUIRE_THROWS_AS(
            packet_v2::is_signed(param_ext_request_list),
            std::invalid_argument);
        // Error messages.
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(heartbeat),
            "Header is incomplete or invalid.");
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(ping),
            "Header is incomplete or invalid.");
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(set_mode),
            "Header is incomplete or invalid.");
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(mission_set_current),
            "Header is incomplete or invalid.");
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(encapsulated_data),
            "Header is incomplete or invalid.");
        REQUIRE_THROWS_WITH(
            packet_v2::is_signed(param_ext_request_list),
            "Header is incomplete or invalid.");
    }
}


TEST_CASE("packet_v2::Packet's can be constructed.", "[packet_v2::Packet]")
{
    HeartbeatV2 heartbeat;
    PingV2 ping;
    SetModeV2 set_mode;
    MissionSetCurrentV2 mission_set_current;
    EncapsulatedDataV2 encapsulated_data;
    ParamExtRequestListV2 param_ext_request_list;
    SECTION("With proper arguments and no signature.")
    {
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(heartbeat)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(ping)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(set_mode)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(mission_set_current)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(encapsulated_data)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector(param_ext_request_list)));
    }
    SECTION("With proper arguments and a signature.")
    {
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector_with_sig(heartbeat)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector_with_sig(ping)));
        REQUIRE_NOTHROW(packet_v2::Packet(to_vector_with_sig(set_mode)));
        REQUIRE_NOTHROW(
            packet_v2::Packet(to_vector_with_sig(mission_set_current)));
        REQUIRE_NOTHROW(
            packet_v2::Packet(to_vector_with_sig(encapsulated_data)));
        REQUIRE_NOTHROW(
            packet_v2::Packet(to_vector_with_sig(param_ext_request_list)));
    }
    SECTION("And ensures a complete header is given.")
    {
        // 0 length packet.
        REQUIRE_THROWS_AS(packet_v2::Packet({}), std::length_error);
        REQUIRE_THROWS_WITH(packet_v2::Packet({}), "Packet is empty.");
        // Packet short 1 byte.
        REQUIRE_THROWS_AS(
            packet_v2::Packet({0xFD, 2, 3, 4, 5, 6, 7, 8, 9}),
            std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet({0xFD, 2, 3, 4, 5, 6, 7, 8, 9}),
            "Packet (9 bytes) is shorter than a v2.0 header (10 bytes).");
    }
    SECTION("And ensures packets begin with the magic byte (0xFD).")
    {
        ping.magic = 0xAD;
        REQUIRE_THROWS_AS(
            packet_v2::Packet(to_vector(ping)), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(to_vector(ping)),
            "Invalid packet starting byte (0xAD), "
            "v2.0 packets should start with 0xFD.");
    }
    SECTION("And ensures the message ID is valid.")
    {
        ping.msgid = 255; // ID 255 is not currently valid.
        REQUIRE_THROWS_AS(
            packet_v2::Packet(to_vector(ping)), std::runtime_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(to_vector(ping)), "Invalid packet ID (#255).");
        ping.msgid = 5000; // ID 5000 is not currently valid.
        REQUIRE_THROWS_AS(
            packet_v2::Packet(to_vector(ping)), std::runtime_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(to_vector(ping)), "Invalid packet ID (#5000).");
    }
    SECTION("And ensures the packet is the correct length (without signature).")
    {
        // HEARTBEAT (no target system/component).
        auto heartbeat_data = to_vector(heartbeat);
        heartbeat_data.push_back(0x00);
        REQUIRE_THROWS_AS(packet_v2::Packet(heartbeat_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(heartbeat_data),
            "Packet is 22 bytes, should be 21 bytes.");
        // PING (with target system/component).
        auto ping_data = to_vector(ping);
        ping_data.pop_back();
        REQUIRE_THROWS_AS(packet_v2::Packet(ping_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(ping_data),
            "Packet is 25 bytes, should be 26 bytes.");
        // SET_MODE (target system only, no target component).
        auto set_mode_data = to_vector(set_mode);
        set_mode_data.push_back(0x00);
        REQUIRE_THROWS_AS(packet_v2::Packet(set_mode_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(set_mode_data),
            "Packet is 19 bytes, should be 18 bytes.");
        // PARAM_REQUEST_READ (trimmed out, 0.0, system/component).
        auto mission_set_current_data = to_vector(mission_set_current);
        mission_set_current_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v2::Packet(mission_set_current_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(mission_set_current_data),
            "Packet is 13 bytes, should be 14 bytes.");
        // ENCAPSULATED_DATA (longest packet).
        auto encapsulated_data_data = to_vector(encapsulated_data);
        encapsulated_data_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            packet_v2::Packet(encapsulated_data_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(encapsulated_data_data),
            "Packet is 268 bytes, should be 267 bytes.");
        // PARAM_EXT_REQUEST_LIST (message ID beyond 255).
        auto param_ext_request_list_data = to_vector(param_ext_request_list);
        param_ext_request_list_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v2::Packet(param_ext_request_list_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(param_ext_request_list_data),
            "Packet is 13 bytes, should be 14 bytes.");
    }
    SECTION("And ensures the packet is the correct length (with signature).")
    {
        // HEARTBEAT (no target system/component).
        auto heartbeat_data = to_vector_with_sig(heartbeat);
        heartbeat_data.push_back(0x00);
        REQUIRE_THROWS_AS(packet_v2::Packet(heartbeat_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(heartbeat_data),
            "Signed packet is 35 bytes, should be 34 bytes.");
        // PING (with target system/component).
        auto ping_data = to_vector_with_sig(ping);
        ping_data.pop_back();
        REQUIRE_THROWS_AS(packet_v2::Packet(ping_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(ping_data),
            "Signed packet is 38 bytes, should be 39 bytes.");
        // SET_MODE (target system only, no target component).
        auto set_mode_data = to_vector_with_sig(set_mode);
        set_mode_data.push_back(0x00);
        REQUIRE_THROWS_AS(packet_v2::Packet(set_mode_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(set_mode_data),
            "Signed packet is 32 bytes, should be 31 bytes.");
        // PARAM_REQUEST_READ (trimmed out, 0.0, system/component).
        auto mission_set_current_data = to_vector_with_sig(mission_set_current);
        mission_set_current_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v2::Packet(mission_set_current_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(mission_set_current_data),
            "Signed packet is 26 bytes, should be 27 bytes.");
        // ENCAPSULATED_DATA (longest packet).
        auto encapsulated_data_data = to_vector_with_sig(encapsulated_data);
        encapsulated_data_data.push_back(0x00);
        REQUIRE_THROWS_AS(
            packet_v2::Packet(encapsulated_data_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(encapsulated_data_data),
            "Signed packet is 281 bytes, should be 280 bytes.");
        // PARAM_EXT_REQUEST_LIST (message ID beyond 255).
        auto param_ext_request_list_data =
            to_vector_with_sig(param_ext_request_list);
        param_ext_request_list_data.pop_back();
        REQUIRE_THROWS_AS(
            packet_v2::Packet(param_ext_request_list_data), std::length_error);
        REQUIRE_THROWS_WITH(
            packet_v2::Packet(param_ext_request_list_data),
            "Signed packet is 26 bytes, should be 27 bytes.");
    }
}


TEST_CASE("packet_v2::Packet's are comparable.", "[packet_v2::Packet]")
{
    SECTION("with ==")
    {
        REQUIRE(
            packet_v2::Packet(to_vector(PingV2())) ==
            packet_v2::Packet(to_vector(PingV2())));
        REQUIRE_FALSE(
            packet_v2::Packet(to_vector(PingV2())) ==
            packet_v2::Packet(to_vector(SetModeV2())));
        REQUIRE_FALSE(
            packet_v2::Packet(to_vector(PingV2())) ==
            packet_v2::Packet(to_vector_with_sig(PingV2())));
    }
    SECTION("with !=")
    {
        REQUIRE(
            packet_v2::Packet(to_vector(PingV2())) !=
            packet_v2::Packet(to_vector(SetModeV2())));
        REQUIRE(
            packet_v2::Packet(to_vector(PingV2())) !=
            packet_v2::Packet(to_vector_with_sig(PingV2())));
        REQUIRE_FALSE(
            packet_v2::Packet(to_vector(PingV2())) !=
            packet_v2::Packet(to_vector(PingV2())));
    }
}


TEST_CASE("packet_v2::Packet's are copyable.", "[packet_v2::Packet]")
{
    packet_v2::Packet original(to_vector(PingV2()));
    packet_v2::Packet copy(original);
    REQUIRE(copy == packet_v2::Packet(to_vector(PingV2())));
}


TEST_CASE("packet_v2::Packet's are movable.", "[packet_v2::Packet]")
{
    packet_v2::Packet original(to_vector(PingV2()));
    packet_v2::Packet moved(std::move(original));
    REQUIRE(moved == packet_v2::Packet(to_vector(PingV2())));
}


TEST_CASE("packet_v2::Packet's are assignable.", "[Packet]")
{
    packet_v2::Packet a(to_vector(PingV2()));
    packet_v2::Packet b(to_vector(SetModeV2()));
    REQUIRE(a == packet_v2::Packet(to_vector(PingV2())));
    a = b;
    REQUIRE(b == packet_v2::Packet(to_vector(SetModeV2())));
}


TEST_CASE("packet_v2::Packet's are assignable (by move semantics).", "[Packet]")
{
    packet_v2::Packet a(to_vector(PingV2()));
    packet_v2::Packet b(to_vector(SetModeV2()));
    REQUIRE(a == packet_v2::Packet(to_vector(PingV2())));
    a = b;
    REQUIRE(b == packet_v2::Packet(to_vector(SetModeV2())));
}


TEST_CASE("packet_v2::Packet's contain raw packet data and make it accessible.",
          "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector_with_sig(ParamExtRequestListV2());
    REQUIRE(packet_v2::Packet(heartbeat).data() == heartbeat);
    REQUIRE(packet_v2::Packet(ping).data() == ping);
    REQUIRE(packet_v2::Packet(set_mode).data() == set_mode);
    REQUIRE(
        packet_v2::Packet(mission_set_current).data() == mission_set_current);
    REQUIRE(
        packet_v2::Packet(encapsulated_data).data() == encapsulated_data);
    REQUIRE(
        packet_v2::Packet(param_ext_request_list).data() ==
        param_ext_request_list);
}


TEST_CASE("packet_v2::Packet's have a version.", "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    // All should read Packet::V2 (0x0200) for v2.0.
    REQUIRE(packet_v2::Packet(heartbeat).version() == Packet::V2);
    REQUIRE(packet_v2::Packet(ping).version() == Packet::V2);
    REQUIRE(packet_v2::Packet(set_mode).version() == Packet::V2);
    REQUIRE(packet_v2::Packet(mission_set_current).version() == 0x200);
    REQUIRE(packet_v2::Packet(encapsulated_data).version() == 0x200);
    REQUIRE(packet_v2::Packet(param_ext_request_list).version() == 0x200);
}


TEST_CASE("packet_v2::Packet's have an ID.", "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    REQUIRE(packet_v2::Packet(heartbeat).id() == 0);
    REQUIRE(packet_v2::Packet(ping).id() == 4);
    REQUIRE(packet_v2::Packet(set_mode).id() == 11);
    REQUIRE(packet_v2::Packet(mission_set_current).id() == 41);
    REQUIRE(packet_v2::Packet(encapsulated_data).id() == 131);
    REQUIRE(packet_v2::Packet(param_ext_request_list).id() == 321);
}


TEST_CASE("packet_v2::Packet's have a name.", "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    REQUIRE(packet_v2::Packet(heartbeat).name() == "HEARTBEAT");
    REQUIRE(packet_v2::Packet(ping).name() == "PING");
    REQUIRE(packet_v2::Packet(set_mode).name() == "SET_MODE");
    REQUIRE(
        packet_v2::Packet(mission_set_current).name() == "MISSION_SET_CURRENT");
    REQUIRE(packet_v2::Packet(encapsulated_data).name() == "ENCAPSULATED_DATA");
    REQUIRE(
        packet_v2::Packet(param_ext_request_list).name() ==
        "PARAM_EXT_REQUEST_LIST");
}


TEST_CASE("packet_v2::Packet's have a source address.", "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    REQUIRE(packet_v2::Packet(heartbeat).source() == MAVAddress("127.1"));
    REQUIRE(packet_v2::Packet(ping).source() == MAVAddress("192.168"));
    REQUIRE(packet_v2::Packet(set_mode).source() == MAVAddress("172.0"));
    REQUIRE(
        packet_v2::Packet(mission_set_current).source() == MAVAddress("255.0"));
    REQUIRE(
        packet_v2::Packet(encapsulated_data).source() == MAVAddress("224.255"));
    REQUIRE(
        packet_v2::Packet(param_ext_request_list).source() ==
        MAVAddress("0.255"));
}


TEST_CASE("packet_v2::Packet's optionally have a destination address.",
          "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    REQUIRE_THROWS_AS(
        packet_v2::Packet(heartbeat).dest().value(), std::bad_optional_access);
    REQUIRE(packet_v2::Packet(ping).dest().value() == MAVAddress("127.1"));
    REQUIRE(packet_v2::Packet(set_mode).dest().value() == MAVAddress("123.0"));
    REQUIRE(
        packet_v2::Packet(mission_set_current).dest().value() ==
        MAVAddress("0.0"));
    REQUIRE_THROWS_AS(
        packet_v2::Packet(encapsulated_data).dest().value(),
        std::bad_optional_access);
    REQUIRE(
        packet_v2::Packet(param_ext_request_list).dest().value() ==
        MAVAddress("32.64"));
}


TEST_CASE("packet_v2::Packet's are printable.", "[packet_v2::Packet]")
{
    auto heartbeat = to_vector_with_sig(HeartbeatV2());
    auto ping = to_vector(PingV2());
    auto set_mode = to_vector_with_sig(SetModeV2());
    auto mission_set_current = to_vector(MissionSetCurrentV2());
    auto encapsulated_data = to_vector_with_sig(EncapsulatedDataV2());
    auto param_ext_request_list = to_vector(ParamExtRequestListV2());
    REQUIRE(
        str(packet_v2::Packet(heartbeat)) ==
        "HEARTBEAT (#0) from 127.1 (v2.0)");
    REQUIRE(
        str(packet_v2::Packet(ping)) ==
        "PING (#4) from 192.168 to 127.1 (v2.0)");
    REQUIRE(
        str(packet_v2::Packet(set_mode)) ==
        "SET_MODE (#11) from 172.0 to 123.0 (v2.0)");
    REQUIRE(
        str(packet_v2::Packet(mission_set_current)) ==
        "MISSION_SET_CURRENT (#41) from 255.0 to 0.0 (v2.0)");
    REQUIRE(
        str(packet_v2::Packet(encapsulated_data)) ==
        "ENCAPSULATED_DATA (#131) from 224.255 (v2.0)");
    REQUIRE(
        str(packet_v2::Packet(param_ext_request_list)) ==
        "PARAM_EXT_REQUEST_LIST (#321) from 0.255 to 32.64 (v2.0)");
}
