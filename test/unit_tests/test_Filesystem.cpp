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

#include "Filesystem.hpp"


TEST_CASE("Filesystem's 'exist' method determines the existence of a file.",
          "[Filesystem]")
{
    // Note: both compile time and dynamic construction used for complete
    //       coverage.
    SECTION("Returns 'true' when the file exists.")
    {
        Filesystem filesystem;
        REQUIRE(filesystem.exists("test/mavtables.conf"));
    }
    SECTION("Returns 'false' when the file does not exist.")
    {
        auto filesystem = std::make_unique<Filesystem>();
        REQUIRE_FALSE(
            filesystem->exists("file_that_does_not_exist.conf"));
    }
}
