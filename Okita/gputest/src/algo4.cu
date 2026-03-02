#define REVIEW_NUM 100

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
#include <fstream>
#include <array>


namespace bs4{
#include <gpu_process.cuh>

const uint32_t EVALUATION = 1;
const uint32_t SN = 5;
const uint32_t EN = 10;
const uint32_t SLICE = 1;
const double   THRESHOLD = 9.85;

void getParamMode(uint32_t depth, uint32_t fsize, uint32_t *paramMode) {
  uint32_t a = (uint32_t)((fsize * (fsize >> 1)) * THRESHOLD) & 0xfffe;
  if((depth<<1) == a) {
    *paramMode = 1;
  }
}
void getParamMode(uint32_t depth, uint32_t fsize, uint32_t *paramMode, uint32_t *ridsPerField, uint32_t *mr) {
  uint32_t a = (uint32_t)((fsize * (fsize >> 1)) * THRESHOLD) & 0xfffe;
  if((depth<<1) == a) {
    *paramMode = 1;
    uint32_t r = getTidPerField(fsize, 2, fsize, 1);
    *ridsPerField = r;
    *mr = r * (r + 1);
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
    val += (1<<6) / (r * r);
    /* int8_t *p1 = pos + i; */
    /* int8_t *p2 = p1 + 2; */
    /* if(r == 1){ */
    /*   val += 1 << 6; */
    /* }else{ */
    /*   val += canMove(p1, p2, fsize) + canMove(p2, p1, fsize); */
    /* } */
  }
  /* printf("value: %d\n", val); */
  return val;
}

__device__  __forceinline__
uint32_t evaluation1(uint16_t *f, uint32_t fsize) {
  uint32_t val = 0, x, y, i;
  uint32_t fs = fsize - 1;
  uint32_t sfs = fsize * fsize - 1;
  for(y = 0, i = 0; y < fs; ++y) {
    for(x = 0; x < fs; ++x) {
      val += (uint32_t)(f[i] == f[i+1]) + (uint32_t)(f[i] == f[i+fsize]);
      ++i;
    }
    ++i;
  }
  for(i = fs * fsize; i < sfs; ++i) {
    val += (uint32_t)(f[i] == f[i+1]);
  }
  for(i = fs; i < sfs; i += fsize) {
    val += (uint32_t)(f[i] == f[i+fsize]);
  }
  return (val << 6);
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
    const uint32_t start_slot,
    const uint32_t paramMode
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
  gparam(prid, fsize, &X1, &Y1, &N1, paramMode);
  createMatrixArrayL(X1, Y1, N1, rot1);

  prid = rid / ridsPerField;
  if(prid != ridsPerField) {
    gparam(prid, fsize, &X2, &Y2, &N2, paramMode);
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
void beam_search_kernel_depth2_after(uint32_t fsize, uint32_t field_size, uint32_t ridsPerField, uint64_t *tasks_gpu,uint32_t idx, uint16_t *df, uint16_t *next_df, uint32_t paramMode) {
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
    gparam(prid, fsize, &X1, &Y1, &N1, paramMode);
    createMatrixArrayL(X1, Y1, N1, rot1);

    // afterはdepth2確定
    prid = rid / ridsPerField;
    gparam(prid, fsize, &X2, &Y2, &N2, paramMode);
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
    tail_counter& counter,
    const uint32_t paramMode
  ) {
  const uint32_t tasksPerProcess = blocksPerGrid * threadsPerBlock;
  const uint32_t maxRid = counter.maxRid;
  /* printf("[run_gpu] start: blocksPerGrid=%d, threadsPerBlock=%d, fsize=%d, ridsPerField=%d, field_len=%d, field_size=%d, tasksPerProcess=%d\n", blocksPerGrid, threadsPerBlock, fsize, ridsPerField, field_len, field_size, tasksPerProcess); */
  int32_t flen = field_len;
  uint32_t endf = 0, start_rid = 0, start_slot = 0, start_field = 0;
  while(true) {
    counter.addTask(tasksPerProcess);
    /* printf("[run_gpu] start gpu: start_rid=%d, start_field=%d, start_slot=%d\n", start_rid, start_field, start_slot); */
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, ridsPerField, df, field_len, field_size, dq, start_rid, start_field, start_slot, paramMode);

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
        val[i] = 0;
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
  std::array<std::array<uint16_t, 500>, REVIEW_NUM> depths{};
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
  counter.maxRid = ridsPerField * (1 + ridsPerField);

  err = cudaMemcpy(df, start_field, field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    cudaFree(df);
    return std::vector<uint32_t>();
  }

  while(true) {
    /* printf("start loop depth=%d\n", depth); */
    // thread生成
    std::thread gthread([&](){run_gpu(beam_search_kernel_depth2, BLOCKS_PER_GRID, THREADS_PER_BLOCK, fsize, ridsPerField, df, field_len, field_size, dq, counter, paramMode);});
    for(size_t i = 0; i < CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], BEAM_WIDTH, counter, ridsPerField, field_size << 5);});
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
    /* printf("start sort depth: %3d, queue: pq.size=%d, pmode=%d, max:score=%d/%d, rid=%d, fid=%d\n", depth, (int)pq.size(), paramMode, (int)cur.t.score >> 6, field_size >> 1, cur.t.rid, cur.t.fid); */
#ifdef REVIEW_NUM
    if(depth < 250) depths[loop][depth] = cur.t.score >> 6;
#endif
    if(cur.t.score >= (field_size << 5)) {
      // 終了処理
      /* printf("end beamsearch: score=%d, rid=%d, fid=%d\n", (int)cur.t.score, cur.t.rid, cur.t.fid); */
      resultOperations[0] = bresultOperations[cur.t.fid];
      resultOperations[0][depth] = cur.t.rid;
      depth += 1;
      break;
    }else if(depth >= 250) {
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
    beam_search_kernel_depth2_after<<<bp, THREADS_PER_BLOCK>>>(fsize, field_size, ridsPerField, tasks_gpu, field_len, df, next_df, paramMode);


    // 諸々初期化
    depth += 1;
    threads.clear();
    counter.clear();
    counter.field_len = field_len;
    getParamMode(depth, fsize, &paramMode, &ridsPerField, &counter.maxRid);
    cudaDeviceSynchronize();
    std::swap(df, next_df);
    std::swap(resultOperations, bresultOperations);
    /* printf("\n"); */
  }

  bool f = false;
  paramMode = 0;
  ridsPerField = getTidPerField(fsize, SN, EN, SLICE);
  for(uint32_t i = 0, b; i < depth; ++i) {
    resultOperations[1][2*i] = resultOperations[0][i] % ridsPerField;
    b = resultOperations[0][i] / ridsPerField;
    f = b == ridsPerField;
    if(f) break; 
    getParamMode(i, fsize, &paramMode, &ridsPerField, &counter.maxRid);
    resultOperations[1][2*i+1] = b;
  }
  depth = depth * 2;
  if(f) --depth;



#ifdef REVIEW_NUM
    if(depth < 500) {
      auto end_time = std::chrono::high_resolution_clock::now();
      double t = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count()/1000.0;

      std::vector<uint16_t> field = sf;
      paramMode = 0;
      for(uint32_t i = 0, x, y, n; i < depth; ++i) {
        gparamcpu(resultOperations[1][i], fsize, &x, &y, &n, paramMode);
        if(i%2 == 0) getParamMode(i/2, fsize, &paramMode);
        rotateField(field, fsize, x, y, n);
      }
      if(!isEnd(field, fsize)){
        printf("ERROR: Don't end\n");
        field = sf;
        paramMode = 0;
        printField(fsize, field.data());
        for(uint32_t i = 0; i < depth; ++i) {
          uint32_t x, y, n;
          gparamcpu(resultOperations[1][i], fsize, &x, &y, &n, paramMode);
          rotateField(field, fsize, x, y, n);
          printf("rotate: rid=%d, (%d, %d, %d)\n", resultOperations[1][i], x, y, n);
          if(i%2==0)getParamMode(i/2, fsize, &paramMode);
          printField(fsize, field.data());
        }
        printf("depth: %d\n", depth);
      }else{
        times.push_back(t);
        operations.push_back(depth);
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

  std::ofstream ofs("output.txt", std::ios::app);
  ofs << "config:" << std::endl;
  ofs << "\tfsize:\t\t\t"         << fsize << std::endl;
  ofs << "\tSN:\t\t\t"            << SN << std::endl;
  ofs << "\tEN:\t\t\t"            << EN << std::endl;
  ofs << "\tSLICE:\t\t\t"         << SLICE << std::endl;
  ofs << "\tTHRESHOLD:\t\t"       << THRESHOLD << std::endl;
  ofs << "\tEVALUATION:\t\t"      << EVALUATION << std::endl;
  ofs << "\tblocksPerGrid:\t\t"   << BLOCKS_PER_GRID << std::endl;
  ofs << "\tthreadsPerBlock:\t"   << THREADS_PER_BLOCK << std::endl;
  ofs << "\tBEAM_WIDTH:\t\t"      << BEAM_WIDTH << std::endl;
  ofs << "\tFIELDS_PER_THREAD:\t" << FIELDS_PER_THREAD << std::endl;
  ofs << "\tCPU_THREAD_NUM:\t\t"  << CPU_THREAD_NUM << std::endl;
  ofs << "\tQUEUE_SIZE:\t\t"      << QUEUE_SIZE << std::endl;
  ofs << "time:" << std::endl;
  ofs << "\tmax:\t"  << *time_max_it << "[sec]" << std::endl;
  ofs << "\tmin:\t"  << *time_min_it << "[sec]" << std::endl;
  ofs << "\tmean:\t" << time_mean    << "[sec]" << std::endl;
  ofs << "\tsd:\t"   << time_sd      << "[sec]" << std::endl;
  ofs << "operations:" << std::endl;
  ofs << "\tmax:\t"  << *ope_max_it << std::endl;
  ofs << "\tmin:\t"  << *ope_min_it << std::endl;
  ofs << "\tmean:\t" << ope_mean    << std::endl;
  ofs << "\tsd:\t"   << ope_sd      << std::endl;
  ofs << "\tmean time per operation:\t" << time_mean / ope_mean<< std::endl;
  ofs << "\tmean operation per pair:\t" << ope_mean / (field_size >> 1) << std::endl;
  ofs << "\tDon't end:\t\t" << dontend << std::endl;
  ofs << std::endl;

  std::ofstream ofs2("output_analisis.txt", std::ios::app);
  for(size_t i = 0; i < REVIEW_NUM; ++i) {
    for(size_t j = 0; j < 500; ++j) ofs2 << depths[i][j] << ",";
    ofs2 << std::endl;
  }
  
#endif


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
