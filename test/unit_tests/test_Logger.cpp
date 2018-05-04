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
#include "utility.hpp"

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


TEST_CASE("The Logger can log to stdout with the static 'log' method if the "
          " logger's level is the same or higher than the loglevel of the "
          "message.", "[Logger]")
{
    MockCOut mock_cout;
    SECTION("No message can be logged if the logger's level is 0.")
    {
        Logger::level(0);
        Logger::log("a log message");
        REQUIRE(mock_cout.buffer().empty());
        mock_cout.reset();
        Logger::log("another log message");
        REQUIRE(mock_cout.buffer().empty());
    }
    SECTION("The default level of the 'log' method is 1.")
    {
        Logger::level(0);
        Logger::log("not logged");
        REQUIRE(mock_cout.buffer().empty());
        Logger::level(1);
        Logger::log("a log message");
        REQUIRE(mock_cout.buffer().substr(21) == "a log message\n");
        mock_cout.reset();
        Logger::log("another log message");
        REQUIRE(mock_cout.buffer().substr(21) == "another log message\n");
    }
    SECTION("A level of 0 will be changed to level 1.")
    {
        Logger::level(0);
        Logger::log(0, "not logged");
        REQUIRE(mock_cout.buffer().empty());
        Logger::level(1);
        Logger::log(0, "a log message");
        REQUIRE(mock_cout.buffer().substr(21) == "a log message\n");
        mock_cout.reset();
        Logger::log(0, "another log message");
        REQUIRE(mock_cout.buffer().substr(21) == "another log message\n");
    }
    SECTION("The level can be set from 1 to 65535.")
    {
        Logger::level(0);
        Logger::log(65535, "not logged");
        REQUIRE(mock_cout.buffer().empty());
        Logger::level(65535);
        Logger::log(65535, "a log message");
        REQUIRE(mock_cout.buffer().substr(21) == "a log message\n");
        mock_cout.reset();
        Logger::log(65535, "another log message");
        REQUIRE(mock_cout.buffer().substr(21) == "another log message\n");
    }
    Logger::level(0);
}


TEST_CASE("All logged messages have a timestamp (this can sometimes fail if "
          "the clock ticks over a second during the test).", "[Logger]")
{
    Logger::level(1);
    MockCOut mock_cout;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    Logger::log("timestamped log message");
    REQUIRE(
        mock_cout.buffer() ==
        str(std::put_time(&tm, "%Y-%m-%d %H:%M:%S")) +
        "  timestamped log message\n");
    Logger::level(0);
}
