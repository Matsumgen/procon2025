#ifndef ALGO_1_2
#define ALGO_1_2

#include<BitField.hpp>
#include<array>

/* 単純なビームサーチ。盤面を4分割してそれぞれを小盤面に扱えるようにする
 * 盤面の大きさがNの時、盤面を回転させる方法M = N(N-1)(2N-1)/6 - 1
 * N=24の時M=4323回
 * ビーム幅W、深さDとすると、確保すべきBFieldの数Gは
 * G = W*M^D
 * RBFieldは72byte
 */
 // 分割の方法はサッと作るには難しい。
 // 単にNumAjustmjentでやろう
namespace algo1_2 {
  // 2^22
  #define GC_LIMIT 4194304
  using namespace bf;

  struct GCField {
    RBField f;
    uint16_t rank;

    GCField() = default;
    GCField(RBField f);
    GCField(RBField f, std::uint16_t rank);
    GCField copy();
    void update();
  };

  inline std::array<GCField, GC_LIMIT> GC;
  inline std::array<GCField, GC_LIMIT> GC_buffer;
  inline std::array<GCField, 32> GC_next;
  inline size_t gcb = 0; // GC_bufferの最大インデックス
  inline size_t gcm = 0; // GCの最大インデックス
  inline size_t gcn = 0;

  inline std::array<std::uint16_t, 25> f_nums = [](){
    std::array<std::uint16_t, 25> ret;
    for(std::uint16_t i = 0; i < 25; ++i) ret[i] = (i * i) >> 1;
    return ret;
  }(); // f_nums[f.getSize()] = (f.getSize() * f.getSize()) << 1;

  RBField run(RBField& f, std::uint8_t deep, size_t width);
  bool subprocess(GCField& base, GCField &ret, std::uint8_t x, std::uint8_t y, std::uint8_t n);
  uint16_t evaluation(RBField &f);

}


#endif
