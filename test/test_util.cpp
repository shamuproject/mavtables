#include "catch.hpp"
#include "util.hpp"

TEST_CASE( "capital_case capitalizes first character of a string." ){
    REQUIRE( capital_case("the quick brown fox.") == "The quick brown fox.");
}
