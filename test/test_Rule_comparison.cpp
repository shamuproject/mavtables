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


#include <catch.hpp>
#include <fakeit.hpp>


#include "Accept.hpp"
#include "Call.hpp"
#include "Rule.hpp"
#include "Reject.hpp"
#include "GoTo.hpp"
#include "Chain.hpp"

#include "common.hpp"


TEST_CASE("Rule's are polymorphically comparable.", "[Rule]")
{
    fakeit::Mock<Chain> mock;
    std::shared_ptr<Chain> chain = mock_shared(mock.get());
    SECTION("with ==")
    {
        REQUIRE_FALSE(Accept() == Reject());
        REQUIRE_FALSE(Accept() == Call(chain));
        REQUIRE_FALSE(Accept() == GoTo(chain));
        REQUIRE_FALSE(Reject() == Accept());
        REQUIRE_FALSE(Reject() == Call(chain));
        REQUIRE_FALSE(Reject() == GoTo(chain));
        REQUIRE_FALSE(Call(chain) == Accept());
        REQUIRE_FALSE(Call(chain) == Reject());
        REQUIRE_FALSE(Call(chain) == GoTo(chain));
        REQUIRE_FALSE(GoTo(chain) == Accept());
        REQUIRE_FALSE(GoTo(chain) == Reject());
        REQUIRE_FALSE(GoTo(chain) == Call(chain));
    }
    SECTION("with !=")
    {
        REQUIRE(Accept() != Reject());
        REQUIRE(Accept() != Call(chain));
        REQUIRE(Accept() != GoTo(chain));
        REQUIRE(Reject() != Accept());
        REQUIRE(Reject() != Call(chain));
        REQUIRE(Reject() != GoTo(chain));
        REQUIRE(Call(chain) != Accept());
        REQUIRE(Call(chain) != Reject());
        REQUIRE(Call(chain) != GoTo(chain));
        REQUIRE(GoTo(chain) != Accept());
        REQUIRE(GoTo(chain) != Reject());
        REQUIRE(GoTo(chain) != Call(chain));
    }
}
