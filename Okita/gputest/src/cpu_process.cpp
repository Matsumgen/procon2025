#include <cpu_process.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>

bool checkProblem(const uint32_t fsize, const uint16_t *f) {
  std::vector<int> a(fsize * fsize / 2, 0);
  for(size_t y = 0; y < fsize; ++y) {
    for(size_t x = 0; x < fsize; ++x) {
      size_t i = x + y * fsize;
      ++a[f[i]];
    }
  }
  for(size_t i = 0; i < a.size(); ++i){
    if(a[i] != 2){
      for(size_t j = 0; j < a.size(); ++j){ std::cout << "(" << j << ", " << a[j] << "), ";}
      std::cout << std::endl;
      return false;
    }
  }
  return true;
}


std::vector<uint16_t> makeShuffledPairs(const uint16_t fsize) {
    const uint16_t n = fsize * fsize / 2;
    std::vector<uint16_t> result;
    result.reserve(n * 2);

    // 各数字を2回ずつ追加
    for (uint16_t i = 0; i < n; ++i) {
        result.push_back(i);
        result.push_back(i);
    }

    // 乱数生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // シャッフル
    std::shuffle(result.begin(), result.end(), gen);

    return result;
}

void printField(const uint32_t fsize, const uint16_t *field, const uint16_t *before_f) {
  for(size_t y = 0; y < fsize; ++y) {
    for(size_t x = 0; x < fsize; ++x) {
      size_t i = y * fsize + x;
      if(field[i] != before_f[i])
        printf("\x1b[36m%3d\x1b[0m ", (int)field[i]);
      else
        printf("%3d ", (int)field[i]);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void printField(const uint32_t fsize, const uint16_t *field) {
  for(size_t y = 0; y < fsize; ++y) {
    for(size_t x = 0; x < fsize; ++x) {
      size_t i = y * fsize + x;
      printf("%3d ", (int)field[i]);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

uint32_t  getTargetIndex(const uint16_t *vals, const uint32_t length, const uint16_t v) {
  int32_t index = length / 2;
  int32_t min_i = 0;
  int32_t max_i = length - 1;
  while(min_i < max_i) {
    if(vals[index] < v) {
      max_i = index - 1;
    }else{
      min_i = index + 1;
    }
    index = (min_i + max_i) / 2;
  }
  return index;
}

void getParamsCpu(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t sn, uint32_t slice) {
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

std::vector<uint32_t> getTidPerFieldList(uint32_t sn, uint32_t en, uint32_t slice) {
  std::vector<uint32_t> ret(24, 0);
  uint32_t X, Y, N;
  for(uint32_t i = 4; i < 26; i += 2) {
    for(uint32_t t = 0; t < i * i * i; ++t){
      getParamsCpu(t, i, &X, &Y, &N, sn, slice);
      if(en <= N || i <= N){
        ret[i] = t;
        break;
      }
    }
  }
  return ret;
}

uint32_t getTidPerField(uint32_t fsize, uint32_t sn, uint32_t en, uint32_t slice) {
  uint32_t X, Y, N;
  for(uint32_t t = 0; t < fsize * fsize * fsize; ++t){
    getParamsCpu(t, fsize, &X, &Y, &N, sn, slice);
    if(en <= N || fsize <= N){
      return t;
    }
  }
  return 0;
}

void rotateField(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t rid){
  uint32_t x, y, n;
  getParamsCpu(rid, fsize, &x, &y, &n);
  rotateField(field, fsize, x, y, n);
}
void rotateField(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n) {
  uint32_t n_half = n >> 1;

  uint32_t h, w;
  uint32_t i0, i1, i2, i3, buf;
  for(h = 0; h < n_half; ++h) {
    i0 = x + (y+h) * fsize;
    i1 = x + (n - 1 - h) + y * fsize;
    i2 = (x + (n - 1)) + (y + (n - 1 - h)) * fsize;
    i3 = (x + h) + (y + (n - 1)) * fsize;
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
  }

  if(n & 1){
    i0 = x+n_half + y * fsize;
    i1 = x + n - 1 + (y + n_half) * fsize;
    i2 = x+n_half + (y + n - 1) * fsize;
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

bool isIndexAdjacent(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t i) {
  std::uint16_t num = field[i];
  const size_t x = i % fsize;

  const size_t up    = i >= fsize                 ? i - fsize  : 1<<20;
  const size_t down  = i + fsize < fsize * fsize  ? i + fsize  : 1<<20;
  const size_t left  = x != 0                     ? i - 1      : 1<<20;
  const size_t right = x != fsize-1               ? i + 1      : 1<<20;

  return (up    != 1<<20 && field[up]    == num) ||
         (down  != 1<<20 && field[down]  == num) ||
         (left  != 1<<20 && field[left]  == num) ||
         (right != 1<<20 && field[right] == num);
}

bool isEnd(std::vector<uint16_t>& field, const uint32_t fsize) {
  for(std::uint16_t i=0; i < field.size(); ++i){
    if(!isIndexAdjacent(field, fsize, i)){ return false; }
  }
  return true;
}

