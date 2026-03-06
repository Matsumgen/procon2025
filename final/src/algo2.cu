#include <algo2.cuh>
#include <cuda_runtime.h>
#include <iostream>
#include <vector>
#include <array>

MemObj2::MemObj2(const uint32_t fsize)
: resultOperations(std::vector<std::vector<Ope>>(MemObj2::BEAM_WIDTH)),
  bresultOperations(std::vector<std::vector<Ope>>(MemObj2::BEAM_WIDTH)),
  start_idx(MemObj2::BEAM_WIDTH),
  bstart_idx(MemObj2::BEAM_WIDTH),
  tasks(std::vector<uint64_t>(MemObj2::BEAM_WIDTH, 0))
{
  const size_t field_size = fsize * fsize;
  TasksQueue::fieldsiz.resize(MemObj2::BEAM_WIDTH);

  for(size_t i = 0; i < MemObj2::BEAM_WIDTH; ++i) {
    this->resultOperations[i].reserve(350);
    this->bresultOperations[i].reserve(350);
  }

  this->threadQueues.reserve(MemObj2::CPU_THREAD_NUM);
  for(size_t i = 0; i < MemObj2::CPU_THREAD_NUM; ++i) {
    std::vector<algo2lib::Tasks> buf;
    buf.reserve(MemObj2::BEAM_WIDTH);
    this->threadQueues.emplace_back(algo2lib::TasksCompare{}, std::move(buf));
  }

  cudaError_t err = cudaMalloc(&df, MemObj2::BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }
  err = cudaMalloc(&next_df, MemObj2::BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }
  err = cudaMalloc(&tasks_gpu, MemObj2::BEAM_WIDTH * sizeof(uint64_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }

  cudaHostAlloc(&hq, sizeof(MemObj2::ResultQueue), cudaHostAllocMapped);
  cudaHostGetDevicePointer(&dq, hq, 0);
  for(size_t i = 0; i < QUEUE_SIZE; ++i) hq->done[i] = 0;
}

MemObj2::MemObj2() : MemObj2(MemObj2::MAX_FSIZE) {}










MemObj2 init2() {
  return MemObj2{};
}

std::vector<Ope> algorithm2(std::vector<RawField>& fields, std::vector<std::vector<Ope>>& opes, uint32_t fsize, MemObj2& mem2){
  return algo2_1::algo2_1(fields, opes, fsize, mem2);
}



