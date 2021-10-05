#include <onsem/common/utility/random.hpp>
#include <sstream>
#include <random>
#include <array>
#include <algorithm>
#include <functional>

namespace onsem
{
std::atomic<int> Random::_currentNumber(0);


constexpr std::size_t SEED_LENGTH = 8;

std::array<uint_fast32_t, SEED_LENGTH> generateSeedData() {
  std::array<uint_fast32_t, SEED_LENGTH> random_data;
  std::random_device random_source;
  std::generate(random_data.begin(), random_data.end(), std::ref(random_source));
  return random_data;
}

std::mt19937 createEngine() {
  auto random_data = generateSeedData();
  std::seed_seq seed_seq(random_data.begin(), random_data.end());
  return std::mt19937{ seed_seq };
}

static std::mt19937                    gen(createEngine());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string Random::generateUuid() {
  std::stringstream ss;
  int i;
  ss << std::hex;
  for (i = 0; i < 8; i++) {
    ss << dis(gen);
  }
  ss << "-";
  for (i = 0; i < 4; i++) {
    ss << dis(gen);
  }
  ss << "-4";
  for (i = 0; i < 3; i++) {
    ss << dis(gen);
  }
  ss << "-";
  ss << dis2(gen);
  for (i = 0; i < 3; i++) {
    ss << dis(gen);
  }
  ss << "-";
  for (i = 0; i < 12; i++) {
    ss << dis(gen);
  }
  return ss.str();
}


std::string Random::generateUuidWithoutHyphen() {
  std::stringstream ss;
  int i;
  ss << std::hex;
  for (i = 0; i < 30; i++)
    ss << dis(gen);
  return "u" + ss.str();
}


} // End of namespace onsem
