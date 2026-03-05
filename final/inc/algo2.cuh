#ifndef ALGO2_CUH_
#define ALGO2_CUH_

#include <util.hpp>
#include <array>
#include <vector>
#include <stdint.h>

struct MemObj2 {
  int size;
};

MemObj2 init2();
std::array<Ope, 350> algorithm2(std::vector<RawField> fields, std::vector<std::array<Ope, 350>> opes, uint32_t fsize, MemObj2& mem2);



// スタブ関数
inline MemObj2 init2() { return MemObj2{1}; }
inline std::array<Ope, 350> algorithm2(std::vector<RawField> fields, std::vector<std::array<Ope, 350>> opes, uint32_t fsize, MemObj2& mem2){
  return;
}

#endif
