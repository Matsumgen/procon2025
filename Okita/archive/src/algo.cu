#include <algo.cuh>
#include <matrix.cuh>
#include <matrixField.cuh>
#include <iostream>
#include <cuda_runtime.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <mutex>

/*
Device name: NVIDIA GeForce RTX 4070 Laptop GPU
SM count: 36
Compute Capability: 8.9

//global memory
#define BEAM_WIDTH 0
#define INPUT_SIZE BEAM_WIDTH*288


fsize:  tid max     result size(uint32_t)
4:      13          8
6:      54          18
8:      139         32
10:     284         50
12:     505         72
14:     818         98
16:     1239        128
18:     1784        162
20;     2469        200
22:     3310        242
24:     4323        288

*/

const uint32_t tidPerField_list[] = { 0, 0, 0, 0,13, 0, 54, 0, 139, 0, 284, 0, 505, 0, 818, 0, 1239, 0, 1784, 0, 2469, 0, 3310, 0, 4323 };

// queueのサイズ
// cpu thread数,gpu block数 はこの値を超えないものとする
# define QUEUE_SIZE 1024

// blockの結果(1blockが実行するタスクの数 * result_size)
# define BLOCK_SIZE 256 * 576

// queue_sizeのうち、この数までcpuで未処理でもgpuカーネル再起動する
# define BUFFER_LEAVE 8

// cpuの待機時間
# define SLEEP_TIME 50

// operationsの長さの最大値。ここまでにそろわなかったらアウト
# define MAX_DEPTH 512
/*
ResultQueue1
概要:     gpu処理の結果をcpuへ渡すための構造体。Queue構造
fburrer:  回転後のfield
tail:     cpu threadsがthreadで固有ののslot(bufferのindex)を探すため
head:     gpu        がthreadで固有ののslot(bufferのindex)を探すため & fieldsのindexを探すため
done:     0->未処理, 1->gpu処理終了
*/
struct ResultQueue1 {
  uint16_t fbuffer[QUEUE_SIZE][BLOCK_SIZE];

  uint32_t tail;
  uint32_t head;
  uint8_t done[QUEUE_SIZE];
};


struct tail_counter {
  uint32_t *tail;
  uint32_t f_count;
  uint32_t f_index;
  uint32_t triger;
  uint8_t *done;
  std::mutex mtx;

  uint64_t get() {
    std::lock_guard lock(this->mtx);
    uint64_t current = *(this->tail) % QUEUE_SIZE;
    if(!this->done[current]){
      return 0xffffffffffffffff;
    }


    *(this->tail) = current + 1;
    uint64_t fidx = this->f_index;
    f_count += 1;
    if(this->triger == this->f_count){
      this->f_index += 1;
      this->f_count = 0;
      /* std::cout << "update f_index=" << this->f_index << ", tail=" << *(this->tail) << std::endl; */
    }

    return (current << 32) | (fidx & 0xffffffff);
  }

  void get_tail(uint32_t *t, uint8_t *b) {
    std::lock_guard lock(this->mtx);
    *t = *(this->tail) % QUEUE_SIZE;
    *b = done[*t];
  }
  uint32_t get_f_index() {
    std::lock_guard lock(this->mtx);
    return this->f_index;

  }

  void reset() {
    this->f_count = 0;
    this->f_index = 0;
  }
};


// task_idから(X, Y, N)の組を取得
__device__ __host__
void getParams(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N) {
  uint32_t acc = 0;
  uint32_t w = fsize - 1;
  while(true) {
    uint32_t size = w * w;
    if(tid < acc + size){
      break;
    }
    acc += size;
    --w;
  }
  acc = tid - acc;
  *X = acc % w;
  *Y = acc / w;
  *N = fsize - w + 1;
}

__global__
void beam_search_kernel_depth3(const uint32_t fsize, const uint32_t input_length, const uint32_t depth, const uint16_t* field, uint16_t* output) {

}
__global__
void beam_search_kernel_depth2(const uint32_t fsize, const uint32_t input_length, const uint32_t depth, const uint16_t* field, uint16_t* output) {
  /* uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x; */
  /* uint32_t tidy = blockIdx.y * blockDim.y + threadIdx.y; */

}

// 1thread 1task
// 1blockで複数タスクという形のほうが効率的になりそう
// fieldをまとめて実行できるようにする
// blocks * threadsがuint32_tを超えないようにする
// block数がQUEUE_SIZEを超えないようにする
// queueの空きスロットがblocksの数以上にする
__global__
void beam_search_kernel_depth1(const uint32_t fsize, const uint32_t tidPerField, const uint32_t slotPerField, const uint32_t field_size, const uint32_t field_len, const uint16_t *fields, ResultQueue1 *q, const uint32_t start_slot, const uint32_t start_field) {
  __shared__ uint32_t head;
  __shared__ uint32_t slot;
  __shared__ uint32_t current_field;
  if(threadIdx.x == 0){
    head = atomicAdd(&q->head, 1) % QUEUE_SIZE;
    slot = (start_slot + head) % QUEUE_SIZE;
    current_field = head / slotPerField;
    if(current_field >= field_len){
      slot = 0xffffffff;
      atomicSub(&q->head, 1);
    }
  }
  __syncthreads();
  if(slot == 0xffffffff) return; 

  uint32_t tid = (head % slotPerField) * blockDim.x + threadIdx.x;
  if(tid >= tidPerField){
    /* printf("b field_len=%d, tid=%d, head=%d, slot=%d, threadIdx.x=%d, blockIdx.x=%d, current_field=%d\n", field_len, tid, head, slot, threadIdx.x, blockIdx.x, current_field); */
    return;
  }

  const uint16_t *field = fields + (start_field + current_field) * field_size;
  tid = tid % tidPerField;


  /* printf("c field_len=%d, tid=%d, head=%d, slot=%d, threadIdx.x=%d, blockIdx.x=%d, current_field=%d\n", field_len, tid, head, slot, threadIdx.x, blockIdx.x, current_field); */

 // 24 * 24 * 2
  uint32_t data[1152];
  for (uint32_t i = 0; i < fsize * fsize * 2; i += 2) {
    data[i] = 0x01000001;
    data[i+1] = 0;
  }

  uint32_t X, Y, N, i, j;
  getParams(tid, fsize, &X, &Y, &N);
  uint32_t rot[2];
  createMatrixArrayL(X, Y, N, rot);
  /* printf("tid=%d, (%d, %d, %d), rot=%x %x\n", tid, X, Y, N, rot[0], rot[1]); */
  for(uint32_t y = Y; y < Y + N; ++y) {
    for(uint32_t x = X; x < X + N; ++x) {
      i = y * fsize + x;
      multDp4a(data + i * 2, rot);
    }
  }

  uint8_t p[2];
  for(uint32_t y = 0, i; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      p[0] = x;
      p[1] = y;
      i = y * fsize + x;
      culcDp4a(data + i * 2, p);
      j = p[1] * fsize + p[0];
      q->fbuffer[slot][i + threadIdx.x * field_size] = field[j];
    }
  }

  __syncthreads();
  if(threadIdx.x == 0){
    q->done[slot] = 1;
  }
  __threadfence_system();
}

// ペア数 * 2を返す
__device__ __host__
uint32_t evaluation1(uint16_t *f, uint32_t fsize) {
  uint32_t val = 0, x, y, i;
  val += (uint32_t)(f[0] == f[1]) + (uint32_t)(f[0] == f[fsize])
        +(uint32_t)(f[fsize - 1] == f[fsize - 2]) + (uint32_t)(f[fsize - 1] == f[2 * fsize - 1])
        +(uint32_t)(f[(fsize - 1) * fsize] == f[(fsize - 2) * fsize]) + (uint32_t)(f[(fsize - 1) * fsize] == f[(fsize - 1) * fsize + 1])
        +(uint32_t)(f[fsize * fsize - 1] == f[fsize * fsize - 2]) + (uint32_t)(f[fsize * fsize - 1] == f[(fsize - 1) * fsize - 1]);
  for(x = 1; x < fsize - 1; ++x) {
    val += (uint32_t)(f[x] == f[x - 1]) + (uint32_t)(f[x] == f[x + 1]) +(uint32_t)(f[x] == f[fsize + x])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 1) * fsize + x - 1])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 1) * fsize + x + 1])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 2) * fsize + x]);
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


template<typename Func>
void run_gpu(
    Func gpu_process,
    const uint32_t blocksPerGrid,
    const uint32_t threadsPerBlock,
    const uint32_t fsize,
    const uint32_t tidPerField,
    const uint32_t slotPerField,
    const uint32_t field_size,
    const uint32_t field_len,
    const uint16_t *df,
    ResultQueue1 *dq,
    ResultQueue1 *hq,
    tail_counter& tail
  ) {
  /* std::cout << "run_gpu: "; */
  /* std::cout << "blocksPerGrid=" << blocksPerGrid; */
  /* std::cout << ", threadsPerBlock=" << threadsPerBlock; */
  /* std::cout << ", fsize=" << fsize; */
  /* std::cout << ", tidPerField=" << tidPerField; */
  /* std::cout << ", slotPerField=" << slotPerField; */
  /* std::cout << ", field_size=" << field_size; */
  /* std::cout << ", field_len=" << field_len << std::endl; */
  int32_t flen = field_len;
  uint32_t endf = 0;
  uint32_t start_slot = 0;
  uint32_t start_field = 0;
  // tid が uint32_t に収まるようにdfとfield_lenを弄る
  while(true){
    // field_len * field_size
    /* std::cout << "start gpu_process: flen=" << flen << ", start_slot=" << start_slot << ", start_field=" << start_field << std::endl; */
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, tidPerField, slotPerField, field_size, flen, df, dq, start_slot, start_field);
    cudaDeviceSynchronize();

    endf = hq->head / slotPerField;
    flen -= endf;
    /* std::cout << "head=" << hq->head << ", endf=" << endf << ", flen=" << flen << std::endl; */
    if(flen <= 0) break;
    start_field += endf;
    start_slot = (start_slot + hq->head) % QUEUE_SIZE;
    hq->head = hq->head % slotPerField;

    while(true){
      uint8_t done;
      uint32_t tail_current;
      tail.get_tail(&tail_current, &done);
      uint32_t free_queue;
      if(tail_current < start_slot) {
        free_queue = QUEUE_SIZE - (start_slot - tail_current);
      }else if(tail_current > start_slot){
        free_queue = tail_current - start_slot;
      }else if(done) {
        free_queue = 0;
      }else{
        free_queue = QUEUE_SIZE;
      }

      if(free_queue <= blocksPerGrid){
        /* std::cout << "sleep gpu:free_queue=" << free_queue << ", tail_current=" << tail_current << ", start_slot=" << start_slot << std::endl; */
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
      }else{
        break;
      }
    }
  }
  std::cout << "end run_gpu" << std::endl;
}

// slotPerField < 32bitでないとOFする
void test_beam_search() {
  std::cout << "start: test_beam_search()" << std::endl;
  const uint32_t beam_width = 4096;
  const uint32_t fsize = 10;
  const uint32_t field_size = fsize * fsize;
  const uint32_t blocksPerGrid = 512;
  const uint32_t threadsPerBlock = 256;
  const uint32_t tidPerField = tidPerField_list[fsize];
  const uint32_t slotPerField = (tidPerField / threadsPerBlock) + (tidPerField%threadsPerBlock == 0 ? 0 : 1);
  std::vector<uint16_t> start_field = makeShuffledPairs(fsize);
  std::vector<std::vector<uint16_t>> f(beam_width, std::vector<uint16_t>(field_size));
  f[0] = start_field;
  printField(fsize, f[0].data());
  uint32_t field_len = 1;

  // operations[i][0]: value
  std::vector<std::vector<uint32_t>> operations(beam_width, std::vector<uint32_t>(MAX_DEPTH, 0));
  std::vector<std::vector<uint32_t>> operations_buffer(beam_width, std::vector<uint32_t>(MAX_DEPTH, 0));

  std::cout << "fsize=" << fsize << ", tidPerField=" << tidPerField << ", field_size=" << field_size << std::endl;

  // メモリ確保
  std::cout << "start: memory" << std::endl;
  uint16_t *df;
  cudaError_t err = cudaMalloc(&df, beam_width * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return ;
  }

  err = cudaMemcpy(df, f[0].data(), field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    cudaFree(df);
    return ;
  }

  ResultQueue1 *hq, *dq;
  cudaHostAlloc(&hq, sizeof(ResultQueue1), cudaHostAllocMapped);
  cudaHostGetDevicePointer(&dq, hq, 0);
  hq->head = 0;
  hq->tail = 0;
  for(size_t i = 0; i < QUEUE_SIZE; ++i) hq->done[i] = 0;


  // 結果受け取り
  // マルチスレッド化可能(のはず)
  tail_counter counter; 
  counter.tail = &(hq->tail);
  counter.triger = slotPerField;
  counter.done = hq->done;
  counter.reset();
  uint32_t current = 0;
  uint32_t f_index = 0;
  uint32_t tid = 0;
  uint32_t ope_num = 1;
  uint32_t f_tail = 0;
  bool error_end = false;
  while(operations_buffer[0][0] < fsize * fsize + 1 && !error_end){
    std::cout << "depth=" << ope_num - 1 << std::endl;
    if(ope_num >= MAX_DEPTH) {
      std::cout << "false field beam search" << std::endl;
      break;
    }
    // gpuカーネル起動
    std::thread t1([&](){run_gpu(beam_search_kernel_depth1, blocksPerGrid, threadsPerBlock, fsize, tidPerField, slotPerField, field_size, field_len, df, dq, hq, counter);});

    while(counter.get_f_index() < field_len) {
      /* for(uint32_t i = 0; i < 12; ++i) std::cout << "(" << (int)hq->done[i] << ", " << (int)counter.done[i] << "), "; */
      /* std::cout << std::endl; */
      uint64_t bbb = counter.get();
      /* std::cout << "bbb=" << bbb << std::endl; */
      if(bbb == 0xffffffffffffffff){
        /* std::cout << "sleep cpu" << std::endl; */
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
      }else{
        current = bbb >> 32;
        f_index = bbb & 0xffffffff;

        /* std::cout <<"current=" << current << " f_index=" << f_index << std::endl; */

        for(size_t fi = 0; fi < threadsPerBlock; ++fi){
          uint16_t *field = hq->fbuffer[current] + fi * field_size;

          // 結果を処理
          if(!checkProblem(fsize, field)){
            std::cout << "f field" << std::endl;
            error_end = true;
            uint32_t X, Y, N;
            getParams(tid, fsize, &X, &Y, &N);
            std::cout << "tid=" << tid << ", f_index=" << f_index << ", (X, Y, N) = (" << X << ", " << Y << ", " << N << ")" << std::endl;
            printField(fsize, field, start_field.data());
            /* continue; */
            ++tid;
            if(tid >= tidPerField){ tid = 0; break; }
          }

          // 二分探索 -> 挿入
          uint32_t value = evaluation1(field, fsize) + 1;
          int32_t index = beam_width / 2;
          int32_t min_i = 0;
          int32_t max_i = beam_width - 1;
          while(min_i < max_i) {
            /* std::cout << "index=" << index << ", min_i=" << min_i << ", max_i=" << max_i << std::endl; */
            if(operations[index][0] < value) {
              max_i = index - 1;
            }else {
              // operations[index][0] >= value : tid小が有利
              min_i = index + 1;
            }
            index = (min_i + max_i) / 2;
          }
          /* std::cout << "index=" << index << ", f_index=" << f_index << ", value=" << value << ", o_v=" << operations[index][0] << std::endl; */
          for(int32_t i = std::min(f_tail+1, beam_width - 1); index < i; --i) {
            std::swap(f[i-1], f[i]);
            std::swap(operations[i-1], operations[i]);
          }
          /* std::cout << "copy" << std::endl; */
          operations[index] = operations_buffer[f_index];
          operations[index][0] = value;
          operations[index][ope_num] = tid;
          std::copy(field, field + field_size, f[index].begin());


          if(f_tail < beam_width) ++f_tail;
          ++tid;
          if(tid >= tidPerField){
            tid = 0;
            break;
          }
        }
        hq->done[current] = 0;
      }
    }

    // operations -> operations_buffer
    std::swap(operations, operations_buffer);
    for(uint32_t i = 0; i < operations.size(); ++i) {
      operations[i][0] = 0;
    }

    ++ope_num;
    t1.join();
    hq->head = 0;
    hq->tail = 0;
    field_len = f_tail;
    f_tail = 0;
    counter.reset();
    /* std::cout << "result" << std::endl; */
    for(uint32_t i = 0; i < field_len; ++i){
      /* if(!checkProblem(fsize, f[i].data())){ */
      /*   std::cout << "false field" << std::endl; */
      /*   for(uint32_t j = 1, X, Y, N; j < ope_num; ++j){ */
      /*     getParams(operations_buffer[i][j], fsize, &X, &Y, &N); */
      /*     std::cout << "(" << X << ", " << Y << ", " << N << ")->"; */
      /*   } */
      /*   std::cout << std::endl; */
      /*   printField(fsize, f[i].data()); */
      /* } */
      /* printField(fsize, f[i].data()); */
      err = cudaMemcpy(df + i * field_size, f[i].data(), field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
      if (err != cudaSuccess) {
        std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
        cudaFree(df);
        return ;
      }
    }
    std::cout << "result value=" << operations_buffer[0][0] << std::endl;
  }
  std::cout << "end result value =  " << operations_buffer[0][0] << std::endl;
  std::cout << "end operation num = " << ope_num - 1 << std::endl;
  printField(fsize, f[0].data());
  printField(fsize, start_field.data());
  for(uint32_t i = 1, X, Y, N; i < ope_num; ++i){
    getParams(operations_buffer[0][i], fsize, &X, &Y, &N);
    std::cout << X << " " << Y << " " << N << std::endl;
  }
  std::cout << std::endl;
  std::cout << "start: memory free" << std::endl;
  cudaFree(df);
  cudaFree(dq);
}

