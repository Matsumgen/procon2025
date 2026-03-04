#include <cuda_runtime.h>
#include <algo1.hpp>
#include <algo2.cuh>
#include <vector>
#include <array>
#include <iostream>

int main(void) {
  cudaFree(0);

  // Initialization
  MemObj2 mem2{};
  MemObj1 mem1 = init1();

  // get problem
  RawField field;
  uint32_t fsize;
  getProblem(field, fsize);

  // algorithm 1
  std::vector<std::vector<Ope>> opes;
  std::vector<RawField> fields;
  algorithm1(field, fsize, mem1, opes, fields);

  // algorithm 2
  std::vector<Ope> result = algorithm2(fields, opes, fsize, mem2);

  for(Ope& ope : result) {
    std::cout << (int)ope.x() << " " << (int)ope.y() << " " << (int)ope.n() << std::endl;
  }

 
  return 0;
}
