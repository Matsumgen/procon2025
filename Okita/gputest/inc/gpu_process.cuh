#ifndef GPU_PROCESS_CUH_
#define GPU_PROCESS_CUH_

#include <stdint.h>
#include<iostream>

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
  /* printf("(a1, a2, b1, b2, ra, rb) = (%x %x %x %x %x %x)\n", a1, a2, b1, b2, ra, rb); */
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
  /* printf("(%d, %d), %x, %x, %x\n", (int)p[0], (int)p[1], a1, b, a2); */
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
  /* printf("(%d, %d), %x, %x, %x\n", (int)p[0], (int)p[1], a1, b, a2); */
}
__device__ __forceinline__
void culcDp4a(uint32_t *matrix, uint32_t *p) {
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
__device__ __forceinline__
void culcDp4a(uint32_t *matrix, uint32_t x, uint32_t y, uint32_t *p) {
  int32_t b  = (x << 8) | y;
  int32_t a1 = matrix[0] >> 16;
  int32_t a2 = matrix[1] >> 8;
  /* printf("start: %x %x\n", matrix[0], matrix[1]); */
  /* printf("(%d, %d), %x, %x, %x\n", x, y, a1, b, a2); */
  p[0] = (uint8_t)__dp4a(a1, b, a2);

  a1 = matrix[0] &  0xffff;
  a2 = matrix[1] &  0xff;
  p[1] = (uint8_t)__dp4a(a1, b, a2);
  /* printf("(%d, %d), %x, %x, %x\n", (int)p[0], (int)p[1], a1, b, a2); */
}

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

__device__ __forceinline__
void initialRotField(uint32_t *rotField, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n, uint32_t *rot) {
  uint32_t lim = (y * fsize + x) << 1;
  uint32_t i, j;
  for(i = 0; i < lim; i += 2) {
    rotField[i] = 0x01000001;
    rotField[i+1] = 0;
  }
  for(uint32_t k = 0; k < n; ++k){
    j = i;
    while(i < j + (n << 1)){
      rotField[i++] = rot[0];
      rotField[i++] = rot[1];
    }
    while(i < j + (fsize << 1)) {
      rotField[i++] = 0x01000001;
      rotField[i++] = 0;
    }
  }
  for(; i < fsize * fsize * 2; i += 2) {
    rotField[i] = 0x01000001;
    rotField[i+1] = 0;
  }
  /* printf("initial RotField %d, %d, %d, %d, %x %x\n", fsize, x, y, n, rot[0], rot[1]); */
  /* for(i = 0; i < fsize * fsize * 2; i+=2) { */
  /*   printf("%x %x\n", rotField[i], rotField[i+1]); */
  /* } */
}

__device__ __forceinline__
void initialRotField(uint32_t *rotField, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n) {
  uint32_t rot[2];
  createMatrixArrayL(x, y, n, rot);
  initialRotField(rotField, fsize, x, y, n, rot);
}



__device__ __forceinline__
void createRotFieldD2(uint32_t *rotField, const uint32_t fsize, const uint32_t x1, const uint32_t y1, const uint32_t n1, const uint32_t x2, const uint32_t y2, const uint32_t n2) {
  uint32_t rot1[2], rot2[2];
  createMatrixArrayL(x1, y1, n1, rot1);
  createMatrixArrayL(x2, y2, n2, rot2);

  initialRotField(rotField, fsize, x1, y1, n1, rot1);
  for(uint32_t y = y2, i; y < y2 + n2; ++y) {
    i = (y * fsize + x2) << 1;
    for(uint32_t x = 0; x < n2; ++x) {
      rotField[i++] = rot2[0];
      rotField[i++] = rot2[1];
    }
  }

  uint32_t ps1[2], ps2[2];
  ps1[0] = x1 > x2 ? x1 : x2;                     // left
  ps1[1] = y1 + n1-1 < y2 + n2-1 ? y1 + n1-1 : y2 + n2-1; // down
  ps2[0] = x1 + n1-1 < x2 + n2-1 ? x1 + n1-1 : x2 + n2-1; // right
  ps2[1] = y1 > y2 ? y1 : y2;                     // up
  if(ps1[0] <= ps2[0] && ps2[1] <= ps1[1]) {
    //重なりあり
    uint32_t inv[2];
    toInverse(rot2, inv);
    /* printf("rt: (%d, %d, %d) %x %x -> %x %x\n", x2, y2, n2, rot2[0], rot2[1], inv[0], inv[1]); */
    /* printf("st: ps1 = (%d, %d), ps2 = (%d, %d), rot1=%x %x, rot2=%x %x\n", ps1[0], ps1[1], ps2[0], ps2[1], rot1[0], rot1[1], rot2[0], rot2[1]); */
    culcDp4a(inv, ps1);
    culcDp4a(inv, ps2);
    multDp4a(rot1, rot2);
    /* printf("ed: ps1 = (%d, %d), ps2 = (%d, %d), rot12=%x %x\n", ps1[0], ps1[1], ps2[0], ps2[1], rot2[0], rot2[1]); */
    for(uint32_t y = ps1[1], i; y <= ps2[1]; ++y) {
      i = (y * fsize + ps1[0]) << 1;
      for(uint32_t x = ps1[0]; x <= ps2[0]; ++x) {
        rotField[i++] = rot1[0];
        rotField[i++] = rot1[1];
      }
    }
  }

  /* printf("initial RotField %d, %d, %d, %d, %d, %d, %d\n", fsize, x1, y1, n1, x2, y2, n2); */
  /* for(uint32_t i = 0; i < fsize * fsize * 2; i+=2) { */
  /*   printf("%x %x\n", rotField[i], rotField[i+1]); */
  /* } */
}

__device__
void rotateFieldGpu(uint16_t *field, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n) {
  uint32_t n_half = n >> 1;

  uint32_t h, w, buf;
  uint32_t i0 = x + y * fsize;
  uint32_t i1 = x + n - 1 + y * fsize;
  uint32_t i2 = x + n - 1 + (y + n - 1) * fsize;
  uint32_t i3 = x + (y + n - 1) * fsize;
  for(h = 0; h < n_half; ++h) {
    for(w = 0; w < n_half; ++w){
      buf = field[i0];
      field[i0] = field[i3];
      field[i3] = field[i2];
      field[i2] = field[i1];
      field[i1] = buf;

      ++i0;
      i1 += fsize;
      --i2;
      i3 -= fsize;
    }
    i0 = i0 - n_half + fsize;
    i1 = i1 - n_half * fsize - 1;
    i2 = i2 + n_half - fsize;
    i3 = i3 + n_half * fsize + 1;
  }

  if(n & 1){
    i0 = x + n_half + y * fsize;
    i1 = x + n - 1 + (y + n_half) * fsize;
    i2 = x + n_half + (y + n - 1) * fsize;
    i3 = x + (y + n_half) * fsize;
    for(h = 0; h < n_half; ++h) {
      buf = field[i0];
      field[i0] = field[i3];
      field[i3] = field[i2];
      field[i2] = field[i1];
      field[i1] = buf;
      i0 += fsize;
      --i1;
      i2 -= fsize;
      ++i3;
    }
  }
}

#endif
