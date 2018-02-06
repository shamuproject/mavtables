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


#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <catch.hpp>

#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "util.hpp"

#include "common_Packet.hpp"


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Subclass of Packet used for testing the abstract class Packet.
    class PacketTestClass : public Packet
    {
        public:
            PacketTestClass(const PacketTestClass &other) = default;
            PacketTestClass(PacketTestClass &&other) = default;
            PacketTestClass(std::vector<uint8_t> data)
                : Packet(std::move(data))
            {
            }
            virtual ::Packet::Version version() const
            {
                return ::Packet::V1;
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
            PacketTestClass &operator=(PacketTestClass &&other) = default;
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}


TEST_CASE("Packet's can be constructed.", "[Packet]")
{
    std::vector<uint8_t> data = {0, 7, 7, 3, 4};
    REQUIRE_NOTHROW(PacketTestClass({}));
    REQUIRE_NOTHROW(PacketTestClass({1, 3, 3, 7}));
    REQUIRE_NOTHROW(PacketTestClass(data));
}


TEST_CASE("Packet's are comparable.", "[Packet]")
{
    SECTION("with ==")
    {
        REQUIRE(PacketTestClass({1, 3, 3, 7}) == PacketTestClass({1, 3, 3, 7}));
        REQUIRE_FALSE(
            PacketTestClass({1, 3, 3, 7}) == PacketTestClass({0, 7, 7, 3, 4}));
    }
    SECTION("with !=")
    {
        REQUIRE(
            PacketTestClass({1, 3, 3, 7}) != PacketTestClass({0, 7, 7, 3, 4}));
        REQUIRE_FALSE(
            PacketTestClass({1, 3, 3, 7}) != PacketTestClass({1, 3, 3, 7}));
    }
}


TEST_CASE("Packet's are copyable.", "[Packet]")
{
    PacketTestClass original({1, 3, 3, 7});
    PacketTestClass copy(original);
    REQUIRE(copy == PacketTestClass({1, 3, 3, 7}));
}


TEST_CASE("Packet's are movable.", "[Packet]")
{
    PacketTestClass original({1, 3, 3, 7});
    PacketTestClass moved(std::move(original));
    REQUIRE(moved == PacketTestClass({1, 3, 3, 7}));
}


TEST_CASE("Packet's are assignable.", "[Packet]")
{
    PacketTestClass a({1, 3, 3, 7});
    PacketTestClass b({0, 7, 7, 3, 4});
    REQUIRE(a == PacketTestClass({1, 3, 3, 7}));
    a = b;
    REQUIRE(a == PacketTestClass({0, 7, 7, 3, 4}));
}


TEST_CASE("Packet's are assignable (by move semantics).", "[Packet]")
{
    PacketTestClass a({1, 3, 3, 7});
    PacketTestClass b({0, 7, 7, 3, 4});
    REQUIRE(a == PacketTestClass({1, 3, 3, 7}));
    a = std::move(b);
    REQUIRE(a == PacketTestClass({0, 7, 7, 3, 4}));
}


TEST_CASE("Packet's contain raw packet data and make it accessible.",
          "[Packet]")
{
    REQUIRE(
        PacketTestClass({1, 3, 3, 7}).data() ==
        std::vector<uint8_t>({1, 3, 3, 7}));
    REQUIRE_FALSE(
        PacketTestClass({1, 3, 3, 7}).data() ==
        std::vector<uint8_t>({1, 0, 5, 3}));
}


TEST_CASE("Packet's have a version.", "[Packet]")
{
    REQUIRE(PacketTestClass({}).version() == Packet::V1);
}


TEST_CASE("Packet's have an ID.", "[Packet]")
{
    REQUIRE(PacketTestClass({}).id() == 42);
}


TEST_CASE("Packet's have a name.", "[Packet]")
{
    REQUIRE(PacketTestClass({}).name() == "MISSION_CURRENT");
}


TEST_CASE("Packet's have a source address.", "[Packet]")
{
    REQUIRE(PacketTestClass({}).source() == MAVAddress("3.14"));
}


TEST_CASE("Packet's optionally have a destination address.", "[Packet]")
{
    REQUIRE(PacketTestClass({}).dest().value() == MAVAddress("2.71"));
}


TEST_CASE("Packet's are printable.", "[Packet]")
{
    REQUIRE(
        str(PacketTestClass({})) ==
        "MISSION_CURRENT (#42) from 3.14 to 2.71 (v1.0)");
}
