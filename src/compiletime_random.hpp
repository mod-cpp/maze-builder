// From Jason Turner
// https://www.youtube.com/watch?v=rpn_5Mrrxf8
namespace rng {

constexpr auto seed() {
  std::uint64_t shifted = 0;

  for (const auto c : __TIME__) {
    shifted <<= 8;
    shifted |= static_cast<long unsigned>(c);
  }
  return shifted;
}

struct PCG {
  struct pcg32_random_t {
    std::uint64_t state = 0;
    std::uint64_t inc = seed();
  };
  pcg32_random_t rng;
  typedef std::uint32_t result_type;

  constexpr result_type operator()() { return pcg32_random_r(); }

private:
  constexpr std::uint32_t pcg32_random_r() {
    std::uint64_t oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6364136223846793005ULL + (rng.inc | 1);
    // Calculate output function (XSH RR), uses old state for max ILP
    std::uint32_t xorshifted = static_cast<std::uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27lu);
    std::uint32_t rot = static_cast<std::uint32_t>(oldstate >> 59lu);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  }
};

} // namespace rng
