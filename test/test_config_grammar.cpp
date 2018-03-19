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

#include <catch.hpp>
#include <fakeit.hpp>
#include <pegtl.hpp>

#include "config_grammar.hpp"
#include "util.hpp"

#include "common.hpp"


TEST_CASE("Global statements.", "[config]")
{
    SECTION("Missing end of statement ';'.")
    {
        tao::pegtl::string_input<> in("invalid", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":1:7(7): expected end of statement ';' character");
    }
    SECTION("Invalid statement.")
    {
        tao::pegtl::string_input<> in("invalid;", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:8(8): unsupported statement");
    }
}


TEST_CASE("Parse global 'default_action' statement.", "[config]")
{
    SECTION("Parses the 'accept' option.")
    {
        tao::pegtl::string_input<> in("default_action accept;", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  default_action\n"
            ":001:  |  accept\n");
    }
    SECTION("Parses the 'reject' option.")
    {
        tao::pegtl::string_input<> in("default_action reject;", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  default_action\n"
            ":001:  |  reject\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in("default_action accept", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":1:21(21): expected end of statement ';' character");
    }
    SECTION("Invalid default action.")
    {
        tao::pegtl::string_input<> in("default_action invalid;", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:15(15): expected 'accept' or 'reject'");
    }
    SECTION("Missing default action value.")
    {
        tao::pegtl::string_input<> in("default_action;", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:14(14): expected 'accept' or 'reject'");
    }
}


TEST_CASE("UDP configuration block.", "[config]")
{
    SECTION("Empty UDP blocks are allowed (single line).")
    {
        tao::pegtl::string_input<> in("udp {}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  udp\n");
    }
    SECTION("Empty UDP blocks are allowed (1TBS style).")
    {
        tao::pegtl::string_input<> in("udp {\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  udp\n");
    }
    SECTION("Empty UDP blocks are allowed (Allman style).")
    {
        tao::pegtl::string_input<> in("udp\n{\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  udp\n");
    }
    SECTION("Missing opening brace (closing brace exists).")
    {
        tao::pegtl::string_input<> in("udp }", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:4(4): expected opening brace '{'");
    }
    SECTION("Missing opening brace (closing brace missing).")
    {
        tao::pegtl::string_input<> in("udp", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:3(3): expected opening brace '{'");
    }
    SECTION("Missing closing brace.")
    {
        tao::pegtl::string_input<> in("udp {", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:5(5): expected closing brace '}'");
    }
}


TEST_CASE("UDP port setting.", "[config]")
{
    SECTION("Parses port number setting.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port 14500;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  udp\n"
            ":002:  |  port 14500\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port 14500\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(21): expected end of statement ';' character");
    }
    SECTION("Invalid port number.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port a35;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:9(15): expected a valid port number");
    }
    SECTION("Missing port number.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    port;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:8(14): expected a valid port number");
    }
}


TEST_CASE("UDP IP address setting.", "[config]")
{
    SECTION("Parses IP address setting.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.0.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  udp\n"
            ":002:  |  address 127.0.0.1\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.0.1\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(28): expected end of statement ';' character");
    }
    SECTION("Invalid IP address [1].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.0.1.;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:21(27): expected end of statement ';' character");
    }
    SECTION("Invalid IP address [2].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.0.;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:20(26): expected a valid IP address");
    }
    SECTION("Invalid IP address [3].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.0;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:19(25): expected a valid IP address");
    }
    SECTION("Invalid IP address [4].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0.;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:18(24): expected a valid IP address");
    }
    SECTION("Invalid IP address [5].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.0;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:17(23): expected a valid IP address");
    }
    SECTION("Invalid IP address [6].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:16(22): expected a valid IP address");
    }
    SECTION("Invalid IP address [7].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:15(21): expected a valid IP address");
    }
    SECTION("Invalid IP address [8].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127:0:0:1;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:15(21): expected a valid IP address");
    }
    SECTION("Invalid IP address [9].")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address 127.a.0.1;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:16(22): expected a valid IP address");
    }
    SECTION("Missing IP address.")
    {
        tao::pegtl::string_input<> in(
            "udp {\n"
            "    address;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":2:11(17): expected a valid IP address");
    }
}


TEST_CASE("Serial port configuration block.", "[config]")
{
    SECTION("Empty serial port blocks are allowed (single line).")
    {
        tao::pegtl::string_input<> in("serial {}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  serial\n");
    }
    SECTION("Empty serial port blocks are allowed (1TBS style).")
    {
        tao::pegtl::string_input<> in("serial {\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  serial\n");
    }
    SECTION("Empty serial port blocks are allowed (Allman style).")
    {
        tao::pegtl::string_input<> in("serial\n{\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  serial\n");
    }
    SECTION("Missing opening brace (closing brace exists).")
    {
        tao::pegtl::string_input<> in("serial }", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:7(7): expected opening brace '{'");
    }
    SECTION("Missing opening brace (closing brace missing).")
    {
        tao::pegtl::string_input<> in("serial", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:6(6): expected opening brace '{'");
    }
    SECTION("Missing closing brace.")
    {
        tao::pegtl::string_input<> in("serial {", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:8(8): expected closing brace '}'");
    }
}


TEST_CASE("Serial port device setting.", "[config]")
{
    SECTION("Parses device string setting [1].")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device /dev/tty_USB0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  serial\n"
            ":002:  |  device /dev/tty_USB0\n");
    }
    SECTION("Parses device string setting [2].")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device COM1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  serial\n"
            ":002:  |  device COM1\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device ttyUSB0\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(28): expected end of statement ';' character");
    }
    SECTION("Invalid device string.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device +ab;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:11(20): expected a valid serial port device name");
    }
    SECTION("Missing device string.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    device;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:10(19): expected a valid serial port device name");
    }
}


TEST_CASE("Serial port flow control setting.", "[config]")
{
    SECTION("Parses flow control setting (yes).")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    flow_control yes;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  serial\n"
            ":002:  |  flow_control yes\n");
    }
    SECTION("Parses flow control setting (no).")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    flow_control no;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  serial\n"
            ":002:  |  flow_control no\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    flow_control yes\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(30): expected end of statement ';' character");
    }
    SECTION("Invalid flow control setting.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    flow_control maybe;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:17(26): expected 'yes' or 'no'");
    }
    SECTION("Missing flow control setting.")
    {
        tao::pegtl::string_input<> in(
            "serial {\n"
            "    flow_control;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:16(25): expected 'yes' or 'no'");
    }
}


TEST_CASE("Chain block.", "[config]")
{
    SECTION("Empty chain blocks are allowed (single line).")
    {
        tao::pegtl::string_input<> in("chain some_name10 {}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  chain some_name10\n");
    }
    SECTION("Empty chain blocks are allowed (1TBS style).")
    {
        tao::pegtl::string_input<> in("chain some_name10 {\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  chain some_name10\n");
    }
    SECTION("Empty chain blocks are allowed (Allman style).")
    {
        tao::pegtl::string_input<> in("chain some_name10\n{\n}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(str(*root) == ":001:  chain some_name10\n");
    }
    SECTION("Chains must have a name.")
    {
        tao::pegtl::string_input<> in("chain {}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:6(6): expected a valid chain name");
    }
    SECTION("Chain names must be valid.")
    {
        tao::pegtl::string_input<> in("chain 1nv@l1d {}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:6(6): expected a valid chain name");
    }
    SECTION("Missing opening brace (closing brace exists).")
    {
        tao::pegtl::string_input<> in("chain some_name10 }", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:18(18): expected opening brace '{'");
    }
    SECTION("Missing opening brace (closing brace missing).")
    {
        tao::pegtl::string_input<> in("chain some_name10", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:17(17): expected opening brace '{'");
    }
    SECTION("Missing closing brace.")
    {
        tao::pegtl::string_input<> in("chain some_name10 {", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in), ":1:19(19): expected closing brace '}'");
    }
}


TEST_CASE("Invalid rule.", "[config]")
{
    SECTION("Parses 'reject' rule.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    invalid;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(29): expected a valid rule");
    }
}


TEST_CASE("Accept rule.", "[config]")
{
    SECTION("Parses 'accept' rule.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:10(26): expected end of statement ';' character");
    }
}


TEST_CASE("Reject rule.", "[config]")
{
    SECTION("Parses 'reject' rule.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:10(26): expected end of statement ';' character");
    }
}


TEST_CASE("Call rule.", "[config]")
{
    SECTION("Parses 'call' rule.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(37): expected end of statement ';' character");
    }
    SECTION("Invalid chain name.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call 1nv@l1d;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:9(25): expected a valid chain name");
    }
    SECTION("Missing chain name.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:8(24): expected a valid chain name");
    }
}


TEST_CASE("GoTo rule.", "[config]")
{
    SECTION("Parses 'goto' rule.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n");
    }
    SECTION("Missing end of statement.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":3:0(37): expected end of statement ';' character");
    }
    SECTION("Invalid chain name.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto 1nv@l1d;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:9(25): expected a valid chain name");
    }
    SECTION("Missing chain name.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:8(24): expected a valid chain name");
    }
}


TEST_CASE("Condition.", "[config]")
{
    SECTION("Conditions must have at least one restriction.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:13(29): condition is empty or invalid");
    }
}


TEST_CASE("Packet type condition.", "[config]")
{
    SECTION("Parses packet type condition.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("Invalid packet type [1].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if ping;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:14(30): condition is empty or invalid");
    }
    SECTION("Invalid packet type [2].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if 4;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:14(30): condition is empty or invalid");
    }
}


TEST_CASE("Source condition.", "[config]")
{
    SECTION("Parses source condition [1].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 192.0\n");
    }
    SECTION("Parses source condition [2].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.0/8;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 192.0/8\n");
    }
    SECTION("Parses source condition [3].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.0\\4;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 192.0\\4\n");
    }
    SECTION("Parses source condition [4].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192.0:255.255;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 192.0:255.255\n");
    }
    SECTION("Invalid source condition [1].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:22(38): expected a valid MAVLink subnet");
    }
    SECTION("Invalid source condition [2].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192/8;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:22(38): expected a valid MAVLink subnet");
    }
    SECTION("Invalid source condition [3].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192\\8;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:22(38): expected a valid MAVLink subnet");
    }
    SECTION("Invalid source condition [4].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 192:255.255;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:22(38): expected a valid MAVLink subnet");
    }
}


TEST_CASE("Destination condition.", "[config]")
{
    SECTION("Parses destination condition [1].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("Parses destination condition [2].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192.0/8;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0/8\n");
    }
    SECTION("Parses destination condition [3].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192.0\\4;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\\4\n");
    }
    SECTION("Parses destination condition [4].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192.0:255.255;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0:255.255\n");
    }
    SECTION("Invalid destination condition [1].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:20(36): expected a valid MAVLink subnet");
    }
    SECTION("Invalid destination condition [2].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192/8;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:20(36): expected a valid MAVLink subnet");
    }
    SECTION("Invalid destination condition [3].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192\\8;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:20(36): expected a valid MAVLink subnet");
    }
    SECTION("Invalid destination condition [4].")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192:255.255;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:20(36): expected a valid MAVLink subnet");
    }
}


TEST_CASE("Priority.", "[config]")
{
    SECTION("Parses priority (no sign).")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n");
    }
    SECTION("Parses priority (positive).")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority +99;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority +99\n");
    }
    SECTION("Parses priority (negative).")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority -99;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority -99\n");
    }
    SECTION("'with priority' must be followed by a valid priority.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority *99;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:25(41): expected priority level");
    }
    SECTION("'with priority' must be followed by a priority.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:24(40): expected priority level");
    }
    SECTION("'with' must be followed by 'priority'.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with;\n"
            "}", "");
        REQUIRE_THROWS_AS(config::parse(in), tao::pegtl::parse_error);
        REQUIRE_THROWS_WITH(
            config::parse(in),
            ":2:15(31): 'with' keyword must be followed by the "
            "'priority' keyword");
    }
}


TEST_CASE("Rule combinations with 'accept'.", "[config]")
{
    SECTION("accept")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n");
    }
    SECTION("accept if PING.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("accept if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("accept if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("accept if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept with priority 99")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept with priority 99 if PING")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("accept with priority 99 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("accept with priority 99 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept with priority 99 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept with priority 99 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("accept with priority 99 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("accept with priority 99 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    accept with priority 99 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  accept\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
}


TEST_CASE("Rule combinations with 'reject'.", "[config]")
{
    SECTION("reject")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n");
    }
    SECTION("reject if PING.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("reject if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("reject if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("reject if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject with priority 99")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject with priority 99 if PING")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("reject with priority 99 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("reject with priority 99 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject with priority 99 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject with priority 99 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("reject with priority 99 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("reject with priority 99 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    reject with priority 99 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  reject\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
}


TEST_CASE("Rule combinations with 'call'.", "[config]")
{
    SECTION("call some_name10")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n");
    }
    SECTION("call some_name10 if PING.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("call some_name10 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("call some_name10 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("call some_name10 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 with priority 99")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 with priority 99 if PING")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("call some_name10 with priority 99 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("call some_name10 with priority 99 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 with priority 99 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 "
            "if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 with priority 99 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("call some_name10 with priority 99 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("call some_name10 with priority 99 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    call some_name10 with priority 99 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  call some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
}


TEST_CASE("Rule combinations with 'goto'.", "[config]")
{
    SECTION("goto some_name10")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n");
    }
    SECTION("goto some_name10 if PING.")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("goto some_name10 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("goto some_name10 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("goto some_name10 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 with priority 99")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 with priority 99 if PING")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if PING;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n");
    }
    SECTION("goto some_name10 with priority 99 if PING from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if PING from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("goto some_name10 with priority 99 if PING to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if PING to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 with priority 99 if PING from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 "
            "if PING from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  packet_type PING\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 with priority 99 if from 127.1")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if from 127.1;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n");
    }
    SECTION("goto some_name10 with priority 99 if to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  dest 192.0\n");
    }
    SECTION("goto some_name10 with priority 99 if from 127.1 to 192.0")
    {
        tao::pegtl::string_input<> in(
            "chain default {\n"
            "    goto some_name10 with priority 99 if from 127.1 to 192.0;\n"
            "}", "");
        auto root = config::parse(in);
        REQUIRE(root != nullptr);
        REQUIRE(
            str(*root) ==
            ":001:  chain default\n"
            ":002:  |  goto some_name10\n"
            ":002:  |  |  priority 99\n"
            ":002:  |  |  condition\n"
            ":002:  |  |  |  source 127.1\n"
            ":002:  |  |  |  dest 192.0\n");
    }
}
