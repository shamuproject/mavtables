// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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

#include "MAVAddress.hpp"
#include "MAVSubnet.hpp"


TEST_CASE("MAVSubnets's are comparable.", "[MAVSubnet]")
{
    SECTION("with ==")
    {
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) == MAVSubnet(MAVAddress(0x0000), 0x0000));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) == MAVSubnet(MAVAddress(0x1234), 0x5678));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) == MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));

        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) == MAVSubnet(MAVAddress(0x0001), 0x0000));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) == MAVSubnet(MAVAddress(0x0000), 0x0001));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) == MAVSubnet(MAVAddress(0x1235), 0x5678));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) == MAVSubnet(MAVAddress(0x1234), 0x5679));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) == MAVSubnet(MAVAddress(0xFFFE), 0xFFFF));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) == MAVSubnet(MAVAddress(0xFFFF), 0xFFFE));
    }

    SECTION("with !=")
    {
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) != MAVSubnet(MAVAddress(0x0001), 0x0000));
        REQUIRE(MAVSubnet(MAVAddress(0x0000), 0x0000) != MAVSubnet(MAVAddress(0x0000), 0x0001));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) != MAVSubnet(MAVAddress(0x1235), 0x5678));
        REQUIRE(MAVSubnet(MAVAddress(0x1234), 0x5678) != MAVSubnet(MAVAddress(0x1234), 0x5679));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) != MAVSubnet(MAVAddress(0xFFFE), 0xFFFF));
        REQUIRE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) != MAVSubnet(MAVAddress(0xFFFF), 0xFFFE));

        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x0000), 0x0000) != MAVSubnet(MAVAddress(0x0000), 0x0000));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0x1234), 0x5678) != MAVSubnet(MAVAddress(0x1234), 0x5678));
        REQUIRE_FALSE(MAVSubnet(MAVAddress(0xFFFF), 0xFFFF) != MAVSubnet(MAVAddress(0xFFFF), 0xFFFF));
    }
}
