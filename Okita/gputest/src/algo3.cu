#define REVIEW_NUM 20
/*
途中まで省いて終了近づいたら完全にする
Nが大きな回転が存在するほど動作が遅くなる
*/

#include <cuda_runtime.h>
#include <algo1.cuh>
#include <cpu_process.hpp>
#include <vector>
#include <mutex>
#include <cmath>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <queue>
#include <chrono>
#include <algorithm>
#include <param.hpp>
#include <set>

#include <chrono>
#include <numeric>

namespace bs3{
#include <gpu_process.cuh>

const uint32_t EVALUATION = 1;
const uint32_t SN = 5;
const uint32_t EN = 12;
const uint32_t SLICE = 1;
const double   THRESHOLD = 0.75;

void getParamMode(uint32_t depth, uint32_t fsize, uint32_t *paramMode) {
  uint32_t a = (fsize * (fsize >> 1)) * THRESHOLD;
  if(depth == a) {
    *paramMode = 1;
  }
}
void getParamMode(uint32_t depth, uint32_t fsize, uint32_t *paramMode, uint32_t *ridsPerField, uint32_t *cr) {
  uint32_t a = (fsize * (fsize >> 1)) * THRESHOLD;
  if(depth == a) {
    *paramMode = 1;
    *ridsPerField = getTidPerField(fsize, 2, fsize, 1);
    *cr = *ridsPerField;
  }
}

void gparamcpu(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t paramMode) {
  uint32_t sn = 2, slice = 1;
  if(paramMode == 0){
    sn = SN;
    slice = SLICE;
  }
  getParamsCpu(tid, fsize, X, Y, N, sn, slice);
}

__device__ __forceinline__
void gparam(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t paramMode) {
  uint32_t sn = 2, slice = 1;
  if(paramMode == 0) {
    sn = SN;
    slice = SLICE;
  }
  getParams(tid, fsize, X, Y, N, sn, slice);
}



// マンハッタン距離の2乗分の一  80~90
// canMove                      70~80
__device__ __forceinline__
uint32_t evaluation2(uint16_t *f, uint32_t fsize) {
  int8_t pos[24*24*2];
  size_t i, j;
  size_t ms = fsize * fsize * 2;
  for(size_t i = 0; i < ms; i += 4) pos[i] = -1;

  i = 0;
  for(int8_t y = 0; y < fsize; ++y){
    for(int8_t x = 0; x < fsize; ++x){
      j = f[i] << 2;
      if(pos[j] != -1){ j += 2; }
      pos[j]   = x;
      pos[j+1] = y;
      ++i;
    }
  }

  uint32_t val = 0;
  for(size_t i = 0; i < ms; i += 4) {
    uint32_t r = abs(pos[i] - pos[i+2]) + abs(pos[i+1] - pos[i+3]); ;
    val += 1 / (r * r);
    /* int8_t *p1 = pos + i; */
    /* int8_t *p2 = p1 + 2; */
    /* if(r == 1){ */
    /*   val += 1 << 6; */
    /* }else{ */
    /*   val += canMove(p1, p2, fsize) + canMove(p2, p1, fsize); */
    /* } */
  }
  return val;
}

__device__  __forceinline__
uint32_t evaluation1(uint16_t *f, uint32_t fsize) {
  uint32_t val = 0, x, y, i;
  uint32_t fs = fsize - 1;
  uint32_t sfs = fsize * fsize - 1;
  uint8_t hash = 0;
  for(y = 0, i = 0; y < fs; ++y) {
    for(x = 0; x < fs; ++x) {
      val += (uint32_t)(f[i] == f[i+1]) + (uint32_t)(f[i] == f[i+fsize]);
      ++i;
      hash *= f[i] & 0b10110101;
      hash >>= 2;
    }
    ++i;
  }
  for(i = fs * fsize; i < sfs; ++i) {
    val += (uint32_t)(f[i] == f[i+1]);
    hash *= f[i] & 0b10110101;
    hash >>= 2;
  }
  for(i = fs; i < sfs; i += fsize) {
    val += (uint32_t)(f[i] == f[i+fsize]);
    hash *= f[i] & 0b10110101;
    hash >>= 2;
  }
  return (val << 6) | (hash & 0b111111);
}


__device__ __forceinline__
uint32_t evaluation(uint16_t *f, uint32_t fsize) {
  if(EVALUATION == 1) {
    return evaluation1(f, fsize);
  }else if(EVALUATION == 2) {
    return evaluation2(f, fsize);
  }
  return 0;
}

struct Tasks {
  uint32_t fid;
  uint32_t rid;
  uint32_t score;

  uint64_t getTask() {
    return (((uint64_t)rid) << 32) | fid;
  }
};

// min-heap
struct TasksCompare {
  bool operator()(const Tasks& a, const Tasks&b) const {
    return a.score > b.score;
  }
};

// 禁忌に手を出している
struct TasksQueue : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare> {
    size_t current_index;
    TasksQueue(const TasksCompare& comp = TasksCompare{}) : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare>(comp), current_index(0) {}
    TasksQueue(const TasksCompare& comp, std::vector<Tasks>&& v) : std::priority_queue<Tasks, std::vector<Tasks>, TasksCompare>(comp, std::move(v)), current_index(0) {}
  void clear() {
    this->c.clear();
    this->current_index = 0;
  }
  void sortVector() {
    std::sort(this->c.rbegin(), this->c.rend(), [](const Tasks& a, const Tasks& b) { return a.score < b.score; });
  }
  Tasks at(size_t i) {
    return this->c[i];
  }
  Tasks get() {
    return this->c[this->current_index++];
  }

  bool q_empty() {
    return this->current_index >= this->size();
  }

};

struct Node {
  Tasks t;
  size_t thidx;
};
struct NodeCompare {
  bool operator()(const Node& a, const Node&b) const {
    return a.t.score < b.t.score;
  }
};

struct ResultQueue {
  uint32_t scores[QUEUE_SIZE * FIELDS_PER_THREAD / 2]; // scoreは16bit, 二つまとめて書き込む
  uint8_t done[QUEUE_SIZE];
};

// tail max: BEAM_WIDTH * ridsPerField (* ridsPerField (if depth2))
// BEAM_WIDTH = 1 << 20のとき、fsize=22まで可能
struct tail_counter {
  ResultQueue *q;
  uint32_t tail;
  uint32_t ridsPerField;
  uint32_t field_len;
  std::atomic<uint32_t> emp_num;
  std::mutex mtx;
  std::mutex emp_num_mtx;
  std::condition_variable cv;

  bool fetch_tail(uint32_t& t, uint32_t& sc) {
    std::lock_guard lock(this->mtx);
    sc =  this->tail % QUEUE_SIZE;
    if(this->q->done[sc]) {
      t = this->tail++;
      return true;
    }
    t = this->tail;
    return false;
  }

  bool get(uint16_t *scores, uint32_t *rid, uint32_t *fid) {
    uint32_t t, sc;
    if(!this->fetch_tail(t, sc)){
      if(this->field_len <= ((uint32_t)(t / this->ridsPerField)) * FIELDS_PER_THREAD) *fid = 0xffffffff;
      return false;
    }
    *fid = ((uint32_t)(t / this->ridsPerField)) * FIELDS_PER_THREAD;
    if(this->field_len <= *fid) {
      *fid = 0xffffffff;
      return false;
    }

    for(size_t i = 0; i < (FIELDS_PER_THREAD >> 1); ++i) {
      uint32_t score = this->q->scores[(FIELDS_PER_THREAD >> 1) * sc + i];
      scores[2*i] = score >> 16;
      scores[2*i + 1] = score & 0xffff;
    }
    this->q->done[sc] = 0;
    this->emp_num.fetch_add(1);
    *rid = t % this->ridsPerField;
    if(*rid == 0) { this->cv.notify_all(); }
    return true;
  }

  void addTask(uint32_t tasks) {
    std::lock_guard lock(this->mtx);
    this->emp_num.fetch_sub(tasks);
  }
  
  void wait(uint32_t need_emp) {
    /* if(this->emp_num < need_emp) printf("[run gpu] wait\n"); */
    std::unique_lock<std::mutex> lock(this->emp_num_mtx);
    cv.wait(lock, [&] { return this->emp_num.load() >= need_emp; });
    /* printf("wait end need_emp=%d, emp_num=%d\n", need_emp, this->emp_num); */
  }

  void clear() {
    this->tail = 0;
    this->emp_num.store(QUEUE_SIZE);
  }
};
// start_tid: 0 ~ tidsPerField
__global__
void beam_search_kernel_depth1(
    const uint32_t fsize,
    const uint32_t ridsPerField,
    uint16_t *fields,
    const uint32_t field_len,
    const uint32_t field_size,
    ResultQueue    *q,
    const uint32_t start_rid,
    const uint32_t start_field,
    const uint32_t start_slot,
    const uint32_t paramMode
  ) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;
  uint32_t rid = start_rid + gpu_id;
  const uint32_t current_field = start_field + ((uint32_t)(rid / ridsPerField)) * FIELDS_PER_THREAD;
  if(current_field >= field_len) return; 
  rid %= ridsPerField;

  uint32_t exec_field_len = field_len - current_field;
  if(exec_field_len > FIELDS_PER_THREAD) exec_field_len = FIELDS_PER_THREAD;

  /* printf("[gpu %d] rid=%d, current_field=%d, exec_field_len=%d\n", gpu_id, rid, current_field, exec_field_len); */

  uint16_t *field = fields + current_field * field_size;

  uint32_t X, Y, N;
  uint32_t fields4 = field_size >> 2;
  uint32_t rot[2];
  gparam(rid, fsize, &X, &Y, &N, paramMode);
  createMatrixArrayL(X, Y, N, rot);

  uint16_t next_field[576];
  uint32_t scores[FIELDS_PER_THREAD];
  uint32_t cf;
  for(cf = 0; cf < exec_field_len; ++cf){

    // fieldをローカルにコピー
    ushort4 *nf = (ushort4*)next_field;
    ushort4 *f = (ushort4*)field;
    for(uint32_t i = 0; i < fields4; ++i) {
      nf[i] = f[i];
    }
    // 回転
    uint8_t p[2];
    for(uint32_t y = Y, i, j; y < Y + N; ++y) {
      for(uint32_t x = X; x < X + N; ++x) {
        p[0] = x;
        p[1] = y;
        i = y * fsize + x;
        culcDp4a(rot, p);
        j = p[1] * fsize + p[0];
        next_field[i] = field[j];
      }
    }

    /* if(!checkProbrmGpu(next_field, fsize)){ */
    /*   printf("[gpu %d] broken field rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
    /* } */
    /* else{ */
    /*   printf("[gpu %d] rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
    /* } */

    // 評価
    scores[cf] = evaluation(next_field, fsize) + 1;

    // 次の盤面
    field += field_size;
  }
  for(; cf < FIELDS_PER_THREAD; ++cf) { scores[cf] = 0; }

  // 書き込み
  const uint32_t slot = (start_slot + gpu_id) % QUEUE_SIZE;
  for(uint32_t i=0; i < (FIELDS_PER_THREAD >> 1); ++i){
    q->scores[slot * (FIELDS_PER_THREAD >> 1) + i] = (scores[2*i] << 16) | (scores[2*i+1] & 0xffff);
    /* printf("[gpu %d] %d, q->scores[%d] = [%d, %d]\n", gpu_id, i, slot * (FIELDS_PER_THREAD>>1) + i, scores[2*i], scores[2*i+1]); */
  }
  /* printf("[gpu %d] rid=%d, fid=%d, score[%d]=%d %d\n", gpu_id, rid, current_field, slot * (FIELDS_PER_THREAD >> 1), q->scores[slot * (FIELDS_PER_THREAD >> 1)], q->scores[slot * (FIELDS_PER_THREAD >> 1)+1]); */
  q->done[slot] = 1;
  __threadfence_system();
  /* printf("[gpu %d] end\n", gpu_id); */
}

__global__
void beam_search_kernel_depth1_after(uint32_t fsize, uint32_t field_size, uint64_t *tasks_gpu,uint32_t idx, uint16_t *df, uint16_t *next_df, uint32_t paramMode) {
  const uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if(tid < idx) {
    uint64_t task = tasks_gpu[tid];
    uint16_t *field = df + ((uint32_t)(task & 0xffffffff)) * field_size;
    uint16_t *next_field = next_df + tid * field_size;

    uint32_t X, Y, N;
    uint32_t fields4 = field_size >> 2;
    uint32_t rot[2];
    gparam((uint32_t)(task >> 32), fsize, &X, &Y, &N, paramMode);
    createMatrixArrayL(X, Y, N, rot);

    ushort4 *nf = (ushort4*)next_field;
    ushort4 *f = (ushort4*)field;
    for(uint32_t i = 0; i < fields4; ++i) {
      nf[i] = f[i];
    }

    uint8_t p[2];
    for(uint32_t y = Y, i, j; y < Y + N; ++y) {
      for(uint32_t x = X; x < X + N; ++x) {
        p[0] = x;
        p[1] = y;
        i = y * fsize + x;
        culcDp4a(rot, p);
        j = p[1] * fsize + p[0];
        next_field[i] = field[j];
      }
    }
    /* if(!checkProbrmGpu(next_field, fsize)){ */
    /*   printf("[after %d] broken field rid=%d, fid=%d, before_broken=%d\n", tid, (uint32_t)(task >> 32), (uint32_t)(task & 0xffffffff), checkProbrmGpu(field, fsize)); */
    /* } */
    /* else{ */
    /*   printf("[after %d] rid=%d, fid=%d before_broken=%d, next_field=%p\n", tid, (uint32_t)(task >> 32), (uint32_t)(task & 0xffffffff), checkProbrmGpu(field, fsize), next_field); */
    /* } */

    /* if(tid == 0) { */
    /*   uint32_t num[24 * 24]; */
    /*   bool b = false; */
    /*   for(uint32_t i = 0; i < field_size / 2; ++i) num[i] = 0; */
    /*   for(uint32_t y = 0; y < fsize; ++y) { */
    /*     for(uint32_t x = 0; x < fsize; ++x) { */
    /*       uint32_t n = (int)next_field[y * fsize + x]; */
    /*       num[n] += 1; */
    /*       printf("%3d ", n); */
    /*     } */
    /*     printf("\n"); */
    /*   } */
    /*   printf("rid=%d, fid=%d, evaluation=%d\n", (uint32_t)(task >> 32), (uint32_t)(task & 0xffffffff), evaluation(next_field, fsize) + 1); */
    /*   /1* printf("nums = "); *1/ */
    /*   for(uint32_t i = 0; i < field_size / 2; ++i){ */
    /*     /1* printf("[%d]=%d, ", i, num[i]); *1/ */
    /*     if(num[i] != 2) { b = true; } */
    /*   } */
    /*   printf("\n"); */
    /*   if(b){ */
    /*     printf("broken field!!!!!!!!!!!!!!!!!!!!!!!!!\n"); */
    /*   } */
    /* } */
  }
}


template<typename Func>
void run_gpu(
    Func gpu_process,
    const uint32_t blocksPerGrid,
    const uint32_t threadsPerBlock,
    const uint32_t fsize,
    const uint32_t ridsPerField,
    uint16_t *df,
    const uint32_t field_len,
    const uint32_t field_size,
    ResultQueue *dq,
    tail_counter& counter,
    const uint32_t paramMode
  ) {
  const uint32_t tasksPerProcess = blocksPerGrid * threadsPerBlock;
  /* printf("[run_gpu] start: blocksPerGrid=%d, threadsPerBlock=%d, fsize=%d, ridsPerField=%d, field_len=%d, field_size=%d, tasksPerProcess=%d\n", blocksPerGrid, threadsPerBlock, fsize, ridsPerField, field_len, field_size, tasksPerProcess); */
  int32_t flen = field_len;
  uint32_t endf = 0, start_rid = 0, start_slot = 0, start_field = 0;
  while(true) {
    counter.addTask(tasksPerProcess);
    /* printf("[run_gpu] start gpu: start_rid=%d, start_field=%d, start_slot=%d\n", start_rid, start_field, start_slot); */
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, ridsPerField, df, field_len, field_size, dq, start_rid, start_field, start_slot, paramMode);

    cudaDeviceSynchronize();
    endf = (start_rid + tasksPerProcess) / ridsPerField;
    endf *= FIELDS_PER_THREAD;
    flen -= endf;
    if(flen <= 0) break;
    start_field += endf;
    start_slot = (start_slot + tasksPerProcess) % QUEUE_SIZE;
    start_rid = (start_rid + tasksPerProcess) % ridsPerField;

    // queueが空くまで待機
    counter.wait(tasksPerProcess);
  }
  /* printf("[run_gpu] end gpu\n"); */
}

/* void beamSearch(std::vector<uint64_t>& operations, std::vector<uint16_t>& values, uint32_t beam_width, tail_counter& counter) { */
void beamSearch(TasksQueue& queue, uint32_t beam_width, tail_counter& counter) {
  /* printf("[beamSearch] start: beam_width=%d, %p\n", beam_width, &queue); */
  uint16_t val[FIELDS_PER_THREAD];
  uint32_t rid = 0, fid = 0;
  queue.clear();
  while(true) {
    if(counter.get(val, &rid, &fid)) {
      /* printf("[beamSearch] rid=%d, fid=%d\n", rid, fid); */
      for(uint32_t i = 0; i < FIELDS_PER_THREAD && val[i] != 0; ++i){
        /* if(counter.field_len <= fid + i) { */
        /*   printf("[beamSearch] over field rid=%d, fid=%d, field_len=%d,val=%d\n", rid, fid + i, counter.field_len, (int)val[i]); */
        /* } */
        /* else{ */
        /*   printf("[beamSearch]  rid=%d, fid=%d, score=%d\n", rid, fid + i, val[i]); */
        /* } */
        if(queue.size() < beam_width){
          queue.push({fid + i, rid, val[i]});
        }else{
          Tasks cur = queue.top();
          if(cur.score < val[i]) { // 先が優先
            queue.pop();
            queue.push({fid + i, rid, val[i]});
          }
        }
      }
    }else if(fid == 0xffffffff) {
      /* printf("[beamSearch] end %p, %d\n", &queue, (int)queue.size()); */
      queue.sortVector();
      break;
    }else{
      /* printf("[beamSearch] wait\n"); */
      std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
  }
}


std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize) {
  /* printf("start algo\n"); */
  /* printField(fsize, start_field); */
  const uint32_t field_size = fsize * fsize;

  if(BLOCKS_PER_GRID * THREADS_PER_BLOCK >= QUEUE_SIZE) {
    printf("ERROR: blocksPerGrid * threadsPerBlock >= QUEUE_SIZE\n");
    return std::vector<uint32_t>();
  }

  // メモリ確保
  /* printf("start memory\n"); */
  std::vector<std::vector<uint32_t>> resultOperations(BEAM_WIDTH, std::vector<uint32_t>(MAX_DEPTH, 0));
  std::vector<std::vector<uint32_t>> bresultOperations(BEAM_WIDTH, std::vector<uint32_t>(MAX_DEPTH, 0));
  std::vector<uint64_t> tasks(BEAM_WIDTH, 0);
  uint32_t field_len = 1;

  std::vector<TasksQueue> threadQueues;
  threadQueues.reserve(CPU_THREAD_NUM);
  for(size_t i = 0; i < CPU_THREAD_NUM; ++i) {
    std::vector<Tasks> buf;
    buf.reserve(BEAM_WIDTH);
    threadQueues.emplace_back(TasksCompare{}, std::move(buf));
  }

  ResultQueue *hq, *dq;
  uint16_t *df, *next_df;
  uint64_t *tasks_gpu;
  cudaError_t err = cudaMalloc(&df, BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return std::vector<uint32_t>();
  }
  err = cudaMalloc(&next_df, BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return std::vector<uint32_t>();
  }
  err = cudaMalloc(&tasks_gpu, BEAM_WIDTH * sizeof(uint64_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return std::vector<uint32_t>();
  }

  cudaHostAlloc(&hq, sizeof(ResultQueue), cudaHostAllocMapped);
  cudaHostGetDevicePointer(&dq, hq, 0);
  std::vector<std::thread> threads;
  uint32_t depth = 0;

  tail_counter counter;
  for(size_t i = 0; i < QUEUE_SIZE; ++i) hq->done[i] = 0;
  counter.q = hq;

#ifdef REVIEW_NUM
  std::vector<double> times;
  std::vector<int> operations;
  uint32_t dontend = 0;
  for(int loop = 0; loop < REVIEW_NUM; ++loop){
    std::vector<uint16_t> sf = makeShuffledPairs(fsize);
    start_field = sf.data();
    auto start_time = std::chrono::high_resolution_clock::now();
#endif

  // 実行
  depth = 0;
  field_len = 1;
  counter.clear();
  counter.field_len = 1;
  threads.clear();
  uint32_t paramMode = 0;
  uint32_t ridsPerField = getTidPerField(fsize, SN, EN, SLICE);
  counter.ridsPerField = ridsPerField;

  err = cudaMemcpy(df, start_field, field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    cudaFree(df);
    return std::vector<uint32_t>();
  }

  while(true) {
    /* printf("start loop depth=%d\n", depth); */
    // thread生成
    std::thread gthread([&](){run_gpu(beam_search_kernel_depth1, BLOCKS_PER_GRID, THREADS_PER_BLOCK, fsize, ridsPerField, df, field_len, field_size, dq, counter, paramMode);});
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], BEAM_WIDTH, counter);});
    }

    // thread終了待ち
    gthread.join();
    for(auto& t : threads){ t.join(); }

    // 次のfield生成
    // この時点でthreadQueues内のヒープは壊れている
    std::priority_queue<Node, std::vector<Node>, NodeCompare> pq;
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i) {
      if(!threadQueues[i].q_empty()) {
        pq.push({threadQueues[i].get(), i});
      }
    }

    Node cur = pq.top();
    /* printf("start sort queue: pq.size=%d, max:score=%d/%d, rid=%d, fid=%d\n", (int)pq.size(), (int)cur.t.score >> 6, field_size >> 1, cur.t.rid, cur.t.fid); */
    // evalend
    if(cur.t.score >= (field_size << 5)) {
      // 終了処理
      /* printf("end beamsearch: score=%d, rid=%d, fid=%d\n", (int)cur.t.score, cur.t.rid, cur.t.fid); */
      resultOperations[0] = bresultOperations[cur.t.fid];
      resultOperations[0][depth] = cur.t.rid;
      depth += 1;
      break;
    }else if(depth > 500) {
      printf("don't end over 500\n");
      break;
    }
    for(field_len = 0; field_len < BEAM_WIDTH && !pq.empty(); ++field_len) {
      cur = pq.top();
      /* printf("field_len=%d, pq.empty=%d, cur.thidx=%d\n", (int)field_len, (int)pq.empty(), (int)cur.thidx); */
      pq.pop();
      tasks[field_len] = cur.t.getTask();
      resultOperations[field_len] = bresultOperations[cur.t.fid];
      resultOperations[field_len][depth] = cur.t.rid;
      if(!threadQueues[cur.thidx].q_empty()) {
        pq.push({threadQueues[cur.thidx].get(), cur.thidx});
      }
      /* if(field_len < 5) { */
      /*   printf("idx=%d, score=%d, rid=%d, fid=%d\n", field_len, (int)cur.t.score, cur.t.rid, cur.t.fid); */
      /* } */
    }

    /* printf("start next process next field_len=%d\n", field_len); */
    err = cudaMemcpy(tasks_gpu, tasks.data(), field_len * sizeof(uint64_t), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
      std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
      cudaFree(tasks_gpu);
      return std::vector<uint32_t>();
    }

    uint32_t bp = (field_len / THREADS_PER_BLOCK) + 1;
    beam_search_kernel_depth1_after<<<bp, THREADS_PER_BLOCK>>>(fsize, field_size, tasks_gpu, field_len, df, next_df, paramMode);


    // 諸々初期化
    depth += 1;
    threads.clear();
    counter.clear();
    counter.field_len = field_len;
    getParamMode(depth, fsize, &paramMode, &ridsPerField, &counter.ridsPerField);
    cudaDeviceSynchronize();
    std::swap(df, next_df);
    std::swap(resultOperations, bresultOperations);
    /* printf("\n"); */
  }

#ifdef REVIEW_NUM
    if(depth <= 500) {
      auto end_time = std::chrono::high_resolution_clock::now();
      double t = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count()/1000.0;
      times.push_back(t);
      operations.push_back(depth);

      std::vector<uint16_t> field = sf;
      paramMode = 0;
      for(uint32_t i = 0, x, y, n; i < depth; ++i) {
        getParamMode(i, fsize, &paramMode);
        gparamcpu(resultOperations[0][i], fsize, &x, &y, &n, paramMode);
        rotateField(field, fsize, x, y, n);
      }
      if(!isEnd(field, fsize)){
        printf("ERROR: Don't end\n");
        field = sf;
        paramMode = 0;
        printField(fsize, field.data());
        for(uint32_t i = 0; i < depth; ++i) {
          uint32_t x, y, n;
          getParamMode(i, fsize, &paramMode);
          gparamcpu(resultOperations[0][i], fsize, &x, &y, &n, paramMode);
          rotateField(field, fsize, x, y, n);
          printf("rotate: rid=%d, (%d, %d, %d)\n", resultOperations[0][i], x, y, n);
          printField(fsize, field.data());
        }
        printf("depth: %d\n", depth);
      }else{
        std::cout << "time: " << t << " operations: " << depth << std::endl;
      }
    }else{
      dontend += 1;
    }

  }
  auto [ope_min_it, ope_max_it] = std::minmax_element(operations.begin(), operations.end());
  auto [time_min_it, time_max_it] = std::minmax_element(times.begin(), times.end());
  double ope_mean = std::accumulate(operations.begin(), operations.end(), 0) / REVIEW_NUM;
  double time_mean = std::accumulate(times.begin(), times.end(), 0) / REVIEW_NUM;
  double ope_sd = std::accumulate(operations.begin(), operations.end(), 0.0, [ope_mean](double acc, int x){ return acc + (x - ope_mean) * (x - ope_mean); }) / REVIEW_NUM;
  double time_sd = std::accumulate(times.begin(), times.end(), 0.0, [time_mean](double acc, double x){ return acc + (x - time_mean) * (x - time_mean); }) / REVIEW_NUM;
  std::cout << "config:" << std::endl;
  std::cout << "\tfsize:\t\t\t"         << fsize << std::endl;
  std::cout << "\tSN:\t\t\t"            << SN << std::endl;
  std::cout << "\tEN:\t\t\t"            << EN << std::endl;
  std::cout << "\tSLICE:\t\t\t"         << SLICE << std::endl;
  std::cout << "\tTHRESHOLD:\t\t"       << THRESHOLD << std::endl;
  std::cout << "\tEVALUATION:\t\t"      << EVALUATION << std::endl;
  std::cout << "\tblocksPerGrid:\t\t"   << BLOCKS_PER_GRID << std::endl;
  std::cout << "\tthreadsPerBlock:\t"   << THREADS_PER_BLOCK << std::endl;
  std::cout << "\tBEAM_WIDTH:\t\t"      << BEAM_WIDTH << std::endl;
  std::cout << "\tFIELDS_PER_THREAD:\t" << FIELDS_PER_THREAD << std::endl;
  std::cout << "\tCPU_THREAD_NUM:\t\t"  << CPU_THREAD_NUM << std::endl;
  std::cout << "\tQUEUE_SIZE:\t\t"      << QUEUE_SIZE << std::endl;
  std::cout << "time:" << std::endl;
  std::cout << "\tmax:\t"  << *time_max_it << "[sec]" << std::endl;
  std::cout << "\tmin:\t"  << *time_min_it << "[sec]" << std::endl;
  std::cout << "\tmean:\t" << time_mean    << "[sec]" << std::endl;
  std::cout << "\tsd:\t"   << time_sd      << "[sec]" << std::endl;
  std::cout << "operations:" << std::endl;
  std::cout << "\tmax:\t"  << *ope_max_it << std::endl;
  std::cout << "\tmin:\t"  << *ope_min_it << std::endl;
  std::cout << "\tmean:\t" << ope_mean    << std::endl;
  std::cout << "\tsd:\t"   << ope_sd      << std::endl;
  std::cout << "\tmean time per operation:\t" << time_mean / ope_mean<< std::endl;
  std::cout << "\tmean operation per pair:\t" << ope_mean / (field_size >> 1) << std::endl;
  std::cout << "\tDon't end:\t\t" << dontend << std::endl;
#endif

  cudaFree(tasks_gpu);
  cudaFree(df);
  cudaFree(next_df);
  cudaFreeHost(hq);
  resultOperations[0].resize(depth);
  return resultOperations[0];

}

};
