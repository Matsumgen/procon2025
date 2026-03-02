#ifndef ALGO1_HPP_
#define ALGO1_HPP_

class MemObj1 {
  int size;
};

MemObj1 init1();
void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::array<Ope, 350>>& opes, std::vector<RawField>& fields);



// スタブ関数
MemObj1 init1() { return MemObj1{1}; }
void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::array<Ope, 350>>& opes, std::vector<RawField>& fields){
  return;
}

#endif
