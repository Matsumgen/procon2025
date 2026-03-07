#ifndef ALGO2_CUH_
#define ALGO2_CUH_

#include <util.hpp>
#include <array>
#include <vector>
#include <stdint.h>
#include <queue>

namespace algo2lib{

  struct Tasks {
    std::array<uint32_t, 3> data;
    bool isDepth1;

    Tasks();
    Tasks(uint32_t fid, uint32_t rid, uint32_t score);
    Tasks(uint32_t fid, uint32_t rid, uint32_t score, bool isDepth1);

    uint64_t getTask() const;
    uint32_t key() const;
    uint32_t fid() const;
    uint32_t rid() const;
    uint32_t score() const;
  };

  struct TasksCompare { inline bool operator()(const Tasks& a, const Tasks&b) const { return a.key() > b.key(); } };

  struct TasksQueue : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare> {
    inline static std::vector<uint8_t> fieldsiz{};
    inline static std::vector<uint8_t> start_idx{};
    size_t current_index;
    TasksQueue(const TasksCompare& comp = TasksCompare{});
    TasksQueue(const TasksCompare& comp, std::vector<Tasks>&& v);
    void clear();
    void sortVector();
    Tasks get();
    bool q_empty();
  };



  inline Tasks::Tasks() : data({0, 0, 0}), isDepth1(false) {}
  inline Tasks::Tasks(uint32_t fid, uint32_t rid, uint32_t score) : data({fid, rid, score}), isDepth1(false) {}
  inline Tasks::Tasks(uint32_t fid, uint32_t rid, uint32_t score, bool isDepth1) : data({fid, rid, score}), isDepth1(isDepth1) {}
  inline uint64_t Tasks::getTask() const  { return (static_cast<uint64_t>(this->data[1]) << 32) | this->data[0]; }
  inline uint32_t Tasks::key() const      { return (this->data[2] << 1) | static_cast<uint32_t>(this->isDepth1); }
  inline uint32_t Tasks::fid() const      { return this->data[0]; }
  inline uint32_t Tasks::rid() const      { return this->data[1]; }
  inline uint32_t Tasks::score() const    { return this->data[2]; }

  inline TasksQueue::TasksQueue(const TasksCompare& comp) : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare>(comp), current_index(0) {}
  inline TasksQueue::TasksQueue(const TasksCompare& comp, std::vector<Tasks>&& v) : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare>(comp, std::move(v)), current_index(0) {}
  inline void TasksQueue::clear()      { this->c.clear(); this->current_index = 0; }
  inline void TasksQueue::sortVector() { std::sort(this->c.rbegin(), this->c.rend(),
  [](const Tasks& a, const Tasks& b) {
      uint32_t ak = a.key(), bk = b.key();
      return ak < bk || (ak == bk && TasksQueue::fieldsiz[TasksQueue::start_idx[a.fid()]] < TasksQueue::fieldsiz[TasksQueue::start_idx[b.fid()]]); 
    });
  }
  inline Tasks TasksQueue::get()       { if(this->current_index < this->c.size())return this->c[this->current_index++]; printf("ERROR\n"); return Tasks(); }
  inline bool TasksQueue::q_empty()    { return this->current_index >= this->size(); }
}

#ifndef P_BLOCKS_PER_GRID
#define P_BLOCKS_PER_GRID 512
#endif
#ifndef P_THREADS_PER_BLOCK
#define P_THREADS_PER_BLOCK 256
#endif
#ifndef P_QUEUE_SIZE
#define P_QUEUE_SIZE (1 << 20)
#endif
#ifndef P_BEAM_WIDTH
#define P_BEAM_WIDTH (1 << 18)
#endif
#ifndef P_FIELDS_PER_THREAD
#define P_FIELDS_PER_THREAD 32
#endif
#ifndef P_CPU_THREAD_NUM
#define P_CPU_THREAD_NUM 3
#endif
#ifndef P_SLEEP_TIME
#define P_SLEEP_TIME 10
#endif
#ifndef P_MAX_FSIZE
#define P_MAX_FSIZE 14
#endif
#ifndef P_SN
#define P_SN 5
#endif
#ifndef P_EN
#define P_EN 10
#endif
#ifndef P_SLICE
#define P_SLICE 1
#endif
#ifndef P_THRESHOLD
#define P_THRESHOLD 22
#endif
#ifndef P_EVALUATION
#define P_EVALUATION 1  // デフォルトの評価関数
#endif

struct MemObj2 {
  inline static constexpr size_t BLOCKS_PER_GRID   = P_BLOCKS_PER_GRID;
  inline static constexpr size_t THREADS_PER_BLOCK = P_THREADS_PER_BLOCK;
  inline static constexpr size_t QUEUE_SIZE        = P_QUEUE_SIZE;
  inline static constexpr size_t BEAM_WIDTH        = P_BEAM_WIDTH;
  inline static constexpr size_t FIELDS_PER_THREAD = P_FIELDS_PER_THREAD;
  inline static constexpr size_t CPU_THREAD_NUM    = P_CPU_THREAD_NUM;
  inline static constexpr size_t SLEEP_TIME        = P_SLEEP_TIME;
  inline static constexpr uint32_t MAX_FSIZE       = P_MAX_FSIZE;
  inline static constexpr uint32_t SN              = P_SN;
  inline static constexpr uint32_t EN              = P_EN;
  inline static constexpr uint32_t SLICE           = P_SLICE;
  inline static constexpr uint32_t THRESHOLD       = P_THRESHOLD;
  inline static constexpr uint32_t EVALUATION = P_EVALUATION;
  

  // ... (以降のコードはそのまま

  struct ResultQueue {
    uint32_t scores[QUEUE_SIZE * FIELDS_PER_THREAD / 2];
    uint8_t done[QUEUE_SIZE];
  };

  // cpu memory
  ResultQueue *hq;
  std::vector<std::vector<Ope>> resultOperations;
  std::vector<std::vector<Ope>> bresultOperations;
  std::vector<uint8_t> bstart_idx;
  std::vector<uint64_t> tasks;
  std::vector<algo2lib::TasksQueue> threadQueues;

  // gpu memory
  ResultQueue *dq;
  uint16_t *df;
  uint16_t *next_df;
  uint64_t *tasks_gpu;

  MemObj2();
  MemObj2(const uint32_t fsize);
  ~MemObj2();
};


std::vector<Ope> algorithm2(std::vector<RawField>& fields, std::vector<std::vector<Ope>>& opes, std::vector<std::pair<uint8_t, uint8_t>>& offsets, uint32_t fsize, MemObj2& mem2);

namespace algo2_1 {
  std::vector<Ope> algo2_1(std::vector<RawField>& fields, std::vector<std::vector<Ope>>& opes, std::vector<std::pair<uint8_t, uint8_t>>& offsets, uint32_t fsize, MemObj2& mem2);
}
namespace algo2_2 {
  std::vector<Ope> algo2_2(RawField& fields, std::vector<Ope>& opes, std::pair<uint8_t, uint8_t>& offset, uint32_t fsize, MemObj2& mem2);
}



#endif
