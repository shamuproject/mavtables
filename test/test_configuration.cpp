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

#include "stdlib.h"
#include "catch.hpp"
#include "fakeit.hpp"

#include "Filesystem.hpp"
#include "configuration.hpp"

#include "common.hpp"


TEST_CASE("'find_config' returns the path to the highest priority "
          "configuration file.", "[configuration]")
{
    std::vector<Filesystem::path> paths;
    fakeit::Mock<Filesystem> fs_mock;
    SECTION("First found is given file.")
    {
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path("/tmp/mavtables.conf");
        });
        REQUIRE(
            find_config("/tmp/mavtables.conf", mock_unique(fs_mock)) ==
            "/tmp/mavtables.conf");
        fakeit::Verify(Method(fs_mock, exists)).Once();
        REQUIRE(paths.size() == 1);
        REQUIRE(paths[0] == Filesystem::path("/tmp/mavtables.conf"));
    }
    SECTION("First found is given by MAVTABLES_CONFIG_PATH environment "
            "variable.")
    {
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path("mtbls.conf");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        REQUIRE(
            find_config("/tmp/mavtables.conf", mock_unique(fs_mock)) ==
            "mtbls.conf");
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(2);
        REQUIRE(paths.size() == 2);
        REQUIRE(paths[0] == Filesystem::path("/tmp/mavtables.conf"));
        REQUIRE(paths[1] == Filesystem::path("mtbls.conf"));
    }
    SECTION("First found is .mavtablesrc in current directory.")
    {
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path(".mavtablesrc");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        REQUIRE(
            find_config("/tmp/mavtables.conf", mock_unique(fs_mock)) ==
            ".mavtablesrc");
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(3);
        REQUIRE(paths.size() == 3);
        REQUIRE(paths[0] == Filesystem::path("/tmp/mavtables.conf"));
        REQUIRE(paths[1] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[2] == Filesystem::path(".mavtablesrc"));
    }
    SECTION("First found is .mavtablesrc in HOME directory.")
    {
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == home_path;
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        REQUIRE(
            find_config("/tmp/mavtables.conf", mock_unique(fs_mock)) ==
            home_path);
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(4);
        REQUIRE(paths.size() == 4);
        REQUIRE(paths[0] == Filesystem::path("/tmp/mavtables.conf"));
        REQUIRE(paths[1] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[2] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[3] == Filesystem::path(home_path));
    }
    SECTION("First found is /etc/mavtables.conf.")
    {
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == "/etc/mavtables.conf";
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        REQUIRE(
            find_config("/tmp/mavtables.conf", mock_unique(fs_mock)) ==
            "/etc/mavtables.conf");
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(5);
        REQUIRE(paths.size() == 5);
        REQUIRE(paths[0] == Filesystem::path("/tmp/mavtables.conf"));
        REQUIRE(paths[1] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[2] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[3] == Filesystem::path(home_path));
        REQUIRE(paths[4] == Filesystem::path("/etc/mavtables.conf"));
    }
}
