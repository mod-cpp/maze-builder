#include <algorithm>
#include <fmt/format.h>
#include <functional>
#include <limits>
#include <map>
#include <ranges>
#include <string_view>
#include <vector>
#include <random>
#include "compiletime_random.hpp"
#include "cartesian_product.hpp"


template<std::size_t width, std::size_t height>
struct half_map;

template<std::size_t width, std::size_t height>
struct map {
  constexpr map(half_map<width / 2, height> hm) {
    for (std::size_t i = 0; i < height; i++) {
      auto line = std::views::all(hm.walls[i]);
      std::ranges::copy(line, std::ranges::begin(walls[i]));
      int n = width;
      for (auto & c : line) {
        walls[i][--n] = c; // reverse doesn't work here?
      }
    }
  }
  bool walls[height][width] = { 0 };
};

template<std::size_t width, std::size_t height>
map(half_map<width, height>) -> map<width * 2, height>;

struct position {
  int x, y;
  bool operator==(const position &) const = default;
};

template<std::size_t width, std::size_t height>
struct half_map {
  constexpr half_map(std::string_view str) {
    auto view =
      std::views::filter(str,
                         [](char c) { return c == '|' || c == '.'; }) |
      std::views::transform([](char c) { return c == '|' ? 1 : 0; });
    for (std::size_t y = 0; y < height; y++) {
      std::ranges::copy(
        view | std::views::drop(y * width) | std::views::take(width),
        std::begin(walls[y]));
    }
  }

  constexpr bool is_valid(position p) const {
    return p.x>= 0 && static_cast<std::size_t>(p.x) < width && p.y>=0 && static_cast<std::size_t>(p.y) < height;
  }

  constexpr bool is_empty(position p) const {
    return is_valid(p) && !walls[p.y][p.x];
  }

  constexpr bool is_wall(position p) const {
    return is_valid(p) && walls[p.y][p.x];
  }

  constexpr bool can_fit_new_block(position p) const {
    if (!is_valid(p) || !is_valid({ p.x + 3, p.y + 3 }))
      return false;

    auto view = cor3ntin::rangesnext::product(
                  std::views::iota(p.x, p.x + 4),
                  std::views::iota(p.y, p.y + 4)) |
                std::views::transform(&std::make_from_tuple<position, std::tuple<int, int>>);
    return std::ranges::all_of(view, std::bind_front(&half_map::is_empty, this));
  }

  constexpr bool is_wall_block_filled(position p) const {
    auto view = cor3ntin::rangesnext::product(
                  std::views::iota(1, 3),
                  std::views::iota(1, 3)) |
                std::views::transform([&](const auto & tuple) {
                  return position{ p.x + std::get<0>(tuple), p.y + std::get<1>(tuple) };
                });
    return std::ranges::all_of(view, std::bind_front(&half_map::is_wall, this));
  }

  constexpr auto all() const {
    return cor3ntin::rangesnext::product(
             std::views::iota(0uz, width),
             std::views::iota(0uz, height)) |
           std::views::transform(&std::make_from_tuple<position, std::tuple<int, int>>);
  }

  constexpr void collect_valid_starting_positions() {
    free_positions.clear();
    auto view = all() | std::views::filter(std::bind_front(&half_map::can_fit_new_block, this));
    std::ranges::copy(view, std::back_inserter(free_positions));
  }

  std::vector<position> free_positions;
  std::vector<std::tuple<position, std::vector<position>>> connections;
  bool walls[height][width] = { 0 };

  rng::PCG pcg = [](int count = 30) {
    rng::PCG pcg;
    while (count > 0) {
      pcg();
      --count;
    }
    return pcg;
  }();

  auto get_random_number_runtime() {
    static std::random_device rd;
    return rd();
  }

  constexpr auto get_random() {
    if(std::is_constant_evaluated()) {
        return pcg();
    }
    else {
        return get_random_number_runtime();
    }
  }

  constexpr bool has_free_position(position pos) const {
    return std::ranges::find(free_positions, pos) != std::ranges::end(free_positions);
  }

  constexpr void add_connection(position pos, int dx, int dy) {
    if (!has_free_position(pos))
      return;
    auto connect = [&](position dest) {
      if (!has_free_position(dest))
        return;
      auto it = std::ranges::find_if(connections, [&](const auto & tuple) {
        return std::get<0>(tuple) == dest;
      });
      if (it == std::ranges::end(connections))
        it = connections.insert(it, std::tuple{ dest, std::vector<position>{} });
      std::get<1>(*it).push_back(pos);
    };
    auto [x, y] = pos;
    connect({ x + dx, y + dy });
    connect({ x + 2 * dx, y + 2 * dy });

    if (!has_free_position({ x - dx, y - dx }))
      connect({ x + dx - dy, y + dy - dx });
    if (!has_free_position({ x + dy, y + dx }))
      connect({ x + dx + dy, y + dy + dx });
    if (!has_free_position({ x + dx - dy, y + dy - dx }))
      connect({ x + 2 * dx - dy, y + 2 * dy - dx });
    if (!has_free_position({ x + dx + dy, y + dy + dx }))
      connect({ x + 2 * dx + dy, y + 2 * dy + dx });
  }

  constexpr void collect_connections() {
    connections.clear();
    auto view = all() | std::views::filter(std::bind_front(&half_map::has_free_position, this));

    auto four = std::views::iota(0, 4);
    for (const auto & pos : view) {
      if (std::ranges::any_of(four, [&](auto i) { return is_wall({ pos.x - 1, pos.y + i }); })) {
        add_connection(pos, 1, 0);
      }
      if (std::ranges::any_of(four, [&](auto i) { return is_wall({ pos.x + 4, pos.y + i }); })) {
        add_connection(pos, -1, 0);
      }
      if (std::ranges::any_of(four, [&](auto i) { return is_wall({ pos.x + i, pos.y - 1 }); })) {
        add_connection(pos, 0, 1);
      }
      if (std::ranges::any_of(four, [&](auto i) { return is_wall({ pos.x + i, pos.y + 4 }); })) {
        add_connection(pos, 0, -1);
      }
    }
  }

  constexpr void add_wall_tile(const position & p) {
    if (is_valid(p)) {
      walls[p.y][p.x] = true;
    }
  }

  constexpr void add_wall_block(const position & p) {
    add_wall_tile({ p.x + 1, p.y + 1 });
    add_wall_tile({ p.x + 2, p.y + 1 });
    add_wall_tile({ p.x + 1, p.y + 2 });
    add_wall_tile({ p.x + 2, p.y + 2 });
  }

  constexpr int expand_wall(std::vector<position> & visited, const position & p) {
    if (std::ranges::find(visited, p) == std::ranges::end(visited))
      return 0;
    visited.push_back(p);
    auto it = std::ranges::find_if(connections, [&](const auto & tuple) {
      return std::get<0>(tuple) == p;
    });
    if (it == std::ranges::end(connections))
      return 0;

    int count = 0;
    for (auto && pos : std::get<1>(*it)) {
      if (!is_wall_block_filled(pos)) {
        count++;
        add_wall_block(pos);
      }
      count += expand_wall(visited, pos);
    }
    return count;
  }

  constexpr int expand_wall(const position & p) {
    std::vector<position> visited;
    return expand_wall(visited, p);
  }

  constexpr bool add_wall() {
    collect_valid_starting_positions();
    collect_connections();
    if (free_positions.empty())
      return false;
    position p = free_positions[get_random() % free_positions.size()];

    add_wall_block(p);
    auto count = expand_wall(p);

    int max_blocks = 4;
    bool turn = false;
    int turn_blocks = max_blocks;
    if((get_random() %  100) <= 35) {
        turn_blocks = 4;
        max_blocks += turn_blocks;
    }

    std::array<position, 4> directions = {{
        {0, -1}, {0, 1}, {1, 0}, {-1, 0}
    }};
    auto orig = directions[get_random() % 4];
    auto [dx, dy] = orig;
    for(int i = 0; count < max_blocks;) {
        auto p0 = position{p.x + dx*i, p.y+dy*i};
        if((!turn && count >= turn_blocks) || !has_free_position(p0)) {
            turn = true;
            auto t1 = -dy, t2 = dx;
            dx = t1;
            dy = t2;
            i = 1;
            if(orig == position{dx, dy})
                break;
            else continue;
        }
        if(!is_wall_block_filled(p0)) {
            add_wall_block(p0);
            count += 1 + expand_wall(p0);
        }
        i++;
    }
    return true;
  }
};

template<std::size_t width, std::size_t height>
struct fmt::formatter<map<width, height>> {
  constexpr auto parse(format_parse_context & ctx) -> auto {
    return ctx.begin();
  }
  template<typename FormatContext>
  auto format(const map<width, height> & m, FormatContext & ctx)
    -> decltype(ctx.out()) {
    for (std::size_t y = 0; y < height; y++) {
      for (std::size_t x = 0; x < width; x++) {
        format_to(ctx.out(), "{}", m.walls[y][x] ? "🟨" : "🟦");
      }
      format_to(ctx.out(), "\n");
    }
    return ctx.out();
  }
};

constexpr auto create_random_map() {
  half_map<16, 31> hm(R"(
||||||||||||||||
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|.........||||||
|.........||||||
|.........||||||
|.........||||||
|.........||||||
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
|...............
||||||||||||||||)");

  while (hm.add_wall())
    ;

  return hm;
}

int main() {
  constexpr map m{ create_random_map() };
  fmt::print("{}", m);
}
