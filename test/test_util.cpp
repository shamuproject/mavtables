// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#include <catch.hpp>
#include <fakeit.hpp>
#include "util.hpp"

using namespace fakeit;


TEST_CASE("capital_case capitalizes first character of a string.")
{
    REQUIRE(capital_case("the quick brown fox.") == "The quick brown fox.");
}


TEST_CASE("fakeit example")
{
    // Sample interface.
    struct SomeInterface
    {
        virtual int is_positive(int) = 0;
        virtual int capital_case(std::string) = 0;
    };
    Mock<SomeInterface> mock;
    When(Method(mock, is_positive).Using(Gt(0))).AlwaysReturn(true);
    When(Method(mock, is_positive).Using(Le(0))).AlwaysReturn(false);
    When(Method(mock, capital_case)).Return(0, 1, 2);
    // Get rerference to the interface and test.
    SomeInterface &i = mock.get();
    REQUIRE(i.is_positive(-1) == false);
    REQUIRE(i.is_positive(0) == false);
    REQUIRE(i.is_positive(1) == true);
    i.capital_case("hello world");
    // Verify methods were called.
    Verify(Method(mock, is_positive)).Exactly(3);
    Verify(Method(mock, capital_case));
}
