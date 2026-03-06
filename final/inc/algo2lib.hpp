#ifndef ALGO2LIB_HPP_
#define ALGO2LIB_HPP_

#include <algo2.cuh>
#include <array>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>
#include <iostream>

namespace algo2lib {

struct Node { Tasks t; size_t thidx; };
struct NodeCompare { inline bool operator()(const Node& a, const Node&b) const { return a.t.key() < b.t.key(); } };

struct tail_counter {
  MemObj2::ResultQueue *q;
  uint64_t tail;
  uint32_t maxRid;
  uint32_t field_len;
  std::atomic<uint32_t> emp_num;
  std::mutex mtx;
  std::mutex emp_num_mtx;
  std::condition_variable cv;

  bool fetch_tail(uint32_t& t, uint32_t& sc);
  bool get(uint16_t *scores, uint32_t *rid, uint32_t *fid);
  void addTask(uint32_t tasks);
  void wait(uint32_t need_emp);
  void clear();
};

inline Ope getParamsCpu(const uint32_t tid, const uint32_t fsize, uint32_t sn, uint32_t slice) {
  uint32_t acc = 0;
  int32_t w = fsize - sn + 1;
  for(; w >= 0; --w) {
    uint32_t size = w * w;
    size = (size / slice) + (size % slice == 0 ? 0 : 1);
    if(tid < acc + size) break; 
    acc += size;
  }

  acc = (tid - acc) * slice;
  return Ope(static_cast<uint8_t>(acc % w), static_cast<uint8_t>(acc / w), static_cast<uint8_t>(fsize - w + 1));
}

inline uint32_t getTidPerField(uint32_t fsize, uint32_t sn, uint32_t en, uint32_t slice) {
  for(uint32_t t = 0; t < fsize * fsize * fsize; ++t){
    Ope ope = getParamsCpu(t, fsize, sn, slice);
    if(en <= ope.n() || fsize <= ope.n()){
      return t;
    }
  }
  return 0;
}


// debug
void rotateField(RawField& field, const uint32_t fsize, const Ope ope);
void rotateField(RawField& field, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n);
inline void printField(const RawField field, const uint32_t fsize) {
  for(size_t y = 0; y < fsize; ++y) {
    for(size_t x = 0; x < fsize; ++x) {
      size_t i = y * fsize + x;
      printf("%3d ", (int)field[i]);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

inline bool isIndexAdjacent(RawField& field, const uint32_t fsize, const uint32_t i) {
  std::uint16_t num = field[i];
  const size_t x = i % fsize;

  const size_t up    = i >= fsize                 ? i - fsize  : 1<<20;
  const size_t down  = i + fsize < fsize * fsize  ? i + fsize  : 1<<20;
  const size_t left  = x != 0                     ? i - 1      : 1<<20;
  const size_t right = x != fsize-1               ? i + 1      : 1<<20;

  return (up    != 1<<20 && field[up]    == num) ||
         (down  != 1<<20 && field[down]  == num) ||
         (left  != 1<<20 && field[left]  == num) ||
         (right != 1<<20 && field[right] == num);
}

inline bool isEnd(RawField& field, const uint32_t fsize) {
  for(std::uint16_t i=0; i < field.size(); ++i){
    if(!isIndexAdjacent(field, fsize, i)){ return false; }
  }
  return true;
}



}
#endif
