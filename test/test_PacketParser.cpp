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


#include <vector>
#include <memory>
#include <utility>
#include <cstdint>
#include <optional>
#include <stdexcept>

#include <catch.hpp>
#include <boost/range/irange.hpp>

#include "PacketVersion1.hpp"
#include "PacketVersion2.hpp"
#include "PacketParser.hpp"
#include "Connection.hpp"

#include "common_Packet.hpp"

namespace
{

    // Forward defines of locatl functions.
    std::unique_ptr<Packet> test_packet_parser(PacketParser &parser,
            const std::vector<uint8_t> &data, size_t packet_end);
    void add_bytes(std::vector<uint8_t> &data, size_t num_bytes);


    /** Test a PacketParser instance on data.
     *
     *  Uses the REQUIRE macro from Catch C++ internally for testing.
     *
     *  \param parser The instance of the packet parser to use.
     *  \param data The data to give to the parser.
     *  \param packet_end The byte (1 indexed) that the packet is supposed to
     *      end on.
     *
     *  \returns The packet parsed from the given \p data.
     */
    std::unique_ptr<Packet> test_packet_parser(PacketParser &parser,
            const std::vector<uint8_t> &data, size_t packet_end)
    {
        auto conn = std::make_shared<ConnectionTestClass>();
        std::unique_ptr<Packet> packet;

        for (auto i : boost::irange(static_cast<size_t>(0), packet_end - 1))
        {
            REQUIRE(parser.parse_byte(data[i]) == nullptr);
        }

        REQUIRE((packet = parser.parse_byte(data[packet_end - 1])) != nullptr);

        for (auto i : boost::irange(packet_end, data.size()))
        {
            REQUIRE(parser.parse_byte(data[i]) == nullptr);
        }

        return packet;
    }


    /** Add bytes to either side of a vector.
     *
     *  Used for testing.
     *
     *  \param data A reference to the vector to modify.
     *  \param num_bytes Number of bytes to add on either end the \p data
     *      vector.
     */
    void add_bytes(std::vector<uint8_t> &data, size_t num_bytes)
    {
        for (auto i : boost::irange(static_cast<size_t>(0), num_bytes))
        {
            data.insert(data.begin(), static_cast<uint8_t>(i));
        }

        for (auto i : boost::irange(static_cast<size_t>(0), num_bytes))
        {
            data.push_back(static_cast<uint8_t>(i));
        }
    }

}


TEST_CASE("PacketParser's can be constructed.", "[PacketParser]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    REQUIRE_NOTHROW(PacketParser(conn));
    REQUIRE_NOTHROW(PacketParser(conn, -10));
    REQUIRE_NOTHROW(PacketParser(conn, +10));
}


TEST_CASE("PacketParser's can parse packets with 'parse_byte'.",
          "[PacketParser]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketParser parser(conn);
    SECTION("Can parse v1.0 packets.")
    {
        auto data = to_vector(PingV1());
        add_bytes(data, 3);
        auto packet = test_packet_parser(parser, data, sizeof(PingV1) + 3);
        REQUIRE(packet->data() == to_vector(PingV1()));
    }
    SECTION("Can parse v2.0 packets.")
    {
        auto data = to_vector(PingV2());
        add_bytes(data, 3);
        auto packet = test_packet_parser(parser, data, sizeof(PingV2) + 3);
        REQUIRE(packet->data() == to_vector(PingV2()));
    }
    SECTION("Can parse v2.0 packets with signature.")
    {
        auto data = to_vector_with_sig(PingV2());
        add_bytes(data, 3);
        auto packet = test_packet_parser(
                          parser, data,
                          sizeof(PingV2) + packet_v2::SIGNATURE_LENGTH + 3);
        REQUIRE(packet->data() == to_vector_with_sig(PingV2()));
    }
    SECTION("Can parse multiple packets back to back.")
    {
        auto data1 = to_vector(PingV1());
        auto data2 = to_vector(PingV2());
        auto data3 = to_vector_with_sig(PingV2());
        auto packet1 = test_packet_parser(parser, data1, sizeof(PingV1));
        auto packet2 = test_packet_parser(parser, data2, sizeof(PingV2));
        auto packet3 = test_packet_parser(
                           parser, data3,
                           sizeof(PingV2) + packet_v2::SIGNATURE_LENGTH);
        REQUIRE(packet1->data() == to_vector(PingV1()));
        REQUIRE(packet2->data() == to_vector(PingV2()));
        REQUIRE(packet3->data() == to_vector_with_sig(PingV2()));
    }
    SECTION("Can parse multiple packets with random bytes interspersed.")
    {
        auto data1 = to_vector(PingV1());
        auto data2 = to_vector(PingV2());
        auto data3 = to_vector_with_sig(PingV2());
        add_bytes(data1, 3);
        add_bytes(data2, 3);
        add_bytes(data3, 3);
        auto packet1 = test_packet_parser(parser, data1, sizeof(PingV1) + 3);
        auto packet2 = test_packet_parser(parser, data2, sizeof(PingV2) + 3);
        auto packet3 = test_packet_parser(
                           parser, data3,
                           sizeof(PingV2) + packet_v2::SIGNATURE_LENGTH + 3);
        REQUIRE(packet1->data() == to_vector(PingV1()));
        REQUIRE(packet2->data() == to_vector(PingV2()));
        REQUIRE(packet3->data() == to_vector_with_sig(PingV2()));
    }
}


TEST_CASE("PacketParser's can be cleared with 'clear'.", "[PacketParser]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketParser parser(conn);
    parser.parse_byte(0xFE);
    parser.parse_byte(14);
    parser.parse_byte(3);
    parser.clear();
    auto data = to_vector(PingV1());
    auto packet = test_packet_parser(parser, data, sizeof(PingV1));
    REQUIRE(packet->data() == to_vector(PingV1()));
}


TEST_CASE("PacketParser's keep track of how many bytes they have parsed of "
          "the current packet.", "[PacketParser]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    PacketParser parser(conn);
    REQUIRE(parser.bytes_parsed() == 0);
    parser.parse_byte(1);
    REQUIRE(parser.bytes_parsed() == 0);
    parser.parse_byte(2);
    REQUIRE(parser.bytes_parsed() == 0);
    parser.parse_byte(3);
    REQUIRE(parser.bytes_parsed() == 0);
    parser.parse_byte(0xFD);
    REQUIRE(parser.bytes_parsed() == 1);
    parser.parse_byte(14);
    REQUIRE(parser.bytes_parsed() == 2);
    parser.parse_byte(3);
    REQUIRE(parser.bytes_parsed() == 3);
    parser.clear();
    REQUIRE(parser.bytes_parsed() == 0);
    parser.parse_byte(1);
    REQUIRE(parser.bytes_parsed() == 0);
}


TEST_CASE("PacketParser's construct packets with a given weak_ptr to a "
          "connection.", "[PacketParser]")
{
    auto conn1 = std::make_shared<ConnectionTestClass>();
    auto conn2 = std::make_shared<ConnectionTestClass>();
    PacketParser parser1(conn1);
    PacketParser parser2(conn2);
    auto data = to_vector(PingV1());
    auto packet1 = test_packet_parser(parser1, data, sizeof(PingV1));
    auto packet2 = test_packet_parser(parser2, data, sizeof(PingV1));
    REQUIRE(packet1->connection().lock() != nullptr);
    REQUIRE(packet1->connection().lock() == conn1);
    REQUIRE(packet1->connection().lock() != conn2);
    REQUIRE(packet2->connection().lock() != nullptr);
    REQUIRE(packet2->connection().lock() != conn1);
    REQUIRE(packet2->connection().lock() == conn2);
}


TEST_CASE("PacketParser's construct packets with a priority.", "[PacketParser]")
{
    auto conn = std::make_shared<ConnectionTestClass>();
    auto data = to_vector(PingV1());
    SECTION("The priority can be given in the constructor.")
    {
        PacketParser parser(conn, 11);
        auto packet = test_packet_parser(parser, data, sizeof(PingV1));
        REQUIRE(packet->priority() == 11);
    }
    SECTION("The default priority is 0.")
    {
        PacketParser parser(conn);
        auto packet = test_packet_parser(parser, data, sizeof(PingV1));
        REQUIRE(packet->priority() == 0);
    }
}