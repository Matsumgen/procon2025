#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <array>
#include <vector>
#include <stdint.h>
#include <random>
#include <algorithm>

typedef std::vector<uint16_t> RawField;


// function define

struct Ope {
  std::array<uint8_t, 3> data;
  Ope();
  Ope(uint8_t x, uint8_t y, uint8_t n);
  Ope(int x, int y, int n);
  bool operator < (const Ope &other);
  uint8_t x() const;
  uint8_t y() const;
  uint8_t n() const;
};

void getProblem(RawField& field, uint32_t& fsize);
void submission(std::vector<Ope> result);
RawField createRandomField(uint32_t fsize, size_t seed=0);

//Inline implementation
inline Ope::Ope() : data({0, 0, 0}) {}
inline Ope::Ope(uint8_t x, uint8_t y, uint8_t n) : data({x, y, n}) {}
inline Ope::Ope(int x, int y, int n) : data({static_cast<uint8_t>(x), static_cast<uint8_t>(y), static_cast<uint8_t>(n)}) {}

inline bool Ope::operator<(const Ope &other) {
  return this->data[0] < other.data[0]
        || (this->data[0] == other.data[0] && this->data[1] < other.data[1])
        || (this->data[0] == other.data[0] && this->data[1] == other.data[1] && this->data[2] < other.data[2]);
}

inline uint8_t Ope::x() const { return this->data[0]; }
inline uint8_t Ope::y() const { return this->data[1]; }
inline uint8_t Ope::n() const { return this->data[2]; }

inline RawField createRandomField(uint32_t fsize, size_t seed) {
  const uint16_t n = fsize * fsize / 2;
  RawField result;
  result.reserve(n * 2);

  for(uint16_t i = 0; i < n; ++i) {
    result.push_back(i);
    result.push_back(i);
  }

  if(seed == 0){
    static std::random_device rd;
    seed = rd();
  }

  static std::mt19937 gen(seed);
  std::shuffle(result.begin(), result.end(), gen);
  return result;
}

// Stub function
inline void getProblem(RawField& field, uint32_t& fsize) {
  fsize = 24;
  field = createRandomField(fsize, 0);
}

#include<iostream>
inline void submission(std::vector<Ope> result) {
  std::cout << "result size: " << result.size() << std::endl;
  return;
}

#endif
