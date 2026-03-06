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
  std::vector<std::vector<Ope>> opes;
  std::vector<std::vector<uint16_t>> fields;
  std::vector<std::pair<uint8_t, uint8_t>> offsets;
  algorithm1(field, fsize, mem1, opes, fields, offsets);;

  // algorithm 2
  std::array<Ope, 350> result = algorithm2(fields, opes, offsets, fsize, mem2);

  // submission
  submission(result);
 
  return 0;
}
