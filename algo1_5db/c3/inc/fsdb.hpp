#ifndef FIELD_SMOLLER_DB_HPP_
#define FIELD_SMOLLER_DB_HPP_

#include <fsfield.hpp>
#include <field.hpp>
#include <state.hpp>
#include <array>
#include <vector>
#include <lmdb.h>
#include <string>
#include <utility>

/*
これなんなら回した後のStatusの一覧渡せるのでは

高さで切られることがある
*/
namespace fsdb {
  #define FSIZE 24
  #define GC_SIZE 100000
  union PKey {
    uint64_t value;
    uint8_t bytes[8];

    struct {
      std::array<uint8_t, 2> p4; // 下位16bit（bytes[1], bytes[0]）
      std::array<uint8_t, 2> p3; // bytes[3], bytes[2]
      std::array<uint8_t, 2> p2; // bytes[5], bytes[4]
      std::array<uint8_t, 2> p1; // 上位16bit（bytes[7], bytes[6]）
    };

  bool operator==(const PKey& other);
  };
  typedef std::pair<PKey, std::vector<std::array<std::uint8_t, 3>>> Record;

  // 揃った数sx, fsize
  struct Key {
    std::uint8_t depth;
    std::uint8_t x1;
    std::uint8_t x2;
    PKey pk;
    std::string str() const;
  };

  // CACHE[deep, x1, x2] = u_max<Points, vector<Ope>> // deep++, x1以上, x2以下
  // CACHE[sx/2, fsize] = u_max<Points, vector<Ope>> // fsize--, 同じやつも紛れ込む // CACHE[X, fisze]にはCACHE[X, fsize-1]を含まない

  // 23(12(横)+11(縦)) * 12(2~24(偶数))
  // CACHE[(progress >> 1) * 12 + (fsize >> 1)]
  // CACHE[deep * 276 + ((FSIZE - x1 - 2) >> 1) *  12 + ((x2 - x1 + 1) >> 1)]
  // 深さ違いもヒットする
  inline std::array<std::vector<Record>, 1587> FSDB24{};
  inline std::array<SRoutes_ptr, GC_SIZE> GC{};

  //24以外は対応しない
  bool fsdb_init(const char *db_path);

  void decodeValue(std::vector<std::array<std::uint8_t, 3>>& ret, std::vector<std::uint8_t>& val);
  void decodeKey(Key& ret, std::vector<std::uint8_t>& key);


  Routes getOperation(const State *s);
}


#endif
