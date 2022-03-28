#include "map.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Make map", "[maze_builder]") {
  constexpr map m{ create_random_map() };
  fmt::print("{}", m);
  REQUIRE(true == true);
}
