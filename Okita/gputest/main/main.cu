#include <algo1.cuh>
#include <vector>
#include <cpu_process.hpp>
#include <chrono>
#include <cmath>
#include <numeric>
#include <iostream>
#include <param.hpp>
#include <cuda_runtime.h>

#define SAMPLE_NUM 20

int main(void) {
  cudaFree(0);
  uint32_t fsize = 12;
  std::vector<uint16_t> start_field = makeShuffledPairs(fsize);
  auto result = bs4::algo(start_field.data(), fsize);
 
  return 0;
}
