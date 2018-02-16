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
#include <chrono>

#include <catch.hpp>

#include <Connection.hpp>
#include <MAVAddress.hpp>
#include "Filter.hpp"

using namespace std::chrono_literals;


// TEST_CASE("Connection's can be constructed.", "[ConnectionPool]")
// {
//     auto filter = std::make_shared<Filter>(Chain("test_chain"));
//     REQUIRE_NOTHROW(Connection(filter));
// }
//
//
// TEST_CASE("Test stuff.", "[ConnectionPool]")
// {
//     auto filter = std::make_shared<Filter>(Chain("test_chain"));
//     Connection conn(filter);
//     conn.add_address(MAVAddress("192.168"));
//     // std::this_thread::sleep_for(2s);
//     conn.add_address(MAVAddress("172.16"));
//     // std::this_thread::sleep_for(2s);
//     conn.add_address(MAVAddress("10.10"));
// }
