#include <algo2.cuh>
#include <algo2lib.cuh>
#include <algo2lib.hpp>
#include <vector>
#include <iostream>
#include <thread>
#include <queue>
#include <algorithm>

namespace algo2_1 {
  using namespace algo2lib;

Ope gparamcpu(const uint32_t tid, const uint32_t fsize, uint32_t paramMode) {
  uint32_t sn = 2, slice = 1;
  if(paramMode == 0){
    sn = MemObj2::SN;
    slice = MemObj2::SLICE;
  }
  return getParamsCpu(tid, fsize, sn, slice);
}

__device__ __forceinline__
void gparam(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t paramMode) {
  uint32_t sn = 2, slice = 1;
  if(paramMode == 0) {
    sn = MemObj2::SN;
    slice = MemObj2::SLICE;
  }
  getParams(tid, fsize, X, Y, N, sn, slice);
}

__device__ __forceinline__
uint32_t evaluation(uint16_t *f, uint32_t fsize, const uint32_t paramMode) {
  if(paramMode == 0) {
    return evaluation2(f, fsize);
  } else if (paramMode == 1){
  return evaluation1(f, fsize);
  }else {
    return evaluation3(f, fsize);
  }
}

__global__
void beam_search_kernel_depth1( const uint32_t fsize, const uint32_t ridsPerField, uint16_t *fields, const uint32_t field_len, const uint32_t field_size, MemObj2::ResultQueue    *q, const uint32_t start_rid, const uint32_t start_field, const uint32_t start_slot, const uint32_t paramMode) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;
  uint32_t rid = start_rid + gpu_id;
  const uint32_t current_field = start_field + ((uint32_t)(rid / ridsPerField)) * MemObj2::FIELDS_PER_THREAD;
  if(current_field >= field_len) return; 
  rid %= ridsPerField;

  uint32_t exec_field_len = field_len - current_field;
  if(exec_field_len > MemObj2::FIELDS_PER_THREAD) exec_field_len = MemObj2::FIELDS_PER_THREAD;

  uint16_t *field = fields + current_field * field_size;

  uint32_t X, Y, N;
  uint32_t fields4 = field_size >> 2;
  uint32_t rot[2];
  gparam(rid, fsize, &X, &Y, &N, paramMode);
  createMatrixArrayL(X, Y, N, rot);

  uint16_t next_field[576];
  uint32_t scores[MemObj2::FIELDS_PER_THREAD];
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

    // 評価
    scores[cf] = evaluation(next_field, fsize, paramMode) + 1;

    // 次の盤面
    field += field_size;
  }
  for(; cf < MemObj2::FIELDS_PER_THREAD; ++cf) { scores[cf] = 0; }

  // 書き込み
  const uint32_t slot = (start_slot + gpu_id) % MemObj2::QUEUE_SIZE;
  for(uint32_t i=0; i < (MemObj2::FIELDS_PER_THREAD >> 1); ++i){
    q->scores[slot * (MemObj2::FIELDS_PER_THREAD >> 1) + i] = (scores[2*i] << 16) | (scores[2*i+1] & 0xffff);
  }
  q->done[slot] = 1;
  __threadfence_system();
}

__global__
void beam_search_kernel_depth1_after(uint32_t fsize, uint32_t field_size, uint32_t ridsPerField, uint64_t *tasks_gpu, uint32_t idx, uint16_t *df, uint16_t *next_df, uint32_t paramMode) {
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
  }
}


__global__
void beam_search_kernel_depth2(const uint32_t fsize, const uint32_t ridsPerField, uint16_t *fields, const uint32_t field_len, const uint32_t field_size, MemObj2::ResultQueue    *q, const uint32_t start_rid, const uint32_t start_field, const uint32_t start_slot, const uint32_t paramMode) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;
  const uint32_t maxRid = ridsPerField * (ridsPerField + 1);
  uint32_t rid = start_rid + gpu_id;
  const uint32_t current_field = start_field + ((uint32_t)(rid / maxRid)) * MemObj2::FIELDS_PER_THREAD;
  if(current_field >= field_len) return; 
  rid %= maxRid;

  uint32_t exec_field_len = field_len - current_field;
  if(exec_field_len > MemObj2::FIELDS_PER_THREAD) exec_field_len = MemObj2::FIELDS_PER_THREAD;

  /* printf("[gpu %d] rid=%d, current_field=%d, exec_field_len=%d\n", gpu_id, rid, current_field, exec_field_len); */

  uint16_t *field = fields + current_field * field_size;

  uint32_t X1, Y1, N1, X2, Y2, N2;
  uint32_t rot1[2], rot2[2], rot12[2], ps1[2], ps2[2], p[2];
  uint32_t fields4 = field_size >> 2;
  uint16_t next_field[MemObj2::MAX_FSIZE * MemObj2::MAX_FSIZE];
  uint32_t scores[MemObj2::FIELDS_PER_THREAD];
  uint32_t cf;

  uint32_t prid = rid % ridsPerField;
  gparam(prid, fsize, &X1, &Y1, &N1, paramMode);
  createMatrixArrayL(X1, Y1, N1, rot1);

  prid = rid / ridsPerField;
  if(prid != ridsPerField) {
    gparam(prid, fsize, &X2, &Y2, &N2, paramMode);
    createMatrixArrayL(X2, Y2, N2, rot2);
    multDp4a(rot1, rot2, rot12);

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

      scores[cf] = evaluation(next_field, fsize, paramMode) + 1;
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

      scores[cf] = evaluation(next_field, fsize, paramMode) + 1;
      field += field_size;
    }
  }
  for(; cf < MemObj2::FIELDS_PER_THREAD; ++cf) { scores[cf] = 0; }

  // 書き込み
  const uint32_t slot = (start_slot + gpu_id) % MemObj2::QUEUE_SIZE;
  for(uint32_t i=0; i < (MemObj2::FIELDS_PER_THREAD >> 1); ++i){
    q->scores[slot * (MemObj2::FIELDS_PER_THREAD >> 1) + i] = (scores[2*i] << 16) | (scores[2*i+1] & 0xffff);
  }
  q->done[slot] = 1;
  __threadfence_system();
}


__global__
void beam_search_kernel_depth2_after(uint32_t fsize, uint32_t field_size, uint32_t ridsPerField, uint64_t *tasks_gpu, uint32_t idx, uint16_t *df, uint16_t *next_df, uint32_t paramMode) {
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
    MemObj2::ResultQueue *dq,
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
    endf *= MemObj2::FIELDS_PER_THREAD;
    flen -= endf;
    if(flen <= 0) break;
    start_field += endf;
    start_slot = (start_slot + tasksPerProcess) % MemObj2::QUEUE_SIZE;
    start_rid = (start_rid + tasksPerProcess) % maxRid;

    // queueが空くまで待機
    counter.wait(tasksPerProcess);
  }
  /* printf("[run_gpu] end gpu\n"); */
}

void beamSearch(TasksQueue& queue, uint32_t beam_width, tail_counter& counter, uint32_t ridsPerField, uint32_t max_score, bool depth2) {
  uint16_t val[MemObj2::FIELDS_PER_THREAD];
  uint32_t rid = 0, fid = 0;
  queue.clear();
  while(true) {
    if(counter.get(val, &rid, &fid)) {
      bool isDepth2 = !depth2 || ((uint32_t)(rid / ridsPerField)) != ridsPerField;
      for(uint32_t i = 0; i < MemObj2::FIELDS_PER_THREAD && val[i] != 0; ++i) {
        if(isDepth2 || val[i] > max_score){
          if(queue.size() < beam_width){
            queue.push(Tasks(fid + i, rid, val[i], !isDepth2));
          }else{
            Tasks cur = queue.top();
            if(cur.score() < val[i]) { // 先が優先
              queue.pop();
              queue.push(Tasks(fid + i, rid, val[i], !isDepth2));
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
      std::this_thread::sleep_for(std::chrono::milliseconds(MemObj2::SLEEP_TIME));
    }
  }
}


void push_back_resultOperations(std::vector<Ope>& resultOperations, const uint32_t& rid, const uint32_t fsize, const uint32_t& paramMode, const bool& depth2, const uint32_t& ridsPerField, std::pair<uint8_t, uint8_t>& offset) {
  if(depth2) {
    resultOperations.push_back(gparamcpu(rid % ridsPerField, fsize, paramMode));
    uint32_t b = rid / ridsPerField;
    if(b != ridsPerField){
      Ope ope = gparamcpu(b, fsize, paramMode);
      ope.data[0] += offset.first;
      ope.data[1] += offset.second;
      resultOperations.push_back(ope);
    }
  }else{
    Ope ope = gparamcpu(rid, fsize, paramMode);
    ope.data[0] += offset.first;
    ope.data[1] += offset.second;
    resultOperations.push_back(ope);
  }
}






std::vector<Ope> algo2_1(std::vector<RawField>& fields, std::vector<std::vector<Ope>>& opes, std::vector<std::pair<uint8_t, uint8_t>>& offsets, uint32_t fsize, MemObj2& mem2) {
  cudaFree(0);
  std::cout << "start algo2_1" << std::endl;
  std::vector<std::thread> threads;
  const uint32_t field_size = fsize * fsize;
  const uint32_t max_score = field_size << 5;
  uint32_t depth = 0;
  uint32_t field_len = fields.size();
  uint32_t ridsPerField = getTidPerField(fsize, MemObj2::SN, MemObj2::EN, MemObj2::SLICE);
  uint32_t beam_width = MemObj2::BEAM_WIDTH;

  uint32_t paramMode = 0;
  bool depth2 = false;
  auto beam_search_func = beam_search_kernel_depth1;
  auto beam_search_func_after = beam_search_kernel_depth1_after;

  tail_counter counter;
  counter.clear();
  counter.field_len = field_len;
  counter.maxRid = depth2 ? ridsPerField * (1 + ridsPerField) : ridsPerField;
  counter.q = mem2.hq;
  
  std::vector<TasksQueue>& threadQueues = mem2.threadQueues;
  std::vector<uint64_t>& tasks = mem2.tasks;
  std::vector<std::vector<Ope>>& resultOperations = mem2.resultOperations;
  std::vector<std::vector<Ope>>& bresultOperations = mem2.bresultOperations;
  std::vector<uint8_t>& bstart_idx = mem2.bstart_idx;
  for(size_t i = 0; i < opes.size() && MemObj2::BEAM_WIDTH; ++i){
    bresultOperations[i].assign(opes[i].begin(), opes[i].end());
    TasksQueue::start_idx[i] = i;
    TasksQueue::fieldsiz[i] = opes[i].size();
  }

  // debug
  /* for(size_t i = 0; i < fields.size(); ++i) { */
  /*   for(size_t j = 0; j < opes[i].size(); ++j) { */
  /*     printf("(%d %d %d) ", (int)opes[i][j].x(), (int)opes[i][j].y(), (int)opes[i][j].n()); */
  /*   } */
  /*   std::cout << std::endl; */
  /*   std::cout << fields[i].data() << " " << opes[i].data() << std::endl; */
  /*   printField(fields[i], fsize); */
  /* } */

  // fieldsをすべて書き込む
  std::vector<uint16_t> flat;
  flat.reserve(field_size * field_len);
  for(auto& v: fields) {
    flat.insert(flat.end(), v.begin(), v.end());
  }
  cudaError_t err = cudaMemcpy(mem2.df, flat.data(), flat.size() * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    return std::vector<Ope>{};
  }
  
  while(true) {

    if(depth >= MemObj2::THRESHOLD && paramMode == 0) {
      paramMode = 1;
      ridsPerField = getTidPerField(fsize, 2, fsize, 1);
      counter.maxRid = depth2 ? ridsPerField * (1 + ridsPerField) : ridsPerField;
      /* beam_search_func = beam_search_kernel_depth2; */
      /* beam_search_func_after = beam_search_kernel_depth2_after; */
      /* depth2 = true; */
    }

    /* printf("depth=%d, paramMode=%d\n", depth, paramMode); */

    std::thread gthread([&](){run_gpu(beam_search_func, MemObj2::BLOCKS_PER_GRID, MemObj2::THREADS_PER_BLOCK, fsize, ridsPerField, mem2.df, field_len, field_size, mem2.dq, counter, paramMode);});
    for(size_t i = 0; i < MemObj2::CPU_THREAD_NUM; ++i){
      threads.emplace_back([&, i](){beamSearch(threadQueues[i], beam_width, counter, ridsPerField, max_score, depth2);});
    }

    gthread.join();
    for(auto& t : threads){ t.join(); }



    std::priority_queue<Node, std::vector<Node>, NodeCompare> pq;
    for(size_t i = 0; i < MemObj2::CPU_THREAD_NUM; ++i) {
      if(!threadQueues[i].q_empty()) pq.push({threadQueues[i].get(), i});
    }

    Node cur = pq.top();
    printf("depth: %3d, queue: pq.size=%d, pmode=%d, max:score=%d/%d, rid=%d, fid=%d\n", depth, (int)pq.size(), paramMode, (int)cur.t.score() >> 6, field_size >> 1, cur.t.rid(), cur.t.fid());
    /* printf("start sort depth: %3d, queue: pq.size=%d, pmode=%d, max:score=%d/%d, rid=%d, fid=%d\n", depth, (int)pq.size(), paramMode, (int)cur.t.score() >> 6, field_size >> 1, cur.t.rid(), cur.t.fid()); */
    if(cur.t.score() >= max_score) {
      // 終了処理
      uint32_t fid = cur.t.fid();
      resultOperations[0] = bresultOperations[fid];
      push_back_resultOperations(resultOperations[0], cur.t.rid(), fsize, paramMode, depth2, ridsPerField, offsets[TasksQueue::start_idx[fid]]);
      break;
    }


    for(field_len = 0; field_len < beam_width && !pq.empty(); ++field_len) {
      cur = pq.top();
      pq.pop();
      tasks[field_len] = cur.t.getTask();
      uint32_t fid = cur.t.fid();
      resultOperations[field_len] = bresultOperations[fid];
      bstart_idx[field_len] = TasksQueue::start_idx[fid];
      push_back_resultOperations(resultOperations[field_len], cur.t.rid(), fsize, paramMode, depth2, ridsPerField, offsets[TasksQueue::start_idx[fid]]);
      if(!threadQueues[cur.thidx].q_empty()) {
        pq.push({threadQueues[cur.thidx].get(), cur.thidx});
      }
      /* if(depth == 1 && field_len < 40) { */
      /*   std::cout << field_len << "/" << beam_width << " " << pq.size() << " " << resultOperations.size() << " " << bresultOperations.size(); */
      /*   printf("idx=%d, score=%d, rid=%d, fid=%d\n", field_len, (int)cur.t.score(), cur.t.rid(), cur.t.fid()); */
      /* } */
    }

    err = cudaMemcpy(mem2.tasks_gpu, tasks.data(), field_len * sizeof(uint64_t), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
      std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
      return std::vector<Ope>{};
    }

    uint32_t bp = (field_len / MemObj2::THREADS_PER_BLOCK) + 1;
    beam_search_func_after<<<bp, MemObj2::THREADS_PER_BLOCK>>>(fsize, field_size, ridsPerField, mem2.tasks_gpu, field_len, mem2.df, mem2.next_df, paramMode);


    depth += depth2 ? 2 : 1;
    threads.clear();
    counter.clear();
    counter.field_len = field_len;
    cudaDeviceSynchronize();

    std::swap(resultOperations, bresultOperations);
    std::swap(bstart_idx, TasksQueue::start_idx);
    std::swap(mem2.df, mem2.next_df);

  }

  return resultOperations[0];
}



}
