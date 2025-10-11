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
  #define GC_SIZE 10000
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
  void adaptation(const std::uint8_t X1, const bool& hor,const std::uint8_t fsize);
  std::string str() const;
  std::array<PKey, 8> getIsotopes();
  };
  /* typedef std::pair<PKey, std::vector<std::array<std::uint8_t, 3>>> Record; */
  typedef std::unordered_map<std::uint16_t, std::vector<std::array<std::uint8_t, 3>>> Record;

  // 揃った数sx, fsize
  struct Key {
    std::uint8_t depth;
    std::uint8_t x1;
    std::uint8_t x2;
    PKey pk;
    std::string str() const;
  };

// PSKey 総数：2901295
// 1盤面で2ペアの組み合わせの総数：41328
  inline std::unordered_map<std::uint64_t, Record> FSDB24{};
  /* inline std::array<std::vector<Record>, 1587> FSDB24{}; */
  inline std::array<SRoutes_ptr, GC_SIZE> GC{};
  inline std::array<PKey, 41328> gPC{};

  //24以外は対応しない
  bool fsdb_init(const char *db_path);

  void decodeValue(std::vector<std::array<std::uint8_t, 3>>& ret, std::vector<std::uint8_t>& val);
  void decodeKey(Key& ret, std::vector<std::uint8_t>& key);


  Routes getOperation(const State *s);
}


#endif
