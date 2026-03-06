#include <cuda_runtime.h>
#include <algo1.hpp>
#include <algo2.cuh>
#include <vector>
#include <array>


// debug
#include <algo2lib.hpp>
using namespace algo2lib;


int main(void) {
  cudaFree(0);

  // Initialization
  MemObj1 mem1 = init1();
  MemObj2 mem2{};

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
  std::vector<Ope> result = algorithm2(fields, opes, offsets, fsize, mem2);

  // debug
  RawField f = field;
  printField(f, fsize);
  std::cout << std::endl;
  for(auto& v : result) {
    rotateField(f, fsize, v);
    std::cout << (int)v.x() << " " << (int)v.y() << " " << (int)v.n() << std::endl;
  }
  std::cout << std::endl;
  printField(f, fsize);

  // submission
  submission(result);
 
  return 0;
}
