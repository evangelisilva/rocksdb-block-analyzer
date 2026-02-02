#include "key_generator.h"
#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

std::string GenerateRandomValue(size_t length) {
  static std::mt19937 generator(std::random_device{}());
  static std::uniform_int_distribution<int> distribution(0, 255);
  std::string str(length, 0);
  for (size_t i = 0; i < length; ++i) {
    str[i] = static_cast<char>(distribution(generator));
  }
  return str;
}

std::string EncodeKeyHybrid(int64_t value, size_t length) {
  if (length < 8)
    throw std::invalid_argument("length must be >= 8");
  std::string bytes(length, 0);
  uint64_t normalized = static_cast<uint64_t>(value) ^ 0x8000000000000000ULL;
  for (int i = 7; i >= 0; i--) {
    bytes[i] = static_cast<char>(normalized & 0xFF);
    normalized >>= 8;
  }
  std::mt19937 generator(static_cast<unsigned long>(value));
  std::uniform_int_distribution<int> distribution(0, 255);
  for (size_t i = 8; i < length; ++i) {
    bytes[i] = static_cast<char>(distribution(generator));
  }
  return bytes;
}

std::string EncodeKeyRandom(int64_t value, size_t length) {
  std::mt19937 generator(static_cast<unsigned long>(value));
  std::uniform_int_distribution<int> distribution(0, 255);
  std::string bytes(length, 0);
  for (size_t i = 0; i < length; ++i) {
    bytes[i] = static_cast<char>(distribution(generator));
  }
  return bytes;
}

std::string EncodeKeySequential(int64_t value, size_t length) {
  std::stringstream ss;
  ss << std::setw(length) << std::setfill('0') << value;
  std::string s = ss.str();
  if (s.length() > length) {
    return s.substr(s.length() - length);
  }
  return s;
}
