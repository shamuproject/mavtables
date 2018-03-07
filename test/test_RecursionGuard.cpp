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


#include <thread>

#include <catch.hpp>

#include "RecursionData.hpp"
#include "RecursionError.hpp"
#include "RecursionGuard.hpp"


TEST_CASE("RecursionGuard's are constructable.", "[RecursionGuard]")
{
    RecursionData rdata;
    REQUIRE_NOTHROW(RecursionGuard(rdata));
}


TEST_CASE(
    "RecusionGuard's prevent recursion within a single thread.",
    "[RecursionGuard]")
{
    SECTION("No recursion.")
    {
        RecursionData rdata;
        {
            REQUIRE_NOTHROW(RecursionGuard(rdata));
        }
        {
            REQUIRE_NOTHROW(RecursionGuard(rdata));
        }
        {
            REQUIRE_NOTHROW(RecursionGuard(rdata));
        }
    }
    SECTION("Throws an error on recursion.")
    {
        RecursionData rdata;
        RecursionGuard rg(rdata);
        REQUIRE_THROWS_AS(RecursionGuard(rdata), RecursionError);
        REQUIRE_THROWS_WITH(RecursionGuard(rdata), "Recursion detected.");
    }
    SECTION("Is thread safe (calls across threads is not recursion).")
    {
        RecursionData rdata;
        RecursionGuard rguard(rdata);
        std::thread thread([&]() { RecursionGuard rguard_(rdata); });
        thread.join();
    }
}
