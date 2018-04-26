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


#include <limits>
#include <utility>

#include <catch.hpp>

#include "PacketVersion2.hpp"
#include "QueuedPacket.hpp"
#include "utility.hpp"

#include "common_Packet.hpp"



TEST_CASE("QueuedPacket's can be constructed.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    REQUIRE_NOTHROW(QueuedPacket(packet, 3, 10));
    REQUIRE_THROWS_AS(QueuedPacket(nullptr, 3, 10), std::invalid_argument);
    REQUIRE_THROWS_WITH(
        QueuedPacket(nullptr, 3, 10), "Given packet pointer is null.");
}


TEST_CASE("QueuedPacket' are comparable.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    SECTION("with ==")
    {
        REQUIRE(QueuedPacket(packet, 3, 10) == QueuedPacket(packet, 3, 10));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) == QueuedPacket(packet, 0, 10));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) == QueuedPacket(packet, 3, 100));
    }
    SECTION("with !=")
    {
        REQUIRE(QueuedPacket(packet, 3, 10) != QueuedPacket(packet, 0, 10));
        REQUIRE(QueuedPacket(packet, 3, 10) != QueuedPacket(packet, 3, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) != QueuedPacket(packet, 3, 10));
    }
    SECTION("with <")
    {
        // Priority takes precidence.
        REQUIRE(
            QueuedPacket(packet, 0, 10) < QueuedPacket(packet, 3, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 100) < QueuedPacket(packet, 0, 10));
        // Ticket number.
        REQUIRE(
            QueuedPacket(packet, 3, 100) < QueuedPacket(packet, 3, 10));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) < QueuedPacket(packet, 3, 100));
        // Ticket number, with rollover.
        REQUIRE(
            QueuedPacket(packet, 3, 0) <
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2 + 1));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 0) <
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2));
        // Not equal.
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) < QueuedPacket(packet, 3, 10));
    }
    SECTION("with >")
    {
        // Priority takes precidence.
        REQUIRE(
            QueuedPacket(packet, 3, 10) > QueuedPacket(packet, 0, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 0, 100) > QueuedPacket(packet, 3, 10));
        // Ticket number.
        REQUIRE(
            QueuedPacket(packet, 3, 10) > QueuedPacket(packet, 3, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 100) > QueuedPacket(packet, 3, 10));
        // Ticket number, with rollover.
        REQUIRE(
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2 + 1) >
            QueuedPacket(packet, 3, 0));
        REQUIRE_FALSE(
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2) >
            QueuedPacket(packet, 3, 0));
        // Not equal.
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) > QueuedPacket(packet, 3, 10));
    }
    SECTION("with <=")
    {
        // Equality.
        REQUIRE(QueuedPacket(packet, 3, 10) <= QueuedPacket(packet, 3, 10));
        // Priority takes precidence.
        REQUIRE(
            QueuedPacket(packet, 0, 10) <= QueuedPacket(packet, 3, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 100) <= QueuedPacket(packet, 0, 10));
        // Ticket number.
        REQUIRE(
            QueuedPacket(packet, 3, 100) <= QueuedPacket(packet, 3, 10));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 10) <= QueuedPacket(packet, 3, 100));
        // Ticket number, with rollover.
        REQUIRE(
            QueuedPacket(packet, 3, 0) <=
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2 + 1));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 0) <=
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2));
    }
    SECTION("with >=")
    {
        // Equality.
        REQUIRE(QueuedPacket(packet, 3, 10) >= QueuedPacket(packet, 3, 10));
        // Priority takes precidence.
        REQUIRE(
            QueuedPacket(packet, 3, 10) >= QueuedPacket(packet, 0, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 0, 100) >= QueuedPacket(packet, 3, 10));
        // Ticket number.
        REQUIRE(
            QueuedPacket(packet, 3, 10) >= QueuedPacket(packet, 3, 100));
        REQUIRE_FALSE(
            QueuedPacket(packet, 3, 100) >= QueuedPacket(packet, 3, 10));
        // Ticket number, with rollover.
        REQUIRE(
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2 + 1) >=
            QueuedPacket(packet, 3, 0));
        REQUIRE_FALSE(
            QueuedPacket(
                packet, 3,
                std::numeric_limits<unsigned long long>::max() / 2) >=
            QueuedPacket(packet, 3, 0));
    }
}


TEST_CASE("QueuedPacket's are copyable.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto original = QueuedPacket(packet, 3, 10);
    auto copy(original);
    REQUIRE(copy == QueuedPacket(packet, 3, 10));
}


TEST_CASE("QueuedPacket's are movable.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto original = QueuedPacket(packet, 3, 10);
    auto moved(std::move(original));
    REQUIRE(moved == QueuedPacket(packet, 3, 10));
}


TEST_CASE("QueuedPacket's are assignable.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto a = QueuedPacket(packet, 3, 10);
    auto b = QueuedPacket(packet, 10, 3);
    REQUIRE(a == QueuedPacket(packet, 3, 10));
    a = b;
    REQUIRE(a == QueuedPacket(packet, 10, 3));
}


TEST_CASE("QueuedPacket's are assignable (by move semantics).",
          "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    auto a = QueuedPacket(packet, 3, 10);
    auto b = QueuedPacket(packet, 10, 3);
    REQUIRE(a == QueuedPacket(packet, 3, 10));
    a = std::move(b);
    REQUIRE(a == QueuedPacket(packet, 10, 3));
}


TEST_CASE("QueuedPacket's 'packet' method returns the contained MAVLink packet",
          "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    REQUIRE(*QueuedPacket(packet, 3, 10).packet() == *packet);
}


TEST_CASE("QueuedPacket's are printable.", "[QueuedPacket]")
{
    auto packet = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    REQUIRE(
        str(QueuedPacket(packet, -10, 1)) ==
        "PING (#4) from 192.168 to 127.1 (v2.0) with priority -10");
    REQUIRE(
        str(QueuedPacket(packet, 0, 1)) ==
        "PING (#4) from 192.168 to 127.1 (v2.0) with priority 0");
    REQUIRE(
        str(QueuedPacket(packet, 3, 10)) ==
        "PING (#4) from 192.168 to 127.1 (v2.0) with priority 3");
    REQUIRE(
        str(QueuedPacket(packet, 10, 3)) ==
        "PING (#4) from 192.168 to 127.1 (v2.0) with priority 10");
}
