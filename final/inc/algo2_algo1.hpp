#ifndef ALGO2_ALGO1_CUH_
#define ALGO2_ALGO1_CUH_

#include<util.hpp>
#include<mutex>
#include <condition_variable>
#include <thread>

namespace algo2_algo1 {
  using namespace algo2lib;


/*
types: ffttaabbccddeeee
  t: 4方どこをそろえているか(3: 右上, 2: 右下, 1: 左下, 0: 左上)
  a: 右上の状況
  f: 盤面縮小回数(00=24のとき、01=20, 10=16, 11=12.)

*/

struct ControlThread;

struct MemObj21 {
  inline static constexpr uint32_t BEAM_WIDTH = 1 << 12;
  inline static constexpr size_t BLOCKS_PER_GRID   = 512;
  inline static constexpr size_t THREADS_PER_BLOCK = 256;
  inline static constexpr size_t GPU_PROCESS_NUM = BLOCKS_PER_GRID * THREADS_PER_BLOCK_NUM;
  inline static constexpr size_t ROT_DEPTH = 2;

  // cpu
  std::vector<uint16_t> fids;
  std::vector<uint8_t> opes_cpu;
  std::vector<uint16_t> types;
  ControlThread ct;

  // gpu
  uint16_t *fields_gpu;
  uint16_t *fields_gpu_buf;
  uint16_t *types_gpu;
  uint16_t *result_gpu;
};

struct TasksQueue {
  std::vector<std::array<uint16_t, MemObj2::ROT_DEPTH+2>> data; // fid(16), rid1(16), rid2(16), rid3(16)
  std::vector<uint16_t> types;
  std::vector<int32_t> scores;

  inline TasksQueue() {
    this->data.reserve(MemObj2::BEAM_WIDTH);
    this->types.reserve(MemObj2::BEAM_WIDTH);
    this->scores.reserve(MemObj2::BEAM_WIDTH);
  }


  inline void sift_up(int idx) {
    while (idx > 0) {
      int parent = (idx - 1) / 2;
      if (scores[parent] <= scores[idx]) break;
      std::swap(scores[parent], scores[idx]);
      std::swap(data[parent], data[idx]);
      std::swap(types[parent], types[idx]);
      idx = parent;
    }
  }

  inline void sift_down(int idx) {
    int n = heap.size();
    while (true) {
      int left = idx * 2 + 1;
      int right = idx * 2 + 2;
      int smallest = idx;

      if (left < n && scores[left] < scores[smallest]) {
        smallest = left;
      }
      if (right < n && scores[right] < scores[smallest]) {
        smallest = right;
      }

      if (smallest == idx) break;

      std::swap(scores[idx], scores[smallest]);
      std::swap(data[idx], data[smallest]);
      std::swap(types[idx], types[smallest]);
      idx = smallest;
    }
  }

  inline bool empty() const {
    return scores.empty();
  }

  inline int size() const {
    return scores.size();
  }

  inline void clear() {
    data.clear();
    types.clear();
    scores.clear();
  }
  inline void push(uint64_t d, uint16_t type, int32_t score) {
    data.push_back(d);
    types.push_back(type);
    scores.push_back(score);
    sift_up(scores.size() - 1);
  }

  inline void pop() {
    swap(scores[0], scores.back());
    swap(data[0], data.back());
    swap(types[0], types.back());
    scores.pop_back();
    data.pop_back();
    types.pop_back();
    if (!scores.empty()) sift_down(0);
  }

  // pop + push 同時
  inline std::array<uint32_t, 4> replace(uint64_t d, uint16_t type, int32_t score) {
    std::array<uint32_t, 4> ret = {data[0] >> 32, (data[0] >> 16) & 0xffff, data[0] & 0xffff, types[0]};
    data[0] = d;
    types[0] = type;
    scores[0] = score;
    sift_down(0);
    return ret;
  }
};


struct ControlThread {
  std::mutex gpu_mtx;
  std::condition_variable gpu_cv;
  bool gpu_stop = false;

  std::mutex afterTask_mtx;
  std::condition_variable afterTask_cv;
  bool afterTask_stop = false;

  uint16_t *result;
  uint16_t *result_buf;
  uint32_t field_len;
  uint32_t fsize;

  std::vector<std::vector<Ope>> resultOperations;
  std::vector<std::vector<Ope>> bresultOperations;
  TasksQueue tq;
  std::vector<std::vector<Ope>>& resultopes;
  

  inline void wait_gpu() {
    std::unique_lock<std::mutex> lock(this->gpu_mtx);
    gpu_stop = false;
    cv.wait(lock, []{ return gpu_stop; });
  }

  inline void start_gpu() {
    std::unique_lock<std::mutex> lock(this->gpu_mtx);
    cv.wait(lock, []{ return !gpu_stop; });
    gpu_stop = true;
  }

  inline void join_gpu() {
    std::unique_lock<std::mutex> lock(this->gpu_mtx);
    cv.wait(lock, []{ return !gpu_stop; });
  }


  inline void wait_afterTask() {
    std::unique_lock<std::mutex> lock(this->afterTask_mtx);
    afterTask_stop = false;
    cv.wait(lock, []{ return afterTask_stop; });
  }

  inline void start_afterTask() {
    std::unique_lock<std::mutex> lock(this->afterTask_mtx);
    cv.wait(lock, []{ return !afterTask_stop; });
    std::swap(this->result, this->result_buf);
    afterTask_stop = true;
  }

  inline void join_afterTask() {
    std::unique_lock<std::mutex> lock(this->afterTask_mtx);
    cv.wait(lock, []{ return !afterTask_stop; });
  }

  inline void setAfterTask() {
    this->start_afterTask();
  }

  inline void tq_add(std::array<uint16_t, MemObj2::ROT_DEPTH+2> d, uint16_t type, uint32_t depth) {
    uint32_t nowfsize = fsize - ((type >> 12) & 0b1100);
    uint32_t pairnum = (fsize << 1) * nowfsize - nowfsize * nowfsize + (type & 0b1111) + __builtin_popcount((int)(type & 0b111111110000)) * (nowfsize >> 1);
    int32_t score = (resultOperations[d[0]].size() / pairnum) * 100000;
    
    if(tq.size() < MemObj2::BEAM_WIDTH) {
      tq.push(d, type, score);
    }else{
      tq.replace(d, type, score);
    }
  }

  void afterTask();




}


}
#endif
