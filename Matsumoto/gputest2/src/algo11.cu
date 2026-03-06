/*
評価関数をgpu処理

cudaHostAlloc
cudaHostGetDevicePointer
レイテンシ(処理応答)は遅い
スループット(帯域)は大きい

N: thread数(CPU_THREAD_NUM)
M: beam幅(BEAM_WIDTH)
R: 1Fieldあたりの回転の状態数

・top-K(min-heap)使用
gpu - cpu:    O(logM) * MR/N + O(MlogM)
mearge:       O(N + MlogN)

・top-K & mearge後の配列を分割して使用
gpu - cpu:    O(logM) * MR/N
sort:         O(MlogM)

TasksをそのままGPUに送る？
送る: Tasksをuint32_t data[3]にする. scoreが不要
送らない：前処理にO(M)かかる


盤面縮小メモ
距離行列：24*24のものを論理式にする
2点が1つの長方形の中にあるなら、一発
無いとき、二つの長方形が重なる場所に移動させてからという流れで想定
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

namespace bs11{
#include <gpu_process.cuh>

struct Tasks {
  uint32_t data[3]; // fid, rid, score
};

// min-heap
struct TasksCompare {
  bool operator()(const Tasks& a, const Tasks&b) const {
    return a.data[2] > b.data[2];
  }
};

class TasksQueue {
private:
  Tasks *heap;
  size_t max_size;
  size_t sz;

public:
  TasksQueue(const size_t max_size) : heap(new Tasks[max_size]), max_size(max_size), sz(0) {}
  TasksQueue(Tasks *data, const size_t max_size) : heap(data), max_size(max_size), sz(0) {}

  void push(const Tasks& val) noexcept {
    if(this->sz >= this->max_size) {
      this->replace_top(val);
      return;
    }
    const uint32_t key = val.data[2];
    size_t i = this->sz++;

    // ---- sift-up (hole method) ----
    while (i > 0) {
      size_t parent = (i - 1) >> 1;
      if (this->heap[parent].data[2] <= key)
        break;

      this->heap[i] = this->heap[parent];
      i = parent;
    }
    this->heap[i] = val;
  }

  void pop() noexcept {
    Tasks last = this->heap[--this->sz];
    const uint32_t key = last.data[2];

    size_t i = 0;
    size_t half = sz >> 1;
    size_t child = 1;

    // ---- sift-down (hole method) ----
    while (i < half) {
      size_t right = child + 1;
      if (right < this->sz && this->heap[right].data[2] < this->heap[child].data[2]) {
        ++child;
      }

      if (this->heap[child].data[2] >= key)
        break;
      this->heap[i] = this->heap[child];
      i = child;
      child = (i << 1) + 1;
    }

    this->heap[i] = last;
  }

  void replace_top(const Tasks& val) noexcept {
    const uint32_t key = val.data[2];

    size_t i = 0;
    size_t child = 1;
    size_t half = this->sz >> 1;

    // sift-down only
    while (i < half) {
      size_t right = child + 1;
      if (right < sz && this->heap[right].data[2] < this->heap[child].data[2]) {
        ++child;
      }

      if (this->heap[child].data[2] >= key)
        break;

      this->heap[i] = this->heap[child];
      i = child;
      child = (i << 1) + 1;
    }

    this->heap[i] = val;
  }

  size_t size() const noexcept {
    return this->sz;
  }

  void after_care() noexcept {
    for(size_t i = this->sz; i < this->max_size; ++i) {
      this->heap[i].data[2] = 0;
    }
  }

  void clear() noexcept {
    this->sz = 0;
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
    const uint32_t start_slot
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

    /* if(!checkProbrmGpu(next_field, fsize)){ */
    /*   printf("[gpu %d] broken field rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
    /* } */
    /* else{ */
    /*   printf("[gpu %d] rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
    /* } */

    // 評価
    scores[cf] = evaluation1(next_field, fsize) + 1;

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
void beam_search_kernel_depth1_after(uint32_t fsize, uint32_t field_size, uint32_t *tasks_gpu,uint32_t idx, uint16_t *df, uint16_t *next_df) {
  const uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if(tid < idx) {
    uint32_t *task = tasks_gpu + (tid << 1);
    uint16_t *field = df + task[0] * field_size;
    uint16_t *next_field = next_df + tid * field_size;

    uint32_t X, Y, N;
    uint32_t fields4 = field_size >> 2;
    uint32_t rot[2];
    getParams(task[1], fsize, &X, &Y, &N);
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
    /*   printf("rid=%d, fid=%d, evaluation=%d\n", (uint32_t)(task >> 32), (uint32_t)(task & 0xffffffff), evaluation1(next_field, fsize) + 1); */
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
    tail_counter& counter
  ) {
  const uint32_t tasksPerProcess = blocksPerGrid * threadsPerBlock;
  /* printf("[run_gpu] start: blocksPerGrid=%d, threadsPerBlock=%d, fsize=%d, ridsPerField=%d, field_len=%d, field_size=%d, tasksPerProcess=%d\n", blocksPerGrid, threadsPerBlock, fsize, ridsPerField, field_len, field_size, tasksPerProcess); */
  int32_t flen = field_len;
  uint32_t endf = 0, start_rid = 0, start_slot = 0, start_field = 0;
  while(true) {
    counter.addTask(tasksPerProcess);
    /* printf("[run_gpu] start gpu: start_rid=%d, start_field=%d, start_slot=%d\n", start_rid, start_field, start_slot); */
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, ridsPerField, df, field_len, field_size, dq, start_rid, start_field, start_slot);

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
void beamSearch(TasksQueue& queue, tail_counter& counter) {
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
        queue.push({fid + i, rid, val[i]});
      }
    }else if(fid == 0xffffffff) {
      /* printf("[beamSearch] end %p, %d\n", &queue, (int)queue.size()); */
      queue.after_care();
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
  const uint32_t ridsPerField = tidPerField_list[fsize];

  if(BLOCKS_PER_GRID * THREADS_PER_BLOCK >= QUEUE_SIZE) {
    printf("ERROR: blocksPerGrid * threadsPerBlock >= QUEUE_SIZE\n");
    return std::vector<uint32_t>();
  }

  // メモリ確保
  /* printf("start memory\n"); */
  std::vector<std::vector<uint32_t>> resultOperations(BEAM_WIDTH, std::vector<uint32_t>(MAX_DEPTH, 0));
  std::vector<std::vector<uint32_t>> bresultOperations(BEAM_WIDTH, std::vector<uint32_t>(MAX_DEPTH, 0));
  Tasks *tasks = new Tasks[BEAM_WIDTH];
  uint32_t *tasks_cpu = new uint32_t[BEAM_WIDTH<<1];
  for(size_t i = 0; i < BEAM_WIDTH; ++i)  tasks[i].data[2] = 0;
  uint32_t field_len = 1;

  std::vector<TasksQueue> threadQueues;
  threadQueues.reserve(CPU_THREAD_NUM);
  for(size_t i = 0, siz = BEAM_WIDTH / CPU_THREAD_NUM; i < CPU_THREAD_NUM; ++i) {
    threadQueues.emplace_back(tasks + siz * i, siz);
  }

  ResultQueue *hq, *dq;
  uint16_t *df, *next_df;
  uint32_t *tasks_gpu;
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
  err = cudaMalloc(&tasks_gpu, BEAM_WIDTH * sizeof(uint32_t) * 2);
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
  counter.ridsPerField = ridsPerField;
  counter.clear();
  counter.q = hq;
  counter.field_len = field_len;



  // 実行
  err = cudaMemcpy(df, start_field, field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    cudaFree(df);
    return std::vector<uint32_t>();
  }

  while(true) {
    /* printf("start loop depth=%d\n", depth); */
    // thread生成
    std::thread gthread([&](){run_gpu(beam_search_kernel_depth1, BLOCKS_PER_GRID, THREADS_PER_BLOCK, fsize, ridsPerField, df, field_len, field_size, dq, counter);});
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], counter);});
    }

    // thread終了待ち
    gthread.join();
    field_len = 0;
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads[i].join();
      field_len += threadQueues[i].size();
    }

    // 次のfield生成
    std::sort(tasks, tasks + BEAM_WIDTH, [](const Tasks& a, const Tasks& b) { return a.data[2] > b.data[2]; });

    
    
    if(tasks[0].data[2] >= field_size) {
      // 終了処理
      resultOperations[0] = bresultOperations[tasks[0].data[0]];
      resultOperations[0][depth] = tasks[0].data[1];
      ++depth;
      break;
    }

    for(size_t i = 0; i < field_len; ++i) {
      uint32_t *d = tasks[i].data;
      resultOperations[i] = bresultOperations[d[0]];
      resultOperations[i][depth] = d[1];
      size_t j = i<<1;
      tasks_cpu[j++] = d[0];
      tasks_cpu[j]   = d[1];

      /* if(i < 5){ */
      /*   printf("idx=%d, score=%d, rid=%d, fid=%d\n", i, t.data[2], t.data[1], t.data[0]); */
      /* } */
    }



    /* printf("start next process next field_len=%d\n", field_len); */
    err = cudaMemcpy(tasks_gpu, tasks_cpu, field_len * sizeof(uint32_t) * 2, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
      std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
      cudaFree(tasks_gpu);
      return std::vector<uint32_t>();
    }

    uint32_t bp = (field_len / THREADS_PER_BLOCK) + 1;
    beam_search_kernel_depth1_after<<<bp, THREADS_PER_BLOCK>>>(fsize, field_size, tasks_gpu, field_len, df, next_df);


    // 諸々初期化
    depth += 1;
    threads.clear();
    counter.clear();
    counter.field_len = field_len;
    cudaDeviceSynchronize();
    std::swap(df, next_df);
    std::swap(resultOperations, bresultOperations);
    /* printf("\n"); */
  }

  /* printf("end search operation num=%d\n",depth); */
  /* printField(fsize, start_field); */
  /* for(uint32_t i = 0, X, Y, N; i < depth; ++i){ */
  /*   getParams(resultOperations[0][i], fsize, &X, &Y, &N); */
  /*   std::cout << X << " " << Y << " " << N << std::endl; */
  /*   /1* printf("%d %d %d %d\n", X, Y, N, resultOperations[0][i]); *1/ */
  /* } */

  cudaFree(tasks_gpu);
  cudaFree(df);
  cudaFree(next_df);
  cudaFreeHost(hq);
  resultOperations[0].resize(depth);
  return resultOperations[0];

}

};
