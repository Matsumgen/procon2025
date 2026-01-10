#include <algo.cuh>
#include <matrix.cuh>
#include <matrixField.cuh>
#include <iostream>
#include <cuda_runtime.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <atomic>

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

const uint32_t tid_max_list[] = { 0, 0, 0, 0,13, 0, 54, 0, 139, 0, 284, 0, 505, 0, 818, 0, 1239, 0, 1784, 0, 2469, 0, 3310, 0, 4323 };

// queueのサイズ
// cpu thread数,gpu block数 はこの値を超えないものとする
# define QUEUE_SIZE 1024

// blockの結果(1blockが実行するタスクの数 * result_size)
# define BLOCK_SIZE 256 * 576

// queue_sizeのうち、この数までcpuで未処理でもgpuカーネル再起動する
# define BUFFER_LEAVE 8

// cpuの待機時間
# define SLEEP_TIME 50

/*
ResultQueue1
概要:     gpu処理の結果をcpuへ渡すための構造体。Queue構造
fburrer:  回転後のfield
tail:     cpu threadsがthreadで固有ののslot(bufferのindex)を探すため
next_slot:  gpu blockが(以下省略)
done:     0->未処理, 1->gpu処理終了
*/
struct ResultQueue1 {
  uint16_t fbuffer[QUEUE_SIZE][BLOCK_SIZE];

  uint32_t tail;
  uint32_t next_slot;
  uint8_t done[QUEUE_SIZE];
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
__global__
void beam_search_kernel_depth1(const uint32_t fsize, const uint32_t tid_max, const uint32_t result_len, const uint16_t* field, ResultQueue1 *q) {
  __shared__ uint32_t next_slot;
  __shared__ uint32_t slot;
  if(threadIdx.x == 0){
    next_slot = atomicAdd(&q->next_slot, 1) % QUEUE_SIZE;
    slot = next_slot % QUEUE_SIZE;
    if(q->done[slot]){
      slot = 0xffffffff;
      atomicSub(&q->next_slot, 1);
    }else{
      q->done[slot] = 1;
    }
  }
  __syncthreads();
  if(slot == 0xffffffff) return;

  uint32_t tid = next_slot * blockDim.x + threadIdx.x;
  /* printf("tid=%d, block=%d\n", tid, blockIdx.x); */
  if(tid >= tid_max) return; 


  /* printf("tid=%d, slot=%d\n", tid, slot); */

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
      q->fbuffer[slot][j + threadIdx.x * result_len] = field[i];
    }
  }

  __syncthreads();
  __threadfence_system();
}

template<typename Func>
void run_gpu(
    Func gpu_process,
    const int blocksPerGrid,
    const int threadsPerBlock,
    const uint32_t fsize,
    const uint32_t tid_max,
    const uint32_t result_len,
    uint16_t *df,
    ResultQueue1 *dq,
    ResultQueue1 *hq,
    std::atomic<uint32_t> *tail
  ) {
  while(true){
    std::cout << "start gpu_process" << std::endl;
    gpu_process<<<blocksPerGrid, threadsPerBlock>>>(fsize, tid_max, result_len, df, dq);
    cudaDeviceSynchronize();
    if(hq->next_slot * threadsPerBlock >= tid_max) break;

    while(hq->next_slot - tail->load(std::memory_order_relaxed) > BUFFER_LEAVE) {
      std::cout << "leave=" << hq->next_slot - tail->load(std::memory_order_relaxed) << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
  }
}


void test_beam_search() {
  std::cout << "start: test_beam_search()" << std::endl;
  const uint32_t fsize = 24;
  const uint32_t tid_max = tid_max_list[fsize];
  const uint32_t result_len = fsize * fsize;
  const int blocksPerGrid = 512;
  const int threadsPerBlock = 256;
  std::vector<uint16_t> f = makeShuffledPairs(fsize);

  std::cout << "fsize=" << fsize << ", tid_max=" << tid_max << ", result_len=" << result_len << std::endl;

  // メモリ確保
  std::cout << "start: memory" << std::endl;
  uint16_t *df;
  cudaError_t err = cudaMalloc(&df, fsize * fsize * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return ;
  }

  err = cudaMemcpy(df, f.data(), fsize * fsize * sizeof(uint16_t), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) {
    std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
    cudaFree(df);
    return ;
  }

  ResultQueue1 *hq, *dq;
  cudaHostAlloc(&hq, sizeof(ResultQueue1), cudaHostAllocMapped);
  cudaHostGetDevicePointer(&dq, hq, 0);
  hq->next_slot = 0;
  hq->tail = 0;
  for(size_t i = 0; i < QUEUE_SIZE; ++i) hq->done[i] = 0;

  std::atomic<uint32_t> *tail = reinterpret_cast<std::atomic<uint32_t>*>(&(hq->tail));
  
  // gpuカーネル起動
  std::cout << "start: run_gpu" << std::endl;
  std::thread t1([&](){run_gpu(beam_search_kernel_depth1, blocksPerGrid, threadsPerBlock, fsize, tid_max, result_len, df, dq, hq, tail);});

  // 結果受け取り
  // マルチスレッド化可能(のはず)
  uint32_t current = 0;
  uint32_t tid = 0;
  while(tid < tid_max) {
    tid = tail->fetch_add(1, std::memory_order_relaxed);
    current = tid % QUEUE_SIZE;
    tid *= threadsPerBlock;
    /* std::cout << "current=" << current << " done=" << (int)hq->done[current] << std::endl; */
    if(hq->done[current]){
      for(size_t fi = 0; fi < threadsPerBlock && tid < tid_max; ++fi){
        uint16_t *field = hq->fbuffer[current] + fi * result_len;

        // 結果を処理
        if(!checkProblem(fsize, field)){
          uint32_t X, Y, N;
          getParams(tid, fsize, &X, &Y, &N);
          std::cout << "tid=" << tid << ", (X, Y, N) = (" << X << ", " << Y << ", " << N << ")" << std::endl;
          /* printField(fsize, field, f.data()); */
        }
        ++tid;
      }
      hq->done[current] = 0;
    }else{
      tail->fetch_sub(1, std::memory_order_relaxed);
      std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
  }
  std::cout << "start: memory free" << std::endl;
  t1.join();
  cudaFree(df);
  cudaFree(dq);
}

