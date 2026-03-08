#ifndef ALGO2LIB_CUH_
#define ALGO2LIB_CUH_

#include <stdint.h>
#include <iostream>
#include <cmath>

namespace algo2lib {

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

  int32_t b1 = ((b[0] & 0xff000000u) >> 16) | ((b[0] & 0x0000ff00u) >> 8);
  int32_t b2 = ((b[0] & 0x00ff0000u) >> 8)  | (b[0] & 0x000000ffu);

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

  int32_t b1 = ((b[0] & 0xff000000u) >> 16) | ((b[0] & 0x0000ff00u) >> 8);
  int32_t b2 = ((b[0] & 0x00ff0000u) >> 8)  | (b[0] & 0x000000ffu);

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
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
}
__device__ __forceinline__
void culcDp4a(uint32_t *matrix, uint32_t *p) {
  int32_t b  = (p[0] << 8) | p[1];
  int32_t a1 = matrix[0] >> 16;
  int32_t a2 = matrix[1] >> 8;
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
}
__device__ __forceinline__
void culcDp4a(uint32_t *matrix, uint32_t x, uint32_t y, uint32_t *p) {
  int32_t b  = (x << 8) | y;
  int32_t a1 = matrix[0] >> 16;
  int32_t a2 = matrix[1] >> 8;
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
}

// matrixの逆行列をresultに入れる
__device__ __forceinline__
void toInverse(const uint32_t *matrix, uint32_t *result) {
  uint32_t r = matrix[0];
  uint32_t rt = (r & 0xff0000ffu) | ((r & 0x00ff0000u) >> 8) | ((r & 0x0000ff00u) << 8);
  result[0] = rt;

  int32_t b = matrix[1];
  int32_t a1 = rt >> 16;
  int32_t a2 = rt & 0x0000ffffu;

  uint8_t b1 = (uint8_t)(-__dp4a(a1, b, 0));
  uint8_t b2 = (uint8_t)(-__dp4a(a2, b, 0));
  result[1] = (b1 << 8) | b2;

}

// task_idから(X, Y, N)の組を取得
__device__ __host__ __forceinline__
void getParam1(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t sn=2, uint32_t slice=1) {
  uint32_t acc = 0;
  int32_t w = fsize - sn + 1;
  for(; w >= 0; --w) {
    uint32_t size = w * w;
    size = (size / slice) + (size % slice == 0 ? 0 : 1);
    if(tid < acc + size) break; 
    acc += size;
  }

  *N = fsize - w + 1;
  
  acc = (tid - acc) * slice;
  *X = acc % w;
  *Y = acc / w;
}

// task_idから(X, Y, N)の組を取得
__device__ __host__ __forceinline__
void getParams(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t sn=2, uint32_t slice=1) {
  uint32_t acc = 0;
  int32_t w = fsize - sn + 1;
  for(; w >= 0; --w) {
    uint32_t size = w * w;
    size = (size / slice) + (size % slice == 0 ? 0 : 1);
    if(tid < acc + size) break; 
    acc += size;
  }

  *N = fsize - w + 1;
  
  acc = (tid - acc) * slice;
  *X = acc % w;
  *Y = acc / w;
}

// 評価関数
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
  }
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

// 評価関数3（超特大の隣接ボーナス＋距離の道標ハイブリッド）
__device__  __forceinline__
uint32_t evaluation3(uint16_t *f, uint32_t fsize) {
  int8_t pos[24*24*2];
  size_t i, j;
  size_t ms = fsize * fsize * 2;
  for(size_t k = 0; k < ms; k += 4) pos[k] = -1;

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
  for(size_t k = 0; k < ms; k += 4) {
    uint32_t r = abs(pos[k] - pos[k+2]) + abs(pos[k+1] - pos[k+3]);
    
    if (r == 1) {
      // 隣接している場合：圧倒的な特大ボーナスを与える (1<<10 = 1024)
      val += 1024; 
    } else if (r > 1) {
      // 離れている場合：近づくほど少しずつスコアが上がる道標 (最大でも64未満)
      val += (1 << 7) / (r * r); 
    }
  }
  return val;
}

// デバッグ用
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

}
#endif
