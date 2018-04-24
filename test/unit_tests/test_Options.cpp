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


#include <sstream>
#include <stdexcept>

#include "catch.hpp"
#include "fakeit.hpp"

#include "config.hpp"
#include "Filesystem.hpp"
#include "Options.hpp"

#include "common.hpp"


TEST_CASE("'find_config' returns the path to the highest priority "
          "configuration file.", "[Options]")
{
    std::vector<Filesystem::path> paths;
    fakeit::Mock<Filesystem> fs_mock;
    SECTION("First found is given by MAVTABLES_CONFIG_PATH environment "
            "variable.")
    {
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path("mtbls.conf");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        auto config_file = find_config(fs_mock.get());
        REQUIRE(config_file.has_value());
        REQUIRE(config_file.value() == "mtbls.conf");
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(1);
        REQUIRE(paths.size() == 1);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
    }
    SECTION("First found is .mavtablesrc in current directory.")
    {
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path(".mavtablesrc");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        auto config_file = find_config(fs_mock.get());
        REQUIRE(config_file.has_value());
        REQUIRE(config_file.value() == ".mavtablesrc");
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(2);
        REQUIRE(paths.size() == 2);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
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
        auto config_file = find_config(fs_mock.get());
        REQUIRE(config_file.has_value());
        REQUIRE(config_file.value() == home_path);
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(3);
        REQUIRE(paths.size() == 3);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
    }
    SECTION("First found is PREFIX/etc/mavtables.conf.")
    {
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == (PREFIX "/etc/mavtables.conf");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        auto config_file = find_config(fs_mock.get());
        REQUIRE(config_file.has_value());
        REQUIRE(config_file.value() == (PREFIX "/etc/mavtables.conf"));
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(4);
        REQUIRE(paths.size() == 4);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
        REQUIRE(paths[3] == Filesystem::path(PREFIX "/etc/mavtables.conf"));
    }
    SECTION("Failed to find any configuration file.")
    {
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return false;
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        auto config_file = find_config(fs_mock.get());
        REQUIRE_FALSE(config_file.has_value());
        unsetenv("MAVTABLES_CONFIG_PATH");
        fakeit::Verify(Method(fs_mock, exists)).Exactly(4);
        REQUIRE(paths.size() == 4);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
        REQUIRE(paths[3] == Filesystem::path(PREFIX "/etc/mavtables.conf"));
    }
}


TEST_CASE("Options's class prints the help message.", "[Options]")
{
    std::string help_message =
        "usage: <program name here>:\n"
        "  -h [ --help ]         print this message\n"
        "  --config arg          specify configuration file\n"
        "  --ast                 print AST of configuration file (do not run)\n"
        "  --version             print version and license information\n"
        "  --loglevel arg        level of logging, between 0 and 3\n\n";
    SECTION("When given the '-h' flag.")
    {
        int argc = 2;
        const char *argv[2] = {"<program name here>", "-h"};
        std::stringstream ss;
        Options options(argc, argv, ss);
        REQUIRE(ss.str() == help_message);
        REQUIRE_FALSE(options);
    }
    SECTION("When given the '--help' flag.")
    {
        int argc = 2;
        const char *argv[2] = {"<program name here>", "--help"};
        std::stringstream ss;
        Options options(argc, argv, ss);
        REQUIRE(ss.str() == help_message);
        REQUIRE_FALSE(options);
    }
}


TEST_CASE("Options's class prints version information when given the "
          "'--version' flag.", "[Options]")
{
    int argc = 2;
    const char *argv[2] = {"<program name here>", "--version"};
    std::stringstream ss;
    Options options(argc, argv, ss);
    REQUIRE(
        ss.str() ==
        "mavtables (SHAMU Project) v" + std::to_string(VERSION_MAJOR) +
        "." + std::to_string(VERSION_MINOR) +
        "." + std::to_string(VERSION_PATCH) +
        "\nCopyright (C) 2018  Michael R. Shannon\n"
        "\n"
        "License: GPL v2.0 or any later version.\n"
        "This is free software; see the source for copying conditions.  "
        "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A "
        "PARTICULAR PURPOSE.\n");
    REQUIRE_FALSE(options);
}



TEST_CASE("Options's class will use the given configuration file "
          "(--config flag).", "[Options]")
{
    std::vector<Filesystem::path> paths;
    fakeit::Mock<Filesystem> fs_mock;
    SECTION("File found.")
    {
        // Setup mocks.
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path("examples/test.conf");
        });
        // Construct Options object.
        int argc = 3;
        const char *argv[3] = {"mavtables", "--config", "examples/test.conf"};
        std::stringstream ss;
        Options options(argc, argv, ss, fs_mock.get());
        // Verify Options object.
        REQUIRE(options.config_file() == "examples/test.conf");
        REQUIRE(options);
        REQUIRE_FALSE(options.ast());
        REQUIRE(options.run());
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(1);
        REQUIRE(paths.size() == 1);
        REQUIRE(paths[0] == Filesystem::path("examples/test.conf"));
    }
    SECTION("File not found (throws error).")
    {
        // Setup mocks.
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return false;
        });
        // Construct Options object.
        int argc = 3;
        const char *argv[3] = {
            "mavtables", "--config", "non_existant_file.conf"};
        std::stringstream ss;
        
        // Verify Options object.
        REQUIRE_THROWS_AS(
            Options(argc, argv, ss, fs_mock.get()), std::runtime_error);
        REQUIRE_THROWS_WITH(
            Options(argc, argv, ss, fs_mock.get()),
            "mavtables could not locate a configuration file");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(2);
        REQUIRE(paths.size() == 2);
        REQUIRE(paths[0] == Filesystem::path("non_existant_file.conf"));
        REQUIRE(paths[1] == Filesystem::path("non_existant_file.conf"));
    }
}


TEST_CASE("Options's class finds the configuration file.", "[Options]")
{
    std::vector<Filesystem::path> paths;
    fakeit::Mock<Filesystem> fs_mock;
    SECTION("First found is given by MAVTABLES_CONFIG_PATH environment "
            "variable.")
    {
        // Setup mocks.
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path("mtbls.conf");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        // Construct Options object.
        int argc = 1;
        const char *argv[2] = {"mavtables"};
        std::stringstream ss;
        Options options(argc, argv, ss, fs_mock.get());
        unsetenv("MAVTABLES_CONFIG_PATH");
        // Verify Options object.
        REQUIRE(options.config_file() == "mtbls.conf");
        REQUIRE(options);
        REQUIRE_FALSE(options.ast());
        REQUIRE(options.run());
        // Verify printing.
        REQUIRE(ss.str() == "");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(1);
        REQUIRE(paths.size() == 1);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
    }
    SECTION("First found is .mavtablesrc in current directory.")
    {
        // Setup mocks.
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == Filesystem::path(".mavtablesrc");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        // Construct Options object.
        int argc = 1;
        const char *argv[2] = {"mavtables"};
        std::stringstream ss;
        Options options(argc, argv, ss, fs_mock.get());
        unsetenv("MAVTABLES_CONFIG_PATH");
        // Verify Options object.
        REQUIRE(options.config_file() == ".mavtablesrc");
        REQUIRE(options);
        REQUIRE_FALSE(options.ast());
        REQUIRE(options.run());
        // Verify printing.
        REQUIRE(ss.str() == "");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(2);
        REQUIRE(paths.size() == 2);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
    }
    SECTION("First found is .mavtablesrc in HOME directory.")
    {
        // Setup mocks.
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == home_path;
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        // Construct Options object.
        int argc = 1;
        const char *argv[2] = {"mavtables"};
        std::stringstream ss;
        Options options(argc, argv, ss, fs_mock.get());
        unsetenv("MAVTABLES_CONFIG_PATH");
        // Verify Options object.
        REQUIRE(options.config_file() == home_path);
        REQUIRE(options);
        REQUIRE_FALSE(options.ast());
        REQUIRE(options.run());
        // Verify printing.
        REQUIRE(ss.str() == "");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(3);
        REQUIRE(paths.size() == 3);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
    }
    SECTION("First found is PREFIX/etc/mavtables.conf.")
    {
        // Setup mocks.
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return p == (PREFIX "/etc/mavtables.conf");
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        // Construct Options object.
        int argc = 1;
        const char *argv[2] = {"mavtables"};
        std::stringstream ss;
        Options options(argc, argv, ss, fs_mock.get());
        unsetenv("MAVTABLES_CONFIG_PATH");
        // Verify Options object.
        REQUIRE(options.config_file() == (PREFIX "/etc/mavtables.conf"));
        REQUIRE(options);
        REQUIRE_FALSE(options.ast());
        REQUIRE(options.run());
        // Verify printing.
        REQUIRE(ss.str() == "");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(4);
        REQUIRE(paths.size() == 4);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
        REQUIRE(paths[3] == Filesystem::path(PREFIX "/etc/mavtables.conf"));
    }
    SECTION("Failed to find any configuration file.")
    {
        // Setup mocks.
        Filesystem::path home_path(std::getenv("HOME"));
        home_path /= Filesystem::path(".mavtablesrc");
        fakeit::When(Method(fs_mock, exists)).AlwaysDo([&](auto p)
        {
            paths.push_back(p);
            return false;
        });
        setenv("MAVTABLES_CONFIG_PATH", "mtbls.conf", true);
        // Construct Options object.
        int argc = 1;
        const char *argv[2] = {"mavtables"};
        std::stringstream ss;
        REQUIRE_THROWS_AS(
            Options(argc, argv, ss, fs_mock.get()),
            std::runtime_error);
        REQUIRE_THROWS_WITH(
            Options(argc, argv, ss, fs_mock.get()),
            "mavtables could not locate a configuration file");
        unsetenv("MAVTABLES_CONFIG_PATH");
        // Verify exists calls.
        fakeit::Verify(Method(fs_mock, exists)).Exactly(8);
        REQUIRE(paths.size() == 8);
        REQUIRE(paths[0] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[1] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[2] == Filesystem::path(home_path));
        REQUIRE(paths[3] == Filesystem::path(PREFIX "/etc/mavtables.conf"));
        REQUIRE(paths[4] == Filesystem::path("mtbls.conf"));
        REQUIRE(paths[5] == Filesystem::path(".mavtablesrc"));
        REQUIRE(paths[6] == Filesystem::path(home_path));
        REQUIRE(paths[7] == Filesystem::path(PREFIX "/etc/mavtables.conf"));
    }
}


TEST_CASE("Options's class sets run to false and ast to true when the --ast "
          "flag is given.", "[Options]")
{
    // Setup mocks.
    fakeit::Mock<Filesystem> fs_mock;
    fakeit::When(Method(fs_mock, exists)).AlwaysReturn(true);
    // Construct Options object.
    int argc = 4;
    const char *argv[4] = {
        "mavtables", "--ast", "--config", "test/mavtables.conf"};
    std::stringstream ss;
    Options options(argc, argv, ss);
    // Verify Options object.
    REQUIRE(options.config_file() == "test/mavtables.conf");
    REQUIRE(options);
    REQUIRE(options.ast());
    REQUIRE_FALSE(options.run());
    // Verify printing.
    REQUIRE(ss.str() == "");
}
