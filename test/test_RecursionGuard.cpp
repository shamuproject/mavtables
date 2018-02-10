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

#include "RecursionError.hpp"
#include "RecursionGuard.hpp"
#include "RecursionGuardData.hpp"


TEST_CASE("RecursionGuard's are constructable.", "[RecursionGuard]")
{
    RecursionGuardData rg_data;
    REQUIRE_NOTHROW(RecursionGuard(rg_data));
}


TEST_CASE("RecusionGuard's prevent recursion within a single thread.", 
          "[RecursionGuard]")
{
    SECTION("No recursion.")
    {
        RecursionGuardData rg_data;
        {
            REQUIRE_NOTHROW(RecursionGuard(rg_data));
        }
        {
            REQUIRE_NOTHROW(RecursionGuard(rg_data));
        }
        {
            REQUIRE_NOTHROW(RecursionGuard(rg_data));
        }
    }
    SECTION("Throws an error on recursion.")
    {
        RecursionGuardData rg_data;
        RecursionGuard rg(rg_data);
        REQUIRE_THROWS_AS(RecursionGuard(rg_data), RecursionError);
        REQUIRE_THROWS_WITH(RecursionGuard(rg_data), "Recursion detected.");
    }
    SECTION("Is thread safe (calls across threads is not recursion).")
    {
        RecursionGuardData rg_data;
        RecursionGuard rg(rg_data);
        std::thread thread([&](){RecursionGuard rg_(rg_data);});
        thread.join();
    }
}
