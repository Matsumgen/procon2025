#ifndef MATRIX_FIELD_CUH_
#define MATRIX_FIELD_CUH_

#include <matrix.cuh>
#include <vector>
#include <algorithm>
#include <random>


__host__
bool checkProblem(const uint32_t fsize, const uint16_t *f) {
  std::vector<int> a(fsize * fsize / 2, 0);
  for(size_t x = 0; x < fsize; ++x) {
    for(size_t y = 0; y < fsize; ++y) {
      size_t i = x + y * fsize;
      ++a[f[i]];
    }
  }
  for(size_t i = 0; i < a.size(); ++i){
    if(a[i] != 2) return false;
  }
  return true;
}


__host__
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

__host__
void printField(const uint32_t fsize, const uint16_t *field, const uint16_t *before_f) {
  for(size_t x = 0; x < fsize; ++x) {
    for(size_t y = 0; y < fsize; ++y) {
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

__host__
void printField(const uint32_t fsize, const uint16_t *field) {
  for(size_t x = 0; x < fsize; ++x) {
    for(size_t y = 0; y < fsize; ++y) {
      size_t i = y * fsize + x;
      printf("%3d ", (int)field[i]);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

// dp4aが使える場合に使用加
// data: { abcd, 00xy } * fsize * fsize * 2
// field: fsize * fsize
// fsize: fieldのsize
struct MatrixArrayDp4a {
  uint32_t *data;
  uint16_t *field;
  uint8_t fsize;

  __host__ __device__
  MatrixArrayDp4a(uint32_t *data, uint16_t *field, uint8_t fsize) : data(data), field(field), fsize(fsize) {}
  __host__
  MatrixArrayDp4a(uint16_t fsize) : fsize(fsize) {
    uint32_t size = fsize * fsize * 2;
    uint32_t field_size = size >> 1;
    std::vector<uint16_t> h_f = makeShuffledPairs(fsize);
    std::vector<uint32_t> h_data(size);
    for (size_t i = 0; i < size; i += 2) {
      h_data[i] = 0x01000001;
      h_data[i+1] = 0;
    }

    cudaError_t err = cudaMallocManaged(&this->data, size * sizeof(uint32_t));
    if (err != cudaSuccess) {
      std::cerr << "cudaMalloc failed: "
                << cudaGetErrorString(err) << std::endl;
        return ;
    }

    err = cudaMallocManaged(&this->field, field_size * sizeof(uint16_t));
    if (err != cudaSuccess) {
      std::cerr << "cudaMalloc failed: "
                << cudaGetErrorString(err) << std::endl;
        return ;
    }

    err = cudaMemcpy(this->data, h_data.data(), size * sizeof(uint32_t), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cerr << "cudaMemcpy failed: "
                  << cudaGetErrorString(err) << std::endl;
        cudaFree(this->data);
        return ;
    }

    err = cudaMemcpy(this->field, h_f.data(), field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cerr << "cudaMemcpy failed: "
                  << cudaGetErrorString(err) << std::endl;
        cudaFree(this->data);
        return ;
    }
  }

  __host__
  void close() {
    cudaFree(this->data);
    cudaFree(this->field);
  }

  // 破壊的処理
  __device__ __forceinline__
  void rotate(const uint8_t x, const uint8_t y, const uint8_t n) {
    uint32_t rot[2];
    createMatrixArrayL(x, y, n, rot);
    uint32_t i = 0;
    for(uint32_t iy = y; iy < y + n; ++iy){
      for(uint32_t ix = x; ix < x + n; ++ix) {
        i = iy * this->fsize + ix;
        /* printf("rotate: ix=%d, iy=%d, i=%d\n", ix, iy, i); */
        /* printf("before: %x, %x\n", this->data[i*2], this->data[i*2+1]); */
        /* printf("rot   : %x, %x\n", rot[0], rot[1]); */
        multDp4a(this->data + i * 2, rot);
        /* printf("after : %x, %x\n", this->data[i*2], this->data[i*2+1]); */
        
      }
    }
  }

  
  __device__ __forceinline__
  void culc(uint16_t *result) const {
    size_t i, j;
    uint8_t p[2];
    for(uint8_t y = 0; y < this->fsize; ++y) {
      for(uint8_t x = 0; x < this->fsize; ++x) {
        p[0] = x;
        p[1] = y;
        i = y * this->fsize + x;
        culcDp4a(this->data + i * 2, p);
        j = p[1] * this->fsize + p[0];
        /* printf("culc: %d, %d, %d, %d\n", x, y, (int)p[0], (int)p[1]); */
        result[j] = this->field[i];
      }
    }
  }

  __host__
  void print() const {
    size_t i = 0;
    for(size_t x = 0; x < this->fsize; ++x) {
      for(size_t y = 0; y < this->fsize; ++y) {
        i = y * this->fsize + x;
        printf("%3d ", (int)this->field[i]);
      }
      std::cout << std::endl;
    }
  }

  

};



#endif
