#include "map.hpp"

int main() {
  constexpr map m{ create_random_map() };
  fmt::print("{}", m);
}
