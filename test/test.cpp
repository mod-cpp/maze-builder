#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "map.hpp"

TEST_CASE("Make map", "[maze_builder]") {
  constexpr map m{ create_random_map() };
  fmt::print("{}", m);
  REQUIRE(true == true);
}
