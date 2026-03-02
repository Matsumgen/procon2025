#ifndef MATRIX_CUH_
#define MATRIX_CUH_

#include <stdint.h>
#include <iostream>

#define RotationMatrix RotationMatrixOptCul
// ヘッダ内に実装書かないと
/*
flag = aabbccdd
data = xxxxyyyy
a,b,c,d: -1, 0, 1 -> 0 ~ 2
x, yは24 * 3を超えない

以下の行列式を表す
a, b, x
c, d, y
0, 0, 1
*/
// メモリ量を限界まで削減したもの
struct RotationMatrixMinMem {
  uint8_t  flag;
  uint16_t data;

  // 反時計回り
  __host__ __device__
  static RotationMatrixMinMem left(uint8_t x, uint8_t y, uint8_t n) {
    int X = static_cast<int>(x) - static_cast<int>(y);
    int Y = static_cast<int>(x) + static_cast<int>(n) - 1;
    int o = X << 8 | Y;
    return RotationMatrixMinMem(static_cast<uint16_t>(o), 97);
  }
  
  // 時計回り
  __host__ __device__
  static RotationMatrixMinMem right(uint8_t x, uint8_t y, uint8_t n) {
    int X = static_cast<int>(x) + static_cast<int>(y) + static_cast<int>(n) - 1;
    int Y = static_cast<int>(y) - static_cast<int>(x);
    int o = X << 8 | Y;
    return RotationMatrixMinMem(static_cast<uint16_t>(o), 73);
  }
  
  __host__ __device__
  RotationMatrixMinMem() : flag(0), data(0) { }
  __host__ __device__
  RotationMatrixMinMem(uint16_t data, uint8_t flag) : data(data), flag(flag) { }
  
  __host__ __device__
  RotationMatrixMinMem operator*(const RotationMatrixMinMem& other) const {
    RotationMatrixMinMem ret;
    int a1 = static_cast<int>((this->flag & 0b11000000) >> 6) - 1;
    int b1 = static_cast<int>((this->flag & 0b00110000) >> 4) - 1;
    int c1 = static_cast<int>((this->flag & 0b00001100) >> 2) - 1;
    int d1 = static_cast<int>(this->flag & 0b00000011) - 1;
    int a2 = static_cast<int>((other.flag & 0b11000000) >> 6) - 1;
    int b2 = static_cast<int>((other.flag & 0b00110000) >> 4) - 1;
    int c2 = static_cast<int>((other.flag & 0b00001100) >> 2) - 1;
    int d2 = static_cast<int>(other.flag & 0b00000011) - 1;

    int x1 = static_cast<int>((this->data & 0xF0) >> 8);
    int y1 = static_cast<int>(this->data & 0x0F);
    int x2 = static_cast<int>((other.data & 0xF0) >> 8);
    int y2 = static_cast<int>(other.data & 0x0F);

    int out0 = ((a1*a2+b1*c2+1) << 6) | ((a1*b2+b1*d2+1) << 4) | ((c1*a2+d1*c2+1) << 2) | (c1*b2+d1*d2+1);
    int out1 = ((a1*x2+b1*y2+x1) << 4) | (c1*x2+d1*y2+y1);

    ret.flag = static_cast<uint8_t>(out0);
    ret.data = static_cast<uint16_t>(out1);
    return ret;
  }

  __host__ __device__
  void mult(const RotationMatrixMinMem& other) {
    int a1 = static_cast<int>((this->flag & 0b11000000) >> 6) - 1;
    int b1 = static_cast<int>((this->flag & 0b00110000) >> 4) - 1;
    int c1 = static_cast<int>((this->flag & 0b00001100) >> 2) - 1;
    int d1 = static_cast<int>(this->flag & 0b00000011) - 1;
    int a2 = static_cast<int>((other.flag & 0b11000000) >> 6) - 1;
    int b2 = static_cast<int>((other.flag & 0b00110000) >> 4) - 1;
    int c2 = static_cast<int>((other.flag & 0b00001100) >> 2) - 1;
    int d2 = static_cast<int>(other.flag & 0b00000011) - 1;

    int x1 = static_cast<int>((this->data & 0xF0) >> 8);
    int y1 = static_cast<int>(this->data & 0x0F);
    int x2 = static_cast<int>((other.data & 0xF0) >> 8);
    int y2 = static_cast<int>(other.data & 0x0F);

    int out0 = ((a1*a2+b1*c2+1) << 6) | ((a1*b2+b1*d2+1) << 4) | ((c1*a2+d1*c2+1) << 2) | (c1*b2+d1*d2+1);
    int out1 = ((a1*x2+b1*y2+x1) << 4) | (c1*x2+d1*y2+y1);

    this->flag = static_cast<uint8_t>(out0);
    this->data = static_cast<uint16_t>(out1);
  }

  __host__ __device__
  void culc(uint8_t x,uint8_t y, uint8_t *buf) const {
    int a1 = static_cast<int>((this->flag & 0b11000000) >> 6) - 1;
    int b1 = static_cast<int>((this->flag & 0b00110000) >> 4) - 1;
    int c1 = static_cast<int>((this->flag & 0b00001100) >> 2) - 1;
    int d1 = static_cast<int>((this->flag & 0b00000011)) - 1;
    int x1 = static_cast<int>((this->data & 0xF0) >> 8);
    int y1 = static_cast<int>(this->data & 0x0F);

    int out0 = a1 * static_cast<int>(x) + b1 * static_cast<int>(y) + x1;
    int out1 = c1 * static_cast<int>(x) + d1 * static_cast<int>(y) + y1;

    buf[0] = static_cast<uint8_t>(out0);
    buf[1] = static_cast<uint8_t>(out1);
  }

  __host__
  void print() const {
    int a = ((this->flag & 0b11000000) >> 6) - 1;
    int b = ((this->flag & 0b00110000) >> 4) - 1;
    int c = ((this->flag & 0b00001100) >> 2) - 1;
    int d = (this->flag & 0b00000011) - 1;
    int x = ((this->data & 0b11110000) >> 4);
    int y = (this->data & 0b00001111);
    
    std::cout << a << " " << b << " " << x << std::endl;
    std::cout << c << " " << d << " " << y << std::endl;
  }

};

// (おそらく)計算量を削減したもの
// dp4a対応
// 並列数によっては遅くなる可能性あり
// 符号が正しく処理されるか実行して確認をとること
// data[2] = { abcd, 00xy } 長さ2で固定
struct RotationMatrixOptCulDp4a {
  uint32_t data[2];

  __host__ __device__
  RotationMatrixOptCulDp4a() {
    this->data[0] = 0;
    this->data[1] = 0;
  }

  __host__ __device__
  RotationMatrixOptCulDp4a(uint32_t a, uint32_t b) {
    this->data[0] = a;
    this->data[1] = b;
  }

  // 反時計回り
  __host__ __device__
  static RotationMatrixOptCulDp4a left(uint8_t x, uint8_t y, uint8_t n) {
    int32_t b = (x - y) << 8 | (x + n - 1);
    return RotationMatrixOptCulDp4a(0x0001ff00, b);
  }

  // 時計回り
  __host__ __device__
  static RotationMatrixOptCulDp4a right(uint8_t x, uint8_t y, uint8_t n) {
    int32_t b = (x + y + n - 1) << 8 | (y - x);
    return RotationMatrixOptCulDp4a(0x00ff0100, b);
  }

  // this * other
  // otherは転置後を想定
  // other.data = { acbd, 00xy}
  __device__ __forceinline__
  RotationMatrixOptCulDp4a operator*(const RotationMatrixOptCulDp4a& other) const {
    RotationMatrixOptCulDp4a ret;
    int32_t a1 = this->data[0] >> 16;
    int32_t a2 = this->data[0] &  0xffff;
    int32_t ax = this->data[1] >> 8;
    int32_t ay = this->data[1] &  0xff;

    int32_t b1 = other.data[0] >> 16;
    int32_t b2 = other.data[0] &  0xffff;

    int32_t ra = __dp4a(a1, b1, 0);
    int32_t rb = __dp4a(a1, b2, 0);
    int32_t rc = __dp4a(a2, b1, 0);
    int32_t rd = __dp4a(a2, b2, 0);

    ret.data[0] = ra | (rb << 8) | (rc << 16) | (rd << 24);

    ra = __dp4a(a1, (int32_t)other.data[1], ax);
    rb = __dp4a(a2, (int32_t)other.data[1], ay);

    ret.data[1] = ra | (rb << 8);

    return ret;
  }

  __device__ __forceinline__
  void mult(const RotationMatrixOptCulDp4a& other) {
    RotationMatrixOptCulDp4a ret;
    int32_t a1 = this->data[0] >>  16;
    int32_t a2 = this->data[0] &  0xffff;
    int32_t ax = this->data[1] >> 8;
    int32_t ay = this->data[1] &  0xff;

    int32_t b1 = other.data[0] >> 16;
    int32_t b2 = other.data[0] &  0xffff;

    int32_t ra = __dp4a(a1, b1, 0);
    int32_t rb = __dp4a(a1, b2, 0);
    int32_t rc = __dp4a(a2, b1, 0);
    int32_t rd = __dp4a(a2, b2, 0);

    this->data[0] = ra | (rb << 8) | (rc << 16) | (rd << 24);

    ra = __dp4a(a1, (int32_t)other.data[1], ax);
    rb = __dp4a(a2, (int32_t)other.data[1], ay);

    this->data[1] = ra | (rb << 8);

  }

  __device__ __forceinline__
  void culc(uint8_t x,uint8_t y, uint8_t *buf) const {
    int32_t a1 = this->data[0] >> 16;
    int32_t a2 = this->data[0] &  0xffff;
    int32_t ax = this->data[1] >> 8;
    int32_t ay = this->data[1] &  0xff;
    int32_t b  = (x << 8) | y;
    buf[0] = (uint8_t)__dp4a(a1, b, ax);
    buf[1] = (uint8_t)__dp4a(a2, b, ay);
  }

  __host__
  void print() const {
    std::cout << (int)(this->data[0] >> 24) << " " << (int)((this->data[0] >> 16) & 0xff) << " " << (int)(this->data[1] >> 8)   << std::endl;
    std::cout << (int)((this->data[0] >> 8) & 0xff) << " " << (int)(this->data[0] & 0xff) << " " << (int)(this->data[5] & 0xff) << std::endl;
  }



};

// (おそらく)計算量を削減したもの
// dp4a未対応
struct RotationMatrixOptCul {
  int8_t data[6]; // {a, b, c, d, x, y} 長さ6固定

  __host__ __device__
  RotationMatrixOptCul() {
    for(size_t i = 0; i < 6; ++i) this->data[i] = 0;
  }

  // 反時計回り
  __host__ __device__
  static RotationMatrixOptCul left(uint8_t x, uint8_t y, uint8_t n) {
    RotationMatrixOptCul ret;
    ret.data[0] = 0;
    ret.data[1] = 1;
    ret.data[2] = -1;
    ret.data[3] = 0;
    ret.data[4] = x - y;
    ret.data[5] = x + n - 1;
    return ret;
  }

  // 時計回り
  __host__ __device__
  static RotationMatrixOptCul right(uint8_t x, uint8_t y, uint8_t n) {
    RotationMatrixOptCul ret;
    ret.data[0] = 0;
    ret.data[1] = -1;
    ret.data[2] = 1;
    ret.data[3] = 0;
    ret.data[4] = x + y + n - 1;
    ret.data[5] = y - x;
    return ret;
  }
  
  // this * other
  __host__ __device__
  RotationMatrixOptCul operator*(const RotationMatrixOptCul& other) const {
    RotationMatrixOptCul ret;

    ret.data[0] = this->data[0] * other.data[0] + this->data[1] * other.data[2];
    ret.data[1] = this->data[0] * other.data[1] + this->data[1] * other.data[3];
    ret.data[2] = this->data[2] * other.data[0] + this->data[3] * other.data[2];
    ret.data[3] = this->data[2] * other.data[1] + this->data[3] * other.data[3];

    ret.data[4] = this->data[0] * other.data[4] + this->data[1] * other.data[5] + this->data[4];
    ret.data[5] = this->data[2] * other.data[4] + this->data[3] * other.data[5] + this->data[5];
    return ret;
  }

  __host__ __device__
  void mult(const RotationMatrixOptCul& other) {
    this->data[4] += this->data[0] * other.data[4] + this->data[1] * other.data[5];
    this->data[5] += this->data[2] * other.data[4] + this->data[3] * other.data[5];

    int8_t buf = this->data[1];
    this->data[0] = this->data[0] * other.data[0] + this->data[1] * other.data[2];
    this->data[1] = buf * other.data[1] + this->data[1] * other.data[3];
    buf = this->data[2];
    this->data[2] = this->data[2] * other.data[0] + this->data[3] * other.data[2];
    this->data[3] = buf * other.data[1] + this->data[3] * other.data[3];
  }

  __host__ __device__
  void culc(uint8_t x,uint8_t y, uint8_t *buf) const {
    buf[0] = this->data[0] * x + this->data[1] * y + this->data[4];
    buf[1] = this->data[2] * x + this->data[3] * y + this->data[5];
  }

  __host__
  void print() const {
    std::cout << (int)this->data[0] << " " << (int)this->data[1] << " " << (int)this->data[4] << std::endl;
    std::cout << (int)this->data[2] << " " << (int)this->data[3] << " " << (int)this->data[5] << std::endl;
  }

};


/*  0  1  X-Y
 * -1  0  X+Y+N-1
 */
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




#endif
