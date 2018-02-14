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


#include <chrono>

#include <catch.hpp>
#include <fake_clock.hh>

#include <AddressPool.hpp>


using namespace std::chrono_literals;
using namespace testing;


TEST_CASE("AddressPool's can be constructed.", "[AddressPool]")
{
    REQUIRE_NOTHROW(AddressPool<>());
    REQUIRE_NOTHROW(AddressPool<>(10s));
}


TEST_CASE("AddressPool's", "[AddressPool]")
{
   fake_clock::time_point t1 = fake_clock::now();
   fake_clock::advance(std::chrono::milliseconds(100));
   fake_clock::time_point t2 = fake_clock::now();
   auto elapsed_us = std::chrono::duration_cast<
          std::chrono::microseconds>(t2 - t1).count();
   REQUIRE(100000 == elapsed_us);
}
