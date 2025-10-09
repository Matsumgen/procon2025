#ifndef FIELD_SMOLLER_DB_HPP_
#define FIELD_SMOLLER_DB_HPP_

#include <fsfield.hpp>
#include <field.hpp>
#include <state.hpp>
#include <array>
#include <vector>
#include <lmdb.h>
#include <string>

/*
これなんなら回した後のStatusの一覧渡せるのでは

高さで切られることがある
*/
namespace fsdb {
  inline MDB_env *fs_env = nullptr;
  inline MDB_dbi fs_dbi  = 0;
  inline MDB_txn *fs_txn = nullptr;
  inline std::uint8_t field_size = 0;

  struct Key {
    std::uint8_t depth;
    std::uint8_t x1;
    std::uint8_t x2;
    std::array<std::uint8_t, 2> p1;
    std::array<std::uint8_t, 2> p2;
    std::array<std::uint8_t, 2> p3;
    std::array<std::uint8_t, 2> p4;

    bool correction(std::uint8_t fx1, bool horizon, std::uint8_t fsize);
    std::string str() const;

  };

  bool fsdb_init(const char *db_path, const std::uint8_t fsize);
  void fsdb_deinit();

  std::vector<Ope> decodeValue(std::vector<std::uint8_t>& val);
  Key decodeKey(std::vector<std::uint8_t>& key);


  /* std::vector<std::array<std::uint8_t, 3>> getMinOpes(const bf::BitField& field,  std::uint8_t x1, std::uint8_t x2); */
  
  // deepまで探索, horizon=Trueで縦
  void getAllOperation(SRoutes_ptr r, const std::uint8_t& x1, const std::uint8_t& x2, const std::uint8_t deep, bool horizon);
  Routes getOperation(const State *s);
}


#endif
