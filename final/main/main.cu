#include <cuda_runtime.h>
#include <algo1.hpp>
#include <algo2.cuh>
#include <vector>
#include <array>

int main(void) {
  cudaFree(0);

  // Initialization
  MemObj1 mem1 = init1();
  MemObj2 mem2 = init2();

  // get problem
  RawField field;
  uint32_t fsize;
  getProblem(field, fsize);

  // algorithm 1
  std::vector<std::array<Ope, 350>> opes;
  std::vector<std::vector<uint16_t>> fields;
  algorithm1(field, fsize, mem1, opes, fields);;

  // algorithm 2
  std::array<Ope, 350> result = algorithm2(fields, opes, fsize, mem2);

  // submission
  submission(result);
 
  return 0;
}
