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
#include <stdexcept>

#include <catch.hpp>
#include <fakeit.hpp>

#include "ConfigFileParser.hpp"
#include "util.hpp"


TEST_CASE("ConfigFileParser can parse a file.")
{
    SECTION("When the file exists and is valid.")
    {
        REQUIRE_NOTHROW(ConfigFileParser("examples/test.conf"));
    }
    SECTION("Throws an error if the file does not exist.")
    {
        REQUIRE_THROWS(
            ConfigFileParser("examples/file_that_does_not_exist.conf"));
    }
}


TEST_CASE("ConfigFileParser are printable.")
{
    auto config = std::make_unique<ConfigFileParser>("examples/test.conf");
    REQUIRE(config != nullptr);
    REQUIRE(
        str(*config) ==
        "examples/test.conf:001:  default_action\n"
        "examples/test.conf:001:  |  accept\n"
        "examples/test.conf:004:  udp\n"
        "examples/test.conf:005:  |  port 14500\n"
        "examples/test.conf:006:  |  address 127.0.0.1\n"
        "examples/test.conf:010:  serial\n"
        "examples/test.conf:011:  |  device /dev/ttyUSB0\n"
        "examples/test.conf:012:  |  baudrate 115200\n"
        "examples/test.conf:013:  |  flow_control yes\n"
        "examples/test.conf:017:  serial\n"
        "examples/test.conf:018:  |  device COM1\n"
        "examples/test.conf:019:  |  baudrate 9600\n"
        "examples/test.conf:020:  |  flow_control no\n"
        "examples/test.conf:025:  chain default\n"
        "examples/test.conf:027:  |  call some_chain10\n"
        "examples/test.conf:027:  |  |  condition\n"
        "examples/test.conf:027:  |  |  |  source 127.1\n"
        "examples/test.conf:027:  |  |  |  dest 192.0\n"
        "examples/test.conf:028:  |  reject\n"
        "examples/test.conf:033:  chain some_chain10\n"
        "examples/test.conf:035:  |  accept\n"
        "examples/test.conf:035:  |  |  priority 99\n"
        "examples/test.conf:035:  |  |  condition\n"
        "examples/test.conf:035:  |  |  |  dest 192.0\n"
        "examples/test.conf:036:  |  accept\n"
        "examples/test.conf:036:  |  |  condition\n"
        "examples/test.conf:036:  |  |  |  packet_type PING\n"
        "examples/test.conf:037:  |  accept\n");
}
