#ifndef GPU_PROCESS_CUH_
#define GPU_PROCESS_CUH_

#include <stdint.h>

__device__ __host__ __forceinline__
void createMatrixArrayL(const uint8_t x, const uint8_t y, const uint8_t n, uint32_t *a) {
  a[0] = 0x0001ff00;
  uint8_t t = x - y;
  a[1] = t << 8 | (x + y + n - 1);
}

__device__ __host__ __forceinline__
void createMatrixArrayR(const uint8_t x, const uint8_t y, const uint8_t n, uint32_t *a) {
  a[0] = 0x00ff0100;
  uint8_t t = y - x;
  a[1] = (x + y + n - 1) << 8 | t;
}


// a * bの配列演算の結果をaに入れる
__device__ __forceinline__
void multDp4a(uint32_t *a, const uint32_t *b) {
  int32_t a1 = a[0] >>  16;
  int32_t a2 = a[0] &  0xffff;
  int32_t ax = a[1] >> 8;
  int32_t ay = a[1] &  0xff;

  int32_t b1 = b[0] >> 16;
  int32_t b2 = b[0] &  0xffff;

  int32_t ra = __dp4a(a1, b1, 0);
  int32_t rb = __dp4a(a1, b2, 0);
  b1 = __dp4a(a2, b1, 0); // = rc
  b2 = __dp4a(a2, b2, 0); // = rb

  a[0] = (uint8_t)ra | ((uint8_t)rb << 8) | ((uint8_t)b1 << 16) | ((uint8_t)b2 << 24);

  ra = __dp4a(a1, (int32_t)b[1], ax);
  rb = __dp4a(a2, (int32_t)b[1], ay);

  a[1] = (uint8_t)rb | ((uint8_t)ra << 8);
}

// a * bの配列演算の結果をcに入れる
__device__ __forceinline__
void multDp4a(const uint32_t *a, const uint32_t *b, uint32_t *c) {
  int32_t a1 = a[0] >>  16;
  int32_t a2 = a[0] &  0xffff;
  int32_t ax = a[1] >> 8;
  int32_t ay = a[1] &  0xff;

  int32_t b1 = b[0] >> 16;
  int32_t b2 = b[0] &  0xffff;

  int32_t ra = __dp4a(a1, b1, 0);
  int32_t rb = __dp4a(a1, b2, 0);
  b1 = __dp4a(a2, b1, 0); // = rc
  b2 = __dp4a(a2, b2, 0); // = rb

  c[0] = (uint8_t)ra | ((uint8_t)rb << 8) | ((uint8_t)b1 << 16) | ((uint8_t)b2 << 24);

  ra = __dp4a(a1, (int32_t)b[1], ax);
  rb = __dp4a(a2, (int32_t)b[1], ay);

  c[1] = (uint8_t)rb | ((uint8_t)ra << 8);
}

// p = {x, y} を移動
__device__ __forceinline__
void culcDp4a(uint32_t *matrix, uint8_t *p) {
  int32_t b  = (p[0] << 8) | p[1];
  int32_t a1 = matrix[0] >> 16;
  int32_t a2 = matrix[1] >> 8;
  /* printf("(%d, %d), %x, %x, %x\n", (int)p[0], (int)p[1], a1, b, a2); */
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
  /* printf("(%d, %d), %x, %x, %x\n", (int)p[0], (int)p[1], a1, b, a2); */
}

// task_idから(X, Y, N)の組を取得
__device__ __host__ __forceinline__
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

__device__ __forceinline__
bool checkProbrmGpu(const uint16_t *f, const uint32_t fsize) {
  uint32_t nums[24*12];
  for(uint32_t i = 0; i < fsize * fsize / 2; ++i) nums[i] = 0;

  for(uint32_t y = 0; y < fsize; ++y) {
    for(uint32_t x = 0; x < fsize; ++x) {
      nums[f[y*fsize+x]] += 1;
    }
  }
  for(uint32_t i = 0; i < fsize * fsize / 2; ++i) {
    if(nums[i] != 2){
      return false;
    }
  }
  return true;
}

#endif
