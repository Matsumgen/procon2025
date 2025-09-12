#define DEBUG_ALGO1_2
#include<algo1_2.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <omp.h>


using namespace algo1_2;


GCField::GCField(RBField f) : f(std::move(f)), rank(0) {};
GCField::GCField(RBField f, std::uint16_t rank) : f(std::move(f)), rank(rank) {};

void GCField::update() {
  this->rank = evaluation(this->f);
}
GCField GCField::copy() {
  return GCField(this->f.copy());
}

//盤面を小さくできるように(GCFieldにpadding (x1, y1), (x2, y2)追加)
//盤面を小さくできる状態に近いほどrankが高く
RBField algo1_2::run(RBField& f, std::uint8_t deep, size_t width) {

  std::uint8_t x, y, n;
  const std::uint8_t fsize = f.getSize();
  const std::uint8_t xy_len = fsize - 1;
  size_t i, j, max_loop;

  GC[gcm++] = GCField(f);

  RBField ret;
  bool is_end = false;

#ifdef DEBUG_ALGO1_2
  for(int k = 0; k < 25; k++){
    std::cout << k << ": " << f_nums[k] << " ";
  }
    std::cout << std::endl;
  uint32_t loop_count = 0;
  auto printgc = [&](){
    std::cout << "loop_count: " << loop_count << std::endl;
    std::cout << "fsize: " << (int)fsize << ", xy_len: " << (int)xy_len << std::endl;
    std::cout << "gcm: " << gcm << std::endl;
    std::cout << "max_loop: " << max_loop << std::endl;
    std::cout << "rank: " << GC[0].rank << std::endl;
    GC[0].f.print();
    /* for(size_t t = 0; t < max_loop; ++t){ */
    /*   std::cout << t << " :" << GC[t].rank << " "; */
    /*   if(t%20 == 0 && t > 1) std::cout << std::endl; */
    /* } */
    std::cout << std::endl;
  };
  std::cout << "start loop" << std::endl;
#endif
  while(1){
    max_loop = (width >  gcm) ? gcm : width;
#ifdef DEBUG_ALGO1_2
    loop_count += 1;
    printgc();
#endif
    for(j = 0; j < deep; ++j){
      for(i = 0; i < max_loop; ++i){
        #pragma omp for collapse(3)
        for(y = 0; y < xy_len; ++y) {
          for(x = 0; x < xy_len; ++x) {
            for(n = 2; n < fsize; ++n) {
              if (std::max(y + n, x + n) > fsize || is_end) continue;
/* #ifdef DEBUG_ALGO1_2 */
/*               #pragma omp critical */
/*               { */
/*                 if(GC_LIMIT <= max_loop){ */
/*                   std::cout << "Overflow loop" << std::endl; */
/*                 } */
/*                 std::cout << "(x, y ,n, i, j), gcb = " << "(" << (int)x << ", " << (int)y << ", " << (int)n << ", " << i << ", " << j << "), " << gcb << std::endl; */
/*               } */
/* #endif */
              if(subprocess(GC[i], GC_buffer[gcb], x, y, n)){
                ret = GC_buffer[gcb].f;
                is_end = true;
              }
              ++gcb;
            }
          }
        }
      }
      if(is_end) return ret;
      std::swap(GC, GC_buffer);
      gcm = gcb;
      gcb = 0;
      max_loop = gcm;
    }
    std::sort(GC.begin(), GC.begin() + gcm, [](GCField a, GCField b){ return a.rank > b.rank; });
  }

}

bool algo1_2::subprocess(GCField& base, GCField &ret, std::uint8_t x, std::uint8_t y, std::uint8_t n) {
  ret = base.copy();
  ret.f.rotate(x, y, n);
  ret.update();
  return ret.rank >= f_nums[ret.f.getSize()];
}

uint16_t algo1_2::evaluation(RBField &f){
  uint16_t fit = 0;
  for(uint16_t i=0; i<f_nums[f.getSize()]; ++i){
    if(f.isNumAdjacent(i)) ++fit;
  }
  return fit;
}

