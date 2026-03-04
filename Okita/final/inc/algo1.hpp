#ifndef ALGO1_HPP_
#define ALGO1_HPP_

#include <util.hpp>
#include <array>
#include <vector>
#include <stdint.h>

struct MemObj1 {
  int size;
};

MemObj1 init1();
void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields);



// スタブ関数
#include <algo2lib.hpp>
#include <random>
inline MemObj1 init1() { return MemObj1{1}; }
inline void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields){
  static std::random_device rd;
  static std::mt19937 gen(rd());
  const uint32_t maxidx = algo2lib::getTidPerField(fsize, 2, fsize, 1);
  std::uniform_int_distribution<> dist1(1, 5);
  std::uniform_int_distribution<> dist2(0, maxidx);
  for(size_t i = 0; i < 20; i++) {
    std::vector<Ope> o;
    RawField f = field;
    uint32_t loop = dist1(gen);
    for(size_t j = 0; j < loop; ++j) {
      Ope ope = algo2lib::getParamsCpu(dist2(gen), fsize, 2, 1);
      algo2lib::rotateField(f, fsize, ope);
      o.push_back(ope);
    }
    fields.push_back(f);
    opes.push_back(o);
  }
}

#endif
