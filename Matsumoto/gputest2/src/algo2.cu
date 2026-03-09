  // X1, X2らから、範囲を確定(X, Y, N)
  // rot_fieldの(X1, Y1, N1)をrot1に置き換え
  // 単位行列で初期化
  // rot_fieldの(X2, Y2, N2)について、
  //  重なっている部分は左回転した位置にrot2を掛ける
  //  重なってなければ置き換え

  // rot[0]を配列で事前保持 0-> {0001, 0100}, 1 -> ...

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

namespace bs2{
#include <gpu_process.cuh>

struct Tasks {
  uint32_t fid;
  uint32_t rid;
  uint32_t score;
  bool isDepth1;

  uint64_t getTask() const {
    return (((uint64_t)rid) << 32) | fid;
  }
  uint32_t key() const {
    return (score << 1) | static_cast<uint32_t>(isDepth1);
  }
};

// min-heap
struct TasksCompare {
  bool operator()(const Tasks& a, const Tasks&b) const {
    return a.key() > b.key();
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
    std::sort(this->c.rbegin(), this->c.rend(), [](const Tasks& a, const Tasks& b) { return a.key() < b.key(); });
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
    return a.t.key() < b.t.key();
  }
};

struct ResultQueue {
  uint32_t scores[QUEUE_SIZE * FIELDS_PER_THREAD / 2]; // scoreは16bit, 二つまとめて書き込む
  uint8_t done[QUEUE_SIZE];
};

struct tail_counter {
  ResultQueue *q;
  uint64_t tail;
  uint32_t maxRid;
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
      if(this->field_len <= ((uint32_t)(t / this->maxRid)) * FIELDS_PER_THREAD) *fid = 0xffffffff;
      return false;
    }
    *fid = ((uint32_t)(t / this->maxRid)) * FIELDS_PER_THREAD;
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
    *rid = t % this->maxRid;
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
void beam_search_kernel_depth2(
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
  const uint32_t maxRid = ridsPerField * (ridsPerField + 1);
  uint32_t rid = start_rid + gpu_id;
  const uint32_t current_field = start_field + ((uint32_t)(rid / maxRid)) * FIELDS_PER_THREAD;
  if(current_field >= field_len) return; 
  rid %= maxRid;

  uint32_t exec_field_len = field_len - current_field;
  if(exec_field_len > FIELDS_PER_THREAD) exec_field_len = FIELDS_PER_THREAD;

  /* printf("[gpu %d] rid=%d, current_field=%d, exec_field_len=%d\n", gpu_id, rid, current_field, exec_field_len); */

  uint16_t *field = fields + current_field * field_size;

  uint32_t X1, Y1, N1, X2, Y2, N2;
  uint32_t rot1[2], rot2[2], rot12[2], ps1[2], ps2[2], p[2];
  uint32_t fields4 = field_size >> 2;
  uint16_t next_field[MAX_FSIZE * MAX_FSIZE];
  uint32_t scores[FIELDS_PER_THREAD];
  uint32_t cf;

  uint32_t prid = rid % ridsPerField;
  getParams(prid, fsize, &X1, &Y1, &N1);
  createMatrixArrayL(X1, Y1, N1, rot1);

  prid = rid / ridsPerField;
  if(prid != ridsPerField) {
    getParams(prid, fsize, &X2, &Y2, &N2);
    createMatrixArrayL(X2, Y2, N2, rot2);
    multDp4a(rot1, rot2, rot12);
    /* printf("[gpu %d]%x %x, %x %x -> %x %x\n", gpu_id, rot1[0], rot1[1], rot2[0], rot2[1], rot12[0], rot12[1]); */

    ps1[0] = X1 > X2 ? X1 : X2;                             // left
    ps1[1] = Y1 + N1-1 < Y2 + N2-1 ? Y1 + N1-1 : Y2 + N2-1; // dowN
    ps2[0] = X1 + N1-1 < X2 + N2-1 ? X1 + N1-1 : X2 + N2-1; // right
    ps2[1] = Y1 > Y2 ? Y1 : Y2;                             // up
    bool kasanari = ps1[0] <= ps2[0] && ps2[1] <= ps1[1];
    toInverse(rot2, p);
    culcDp4a(p, ps1);
    culcDp4a(p, ps2);

    for(cf = 0; cf < exec_field_len; ++cf){
      ushort4 *nf = (ushort4*)next_field;
      ushort4 *f = (ushort4*)field;
      for(uint32_t i = 0; i < fields4; ++i) { nf[i] = f[i]; }

      for(uint32_t y = Y1, i, j; y < Y1+N1; ++y) {
        i = y * fsize + X1;
        for(uint32_t x = X1; x < X1+N1; ++x) {
          culcDp4a(rot1, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = field[j];
        }
      }
      for(uint32_t y = Y2, i, j; y < Y2+N2; ++y) {
        i = y * fsize + X2;
        for(uint32_t x = X2; x < X2+N2; ++x) {
          culcDp4a(rot2, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = field[j];
        }
      }
      if(kasanari){
        for(uint32_t y = ps1[1], i, j; y <= ps2[1]; ++y) {
          i = y * fsize + ps1[0];
          for(uint32_t x = ps1[0]; x <= ps2[0]; ++x) {
            culcDp4a(rot12, x, y, p);
            j = p[1] * fsize + p[0];
            next_field[i++] = field[j];
          }
        }
      }


      /* if(!checkProbrmGpu(next_field, fsize)){ */
      /*   printf("[gpu %d] broken field rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
        /* printf("(%d, %d, %d), (%d, %d, %d)\n", X1, Y1, N1, X2, Y2, N2); */
        /* for(uint32_t y = 0; y < fsize; ++y) { */
        /*   for(uint32_t x = 0; x < fsize; ++x) { */
        /*     printf("%3d\t", next_field[y*fsize+x]); */
        /*   } */
        /*   printf("\n"); */
        /* } */
        /* printf("\n"); */
      /* } */
      /* else{ */
      /*   printf("[gpu %d] rid=%d, fid=%d, before_broken=%d\n", gpu_id, rid, current_field + cf, checkProbrmGpu(field, fsize)); */
      /* } */

      scores[cf] = evaluation1(next_field, fsize) + 1;
      field += field_size;
    }
  }else{
    for(cf = 0; cf < exec_field_len; ++cf){
      ushort4 *nf = (ushort4*)next_field;
      ushort4 *f = (ushort4*)field;
      for(uint32_t i = 0; i < fields4; ++i) { nf[i] = f[i]; }

      for(uint32_t y = Y1, i, j; y < Y1+N1; ++y) {
        i = y * fsize + X1;
        for(uint32_t x = X1; x < X1+N1; ++x) {
          culcDp4a(rot1, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = field[j];
        }
      }

      scores[cf] = evaluation1(next_field, fsize) + 1;
      field += field_size;
    }
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
void beam_search_kernel_depth2_after(uint32_t fsize, uint32_t field_size, uint32_t ridsPerField, uint64_t *tasks_gpu,uint32_t idx, uint16_t *df, uint16_t *next_df) {
  const uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if(tid < idx) {
    uint64_t task = tasks_gpu[tid];
    uint16_t *field = df + ((uint32_t)(task & 0xffffffff)) * field_size;
    uint16_t *next_field = next_df + tid * field_size;

    uint32_t X1, Y1, N1, X2, Y2, N2;
    uint32_t fields4 = field_size >> 2;
    uint32_t rot1[2], rot2[2], rot12[2], ps1[2], ps2[2], p[2];
    uint32_t rid = task >> 32;
    uint32_t prid = rid % ridsPerField;
    getParams(prid, fsize, &X1, &Y1, &N1);
    createMatrixArrayL(X1, Y1, N1, rot1);

    // afterはdepth2確定
    prid = rid / ridsPerField;
    getParams(prid, fsize, &X2, &Y2, &N2);
    createMatrixArrayL(X2, Y2, N2, rot2);
    multDp4a(rot1, rot2, rot12);

    ps1[0] = X1 > X2 ? X1 : X2;                             // left
    ps1[1] = Y1 + N1-1 < Y2 + N2-1 ? Y1 + N1-1 : Y2 + N2-1; // dowN
    ps2[0] = X1 + N1-1 < X2 + N2-1 ? X1 + N1-1 : X2 + N2-1; // right
    ps2[1] = Y1 > Y2 ? Y1 : Y2;                             // up

    ushort4 *nf = (ushort4*)next_field;
    ushort4 *f = (ushort4*)field;
    for(uint32_t i = 0; i < fields4; ++i) { nf[i] = f[i]; }

    for(uint32_t y = Y1, i, j; y < Y1+N1; ++y) {
      i = y * fsize + X1;
      for(uint32_t x = X1; x < X1+N1; ++x) {
        culcDp4a(rot1, x, y, p);
        j = p[1] * fsize + p[0];
        next_field[i++] = field[j];
      }
    }
    for(uint32_t y = Y2, i, j; y < Y2+N2; ++y) {
      i = y * fsize + X2;
      for(uint32_t x = X2; x < X2+N2; ++x) {
        culcDp4a(rot2, x, y, p);
        j = p[1] * fsize + p[0];
        next_field[i++] = field[j];
      }
    }

    if(ps1[0] <= ps2[0] && ps2[1] <= ps1[1]){
      toInverse(rot2, p);
      culcDp4a(p, ps1);
      culcDp4a(p, ps2);
      for(uint32_t y = ps1[1], i, j; y <= ps2[1]; ++y) {
        i = y * fsize + ps1[0];
        for(uint32_t x = ps1[0]; x <= ps2[0]; ++x) {
          culcDp4a(rot12, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = field[j];
        }
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
  const uint32_t maxRid = counter.maxRid;
  /* printf("[run_gpu] start: blocksPerGrid=%d, threadsPerBlock=%d, fsize=%d, ridsPerField=%d, field_len=%d, field_size=%d, tasksPerProcess=%d\n", blocksPerGrid, threadsPerBlock, fsize, ridsPerField, field_len, field_size, tasksPerProcess); */
  int32_t flen = field_len;
  uint32_t endf = 0, start_rid = 0, start_slot = 0, start_field = 0;
  while(true) {
    counter.addTask(tasksPerProcess);
    /* printf("[run_gpu] start gpu: start_rid=%d, start_field=%d, start_slot=%d\n", start_rid, start_field, start_slot); */
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, ridsPerField, df, field_len, field_size, dq, start_rid, start_field, start_slot);

    cudaDeviceSynchronize();
    endf = (start_rid + tasksPerProcess) / maxRid;
    endf *= FIELDS_PER_THREAD;
    flen -= endf;
    if(flen <= 0) break;
    start_field += endf;
    start_slot = (start_slot + tasksPerProcess) % QUEUE_SIZE;
    start_rid = (start_rid + tasksPerProcess) % maxRid;

    // queueが空くまで待機
    /* printf("[run_gpu] wait\n"); */
    counter.wait(tasksPerProcess);
  }
  /* printf("[run_gpu] end gpu\n"); */
}

void beamSearch(TasksQueue& queue, uint32_t beam_width, tail_counter& counter, uint32_t ridsPerField, uint32_t max_score) {
  /* printf("[beamSearch] start: beam_width=%d, %p\n", beam_width, &queue); */
  uint16_t val[FIELDS_PER_THREAD];
  uint32_t rid = 0, fid = 0;
  queue.clear();
  while(true) {
    if(counter.get(val, &rid, &fid)) {
      /* printf("[beamSearch] rid=%d, fid=%d\n", rid, fid); */
      bool isDepth2 = ((uint32_t)(rid / ridsPerField)) != ridsPerField;

      for(uint32_t i = 0; i < FIELDS_PER_THREAD && val[i] != 0; ++i) {
        /* if(counter.field_len <= fid + i) { */
        /*   printf("[beamSearch] over field rid=%d, fid=%d, field_len=%d,val=%d\n", rid, fid + i, counter.field_len, (int)val[i]); */
        /* } */
        /* else{ */
        /*   printf("[beamSearch]  rid=%d, fid=%d, score=%d\n", rid, fid + i, val[i]); */
        /* } */
        if(isDepth2 || val[i] > max_score){
          /* if(!isDepth2 && val[i] > max_score)printf("True %d %d\n",rid, fid); */
          if(queue.size() < beam_width){
            queue.push({fid + i, rid, val[i], !isDepth2});
          }else{
            Tasks cur = queue.top();
            if(cur.score < val[i]) { // 先が優先
              queue.pop();
              queue.push({fid + i, rid, val[i], !isDepth2});
            }
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
  const uint32_t ridsPerField = tidPerField_list[fsize]; // 1回転の数

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
  counter.maxRid = ridsPerField * (1 + ridsPerField);
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
    std::thread gthread([&](){run_gpu(beam_search_kernel_depth2, BLOCKS_PER_GRID, THREADS_PER_BLOCK, fsize, ridsPerField, df, field_len, field_size, dq, counter);});
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], BEAM_WIDTH, counter, ridsPerField, fsize * fsize);});
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
    /* printf("start sort queue: pq.size=%d, max:score=%d, rid=%d, fid=%d\n", (int)pq.size(), (int)cur.t.score, cur.t.rid, cur.t.fid); */
    if(cur.t.score >= field_size) {
      // 終了処理
      /* printf("end beamsearch: score=%d, rid=%d, fid=%d\n", (int)cur.t.score, cur.t.rid, cur.t.fid); */
      resultOperations[0] = bresultOperations[cur.t.fid];
      resultOperations[0][depth] = cur.t.rid;
      depth += 1;
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
    beam_search_kernel_depth2_after<<<bp, THREADS_PER_BLOCK>>>(fsize, field_size, ridsPerField, tasks_gpu, field_len, df, next_df);


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

  bool f = false;
  for(uint32_t i = 0, b; i < depth; ++i) {
    resultOperations[1][2*i] = resultOperations[0][i] % ridsPerField;
    b = resultOperations[0][i] / ridsPerField;
    f = b == ridsPerField;
    if(f) break; 
    resultOperations[1][2*i+1] = b;
  }
  depth = depth * 2;
  if(f) --depth;

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
  resultOperations[1].resize(depth);
  return resultOperations[1];

}

};
