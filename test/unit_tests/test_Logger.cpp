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


#include <ctime>
#include <iomanip>
#include <sstream>

#include <catch.hpp>

#include "Logger.hpp"
#include "util.hpp"

#include "common.hpp"


TEST_CASE("The Logger level can be set and retrieved with the static 'level' "
          "method", "[Logger]")
{
    REQUIRE(Logger::level() == 0);

    for (unsigned int i = 0; i <= 10; i++)
    {
        Logger::level(i);
        REQUIRE(Logger::level() == i);
    }

    Logger::level(0);
}


TEST_CASE("The Logger can log to stdout with the static 'log' method.",
          "[Logger]")
{
    MockCOut mock_cout;
    Logger::log("a log message");
    REQUIRE(mock_cout.buffer().substr(21) == "a log message\n");
    mock_cout.reset();
    Logger::log("another log message");
    REQUIRE(mock_cout.buffer().substr(21) == "another log message\n");
}


TEST_CASE("All logged messages have a timestamp (this can sometimes fail if "
          "the clock ticks over a second during the test).", "[Logger]")
{
    MockCOut mock_cout;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    Logger::log("timestamped log message");
    REQUIRE(
        mock_cout.buffer() ==
        str(std::put_time(&tm, "%Y-%m-%d %H:%M:%S")) +
        "  timestamped log message\n");
}
