#ifndef KEY_GENERATOR_H
#define KEY_GENERATOR_H

#include <cstdint>
#include <string>

// Generates a random value string of specified length
std::string GenerateRandomValue(size_t length);

// 1. Hybrid: Order-preserving prefix + High-entropy suffix
std::string EncodeKeyHybrid(int64_t value, size_t length);

// 2. Random: Purely random bytes (High Entropy, Low Order)
std::string EncodeKeyRandom(int64_t value, size_t length);

// 3. Sequential: Zero-padded sequential number (Low Entropy, High Order)
std::string EncodeKeySequential(int64_t value, size_t length);

#endif // KEY_GENERATOR_H
