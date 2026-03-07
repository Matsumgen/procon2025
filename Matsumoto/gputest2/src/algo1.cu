#define REVIEW_NUM 100
/*
評価関数をgpu処理

cudaHostAlloc
cudaHostGetDevicePointer
レイテンシ(処理応答)は遅い
スループット(帯域)は大きい

N: thread数(CPU_THREAD_NUM)
M: beam幅(BEAM_WIDTH)
R: 1Fieldあたりの回転の状態数
・vector使用
gpu - cpu:    O(M) * MR/N
mearge:       O(N + MlogN)

・top-K(min-heap)使用
gpu - cpu:    O(logM) * MR/N + O(MlogM)
mearge:       O(N + MlogN)
*/

#include <cuda_runtime.h>
#include <algo1.cuh>
#include <cpu_process.hpp>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <queue>
#include <chrono>
#include <algorithm>
#include <param.hpp>

#include <chrono>
#include <numeric>

namespace bs1{
#include <gpu_process.cuh>

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
    std::unique_lock<std::mutex> lock(this->emp_num_mtx);
    cv.wait(lock, [&] { return this->emp_num.load() >= need_emp; });
  }

  void clear() {
    this->tail = 0;
    this->emp_num.store(QUEUE_SIZE);
  }
};

__device__  __forceinline__
uint32_t evaluation1(uint16_t *f, uint32_t fsize) {
  uint32_t val = 0, x, y, i;
  uint32_t fs = fsize - 1;
  val += (uint32_t)(f[0] == f[1]) + (uint32_t)(f[0] == f[fsize])
        +(uint32_t)(f[fs] == f[fsize - 2]) + (uint32_t)(f[fs] == f[2 * fsize - 1])
        +(uint32_t)(f[fs * fsize] == f[(fsize - 2) * fsize]) + (uint32_t)(f[fs * fsize] == f[fs * fsize + 1])
        +(uint32_t)(f[fsize * fsize - 1] == f[fsize * fsize - 2]) + (uint32_t)(f[fsize * fsize - 1] == f[fs * fsize - 1]);
  for(x = 1; x < fs; ++x) {
    val += (uint32_t)(f[x] == f[x - 1]) + (uint32_t)(f[x] == f[x + 1]) +(uint32_t)(f[x] == f[fsize + x])
          +(uint32_t)(f[fs * fsize + x] == f[fs * fsize + x - 1])
          +(uint32_t)(f[fs * fsize + x] == f[fs * fsize + x + 1])
          +(uint32_t)(f[fs * fsize + x] == f[(fsize - 2) * fsize + x]);
  }
  for(y = 1; y < fsize - 1; ++y) {
    val += (uint32_t)(f[y * fsize] == f[(y - 1) * fsize]) + (uint32_t)(f[y * fsize] == f[(y + 1) * fsize]) +(uint32_t)(f[y * fsize] == f[y * fsize] + 1)
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[y * fsize - 1])
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[(y + 2) * fsize - 1])
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[(y + 1) * fsize - 2]);
  }
  for(y = 1; y < fsize - 1; ++y) {
    for(x = 1; x < fsize - 1; ++x) {
      i = y * fsize + x;
      val += (uint32_t)(f[i] == f[i-fsize]) + (uint32_t)(f[i] == f[i+fsize])
            +(uint32_t)(f[i] == f[i-1])     + (uint32_t)(f[i] == f[i+1]);
    }
  }
  return val;
}

// ★ ペア同士の「距離」に応じた5段階の重み（オーバーフロー防止のため 256:64:16:4:1）
#define WEIGHT_STAGE1  64 // 段階1: 完全にくっついている（距離1：隣接）
#define WEIGHT_STAGE2  8 // 段階2: あと1歩（距離2：斜め、1マス空き）
#define WEIGHT_STAGE3  1 // 段階3: あと2歩（距離3：2マス空き）
#define WEIGHT_STAGE4   1 // 段階4: あと3歩（距離4：3マス空き）
#define WEIGHT_STAGE5   1 // 段階5: あと4歩（距離5：4マス空き）

__device__  __forceinline__
uint32_t evaluation2(uint16_t *f, uint32_t fsize) {
  uint32_t val = 0;

  // --- 段階1：完成したペア（縦横の隣接） ---
  val += evaluation1(f, fsize) * WEIGHT_STAGE1;

  // --- 段階2：あと1歩のペア（斜め、縦横の1マス空き） ---
  uint32_t stage2_val = 0;
  // 斜め
  for(uint32_t y = 0; y < fsize - 1; ++y) {
    for(uint32_t x = 0; x < fsize - 1; ++x) {
      uint32_t i = y * fsize + x;
      stage2_val += (uint32_t)(f[i] == f[i + fsize + 1]);
      stage2_val += (uint32_t)(f[i + 1] == f[i + fsize]);
    }
  }
  // 横方向の1マス空き
  for(uint32_t y = 0; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize - 2; ++x) {
      stage2_val += (uint32_t)(f[y * fsize + x] == f[y * fsize + x + 2]);
    }
  }
  // 縦方向の1マス空き
  for(uint32_t y = 0; y < fsize - 2; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      stage2_val += (uint32_t)(f[y * fsize + x] == f[(y + 2) * fsize + x]);
    }
  }
  val += stage2_val * WEIGHT_STAGE2;

  // --- 段階3：あと2歩のペア（縦横の2マス空き） ---
  uint32_t stage3_val = 0;
  for(uint32_t y = 0; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize - 3; ++x) {
      stage3_val += (uint32_t)(f[y * fsize + x] == f[y * fsize + x + 3]);
    }
  }
  for(uint32_t y = 0; y < fsize - 3; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      stage3_val += (uint32_t)(f[y * fsize + x] == f[(y + 3) * fsize + x]);
    }
  }
  val += stage3_val * WEIGHT_STAGE3;

  // --- 段階4：あと3歩のペア（縦横の3マス空き） ---
  uint32_t stage4_val = 0;
  for(uint32_t y = 0; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize - 4; ++x) {
      stage4_val += (uint32_t)(f[y * fsize + x] == f[y * fsize + x + 4]);
    }
  }
  for(uint32_t y = 0; y < fsize - 4; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      stage4_val += (uint32_t)(f[y * fsize + x] == f[(y + 4) * fsize + x]);
    }
  }
  val += stage4_val * WEIGHT_STAGE4;

  // --- 段階5：あと4歩のペア（縦横の4マス空き） ---
  uint32_t stage5_val = 0;
  for(uint32_t y = 0; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize - 5; ++x) {
      stage5_val += (uint32_t)(f[y * fsize + x] == f[y * fsize + x + 5]);
    }
  }
  for(uint32_t y = 0; y < fsize - 5; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      stage5_val += (uint32_t)(f[y * fsize + x] == f[(y + 5) * fsize + x]);
    }
  }
  val += stage5_val * WEIGHT_STAGE5;

  return val;
}

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
    int eval_type
  ) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;
  uint32_t rid = start_rid + gpu_id;
  const uint32_t current_field = start_field + ((uint32_t)(rid / ridsPerField)) * FIELDS_PER_THREAD;
  if(current_field >= field_len) return; 
  rid %= ridsPerField;

  uint32_t exec_field_len = field_len - current_field;
  if(exec_field_len > FIELDS_PER_THREAD) exec_field_len = FIELDS_PER_THREAD;

  uint16_t *field = fields + current_field * field_size;

  uint32_t X, Y, N;
  uint32_t fields4 = field_size >> 2;
  uint32_t rot[2];
  getParams(rid, fsize, &X, &Y, &N);
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

    // 評価
    if (eval_type == 1) {
      scores[cf] = evaluation1(next_field, fsize) + 1;
    } else {
      scores[cf] = evaluation2(next_field, fsize) + 1;
    }

    // 次の盤面
    field += field_size;
  }
  for(; cf < FIELDS_PER_THREAD; ++cf) { scores[cf] = 0; }

  // 書き込み
  const uint32_t slot = (start_slot + gpu_id) % QUEUE_SIZE;
  for(uint32_t i=0; i < (FIELDS_PER_THREAD >> 1); ++i){
    q->scores[slot * (FIELDS_PER_THREAD >> 1) + i] = (scores[2*i] << 16) | (scores[2*i+1] & 0xffff);
  }
  q->done[slot] = 1;
  __threadfence_system();
}

__global__
void beam_search_kernel_depth1_after(uint32_t fsize, uint32_t field_size, uint64_t *tasks_gpu,uint32_t idx, uint16_t *df, uint16_t *next_df) {
  const uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if(tid < idx) {
    uint64_t task = tasks_gpu[tid];
    uint16_t *field = df + ((uint32_t)(task & 0xffffffff)) * field_size;
    uint16_t *next_field = next_df + tid * field_size;

    uint32_t X, Y, N;
    uint32_t fields4 = field_size >> 2;
    uint32_t rot[2];
    getParams((uint32_t)(task >> 32), fsize, &X, &Y, &N);
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
    int eval_type
  ) {
  const uint32_t tasksPerProcess = blocksPerGrid * threadsPerBlock;
  int32_t flen = field_len;
  uint32_t endf = 0, start_rid = 0, start_slot = 0, start_field = 0;
  while(true) {
    counter.addTask(tasksPerProcess);
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, ridsPerField, df, field_len, field_size, dq, start_rid, start_field, start_slot, eval_type);

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
}

void beamSearch(TasksQueue& queue, uint32_t beam_width, tail_counter& counter) {
  uint16_t val[FIELDS_PER_THREAD];
  uint32_t rid = 0, fid = 0;
  queue.clear();
  while(true) {
    if(counter.get(val, &rid, &fid)) {
      for(uint32_t i = 0; i < FIELDS_PER_THREAD && val[i] != 0; ++i){
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
      queue.sortVector();
      break;
    }else{
      std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
  }
}


std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize, int eval_type) {
  const uint32_t field_size = fsize * fsize;
  const uint32_t ridsPerField = tidPerField_list[fsize];

  if(BLOCKS_PER_GRID * THREADS_PER_BLOCK >= QUEUE_SIZE) {
    printf("ERROR: blocksPerGrid * threadsPerBlock >= QUEUE_SIZE\n");
    return std::vector<uint32_t>();
  }

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
  cudaMalloc(&df, BEAM_WIDTH * field_size * sizeof(uint16_t));
  cudaMalloc(&next_df, BEAM_WIDTH * field_size * sizeof(uint16_t));
  cudaMalloc(&tasks_gpu, BEAM_WIDTH * sizeof(uint64_t));

  cudaHostAlloc(&hq, sizeof(ResultQueue), cudaHostAllocMapped);
  cudaHostGetDevicePointer(&dq, hq, 0);
  std::vector<std::thread> threads;
  uint32_t depth = 0;

  tail_counter counter;
  for(size_t i = 0; i < QUEUE_SIZE; ++i) hq->done[i] = 0;
  counter.q = hq;
  counter.ridsPerField = ridsPerField;

  depth = 0;
  field_len = 1;
  counter.clear();
  counter.field_len = 1;
  threads.clear();

  cudaMemcpy(df, start_field, field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);

  while(true) {
    // ★セグフォ対策（深さ上限のチェック）
    if (depth >= MAX_DEPTH) {
      std::cout << "  [Warning] 最大探索深さ (" << MAX_DEPTH << ") に到達したため探索を打ち切りました。" << std::endl;
      break;
    }

    // eval_type を渡してGPU実行
    std::thread gthread([&](){run_gpu(beam_search_kernel_depth1, BLOCKS_PER_GRID, THREADS_PER_BLOCK, fsize, ridsPerField, df, field_len, field_size, dq, counter, eval_type);});
    
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], BEAM_WIDTH, counter);});
    }

    gthread.join();
    for(auto& t : threads){ t.join(); }

    std::priority_queue<Node, std::vector<Node>, NodeCompare> pq;
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i) {
      if(!threadQueues[i].q_empty()) {
        pq.push({threadQueues[i].get(), i});
      }
    }

    Node cur = pq.top();

    // ★評価方法に合わせてクリア基準（満点）を自動計算する
    uint32_t clear_score = field_size;
    if (eval_type == 2) {
      // 段階1のペア数がクリア基準(field_size - 1)に達していればクリア
      clear_score = (field_size - 1) * WEIGHT_STAGE1; 
    }

    if (cur.t.score >= clear_score) {
      resultOperations[0] = bresultOperations[cur.t.fid];
      resultOperations[0][depth] = cur.t.rid;
      depth += 1;
      break;
    }

    for(field_len = 0; field_len < BEAM_WIDTH && !pq.empty(); ++field_len) {
      cur = pq.top();
      pq.pop();
      tasks[field_len] = cur.t.getTask();
      resultOperations[field_len] = bresultOperations[cur.t.fid];
      resultOperations[field_len][depth] = cur.t.rid;
      if(!threadQueues[cur.thidx].q_empty()) {
        pq.push({threadQueues[cur.thidx].get(), cur.thidx});
      }
    }

    cudaMemcpy(tasks_gpu, tasks.data(), field_len * sizeof(uint64_t), cudaMemcpyHostToDevice);

    uint32_t bp = (field_len / THREADS_PER_BLOCK) + 1;
    beam_search_kernel_depth1_after<<<bp, THREADS_PER_BLOCK>>>(fsize, field_size, tasks_gpu, field_len, df, next_df);

    depth += 1;
    threads.clear();
    counter.clear();
    counter.field_len = field_len;
    cudaDeviceSynchronize();
    std::swap(df, next_df);
    std::swap(resultOperations, bresultOperations);
  }

  cudaFree(tasks_gpu);
  cudaFree(df);
  cudaFree(next_df);
  cudaFreeHost(hq);
  resultOperations[0].resize(depth);
  return resultOperations[0];
}

} // namespace bs1