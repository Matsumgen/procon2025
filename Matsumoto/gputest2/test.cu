
/* GPUが使えるかの確認 */
/* #include <cstdio> */
/* #include <cuda_runtime.h> */

/* __global__ */
/* void testprocess() { */
/*     int idx = threadIdx.x + blockIdx.x * blockDim.x; */
/*     printf("Thread %d\n", idx); */
/* } */

/* int main() { */
/*     std::printf("Launching kernel...\n"); */
/*     testprocess<<<1, 2>>>(); */
/*     cudaError_t err = cudaDeviceSynchronize(); // 必須 */
/*     if (err != cudaSuccess) { */
/*         std::printf("CUDA error: %s\n", cudaGetErrorString(err)); */
/*     } */
/*     std::printf("Kernel done.\n"); */
/*     return 0; */
/* } */

/* #include <cuda_runtime.h> */
/* #include <stdio.h> */

/* int main() { */
/*     int deviceCount; */
/*     cudaGetDeviceCount(&deviceCount); */

/*     for (int i = 0; i < deviceCount; ++i) { */
/*         cudaDeviceProp prop; */
/*         cudaGetDeviceProperties(&prop, i); */
/*         printf("Device %d: %s\n", i, prop.name); */
/*         printf("  Compute Capability: %d.%d\n", prop.major, prop.minor); */
/*     } */
/*     return 0; */
/* } */

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>

#define N 4  // dp4a は 4 要素単位で計算する

__global__ void test_dp4a_kernel(const int8_t *A, const int8_t *B, int32_t *C)
{
#if __CUDA_ARCH__ >= 610
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= 1) return;  // テスト用に1スレッドだけ

    // dp4a intrinsic: 4つの int8 をまとめて積和
    // 初期和は 0
    int sum = 0;
    sum = __dp4a(*(int32_t*)&A[0], *(int32_t*)&B[0], sum);
    C[0] = sum;
#else
    // dp4a 非対応の GPU の場合
    C[0] = -1;
#endif
}

void main1() {
    // 入力データ（4要素ずつ）
    int8_t h_A[N] = {1, 2, 3, 4};
    int8_t h_B[N] = {5, 6, 7, 8};
    int32_t h_C[1];

    int8_t *d_A, *d_B;
    int32_t *d_C;

    cudaMalloc(&d_A, N * sizeof(int8_t));
    cudaMalloc(&d_B, N * sizeof(int8_t));
    cudaMalloc(&d_C, sizeof(int32_t));

    cudaMemcpy(d_A, h_A, N * sizeof(int8_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, N * sizeof(int8_t), cudaMemcpyHostToDevice);

    // カーネル起動（1スレッドでテスト）
    test_dp4a_kernel<<<1, 1>>>(d_A, d_B, d_C);
    cudaDeviceSynchronize();

    cudaMemcpy(h_C, d_C, sizeof(int32_t), cudaMemcpyDeviceToHost);

    printf("dp4a result: %d\n", h_C[0]);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);


}

void main2() {
  int device;
  cudaGetDevice(&device);
  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, device);
  std::cout << "Device name: " << prop.name << std::endl;
  std::cout << "SM count: " << prop.multiProcessorCount << std::endl;
  std::cout << "Compute Capability: " << prop.major << "." << prop.minor << std::endl;
}

int main()
{
  main2();
  return 0;
}

