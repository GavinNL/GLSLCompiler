#include <catch2/catch.hpp>

int test(int d)
{
   return d*2;
}

SCENARIO("test")
{
    REQUIRE( test(2) == 4);
}
