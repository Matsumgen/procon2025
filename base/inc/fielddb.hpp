#ifndef FIELD_DB_HPP_
#define FIELD_DB_HPP_

#include <Field.hpp>
#include <BitField.hpp>
#include <lmdb.h>

/*
一度にまとめて呼び出す方が メモリの局所性により速くなる可能性が高い
*/

namespace fdb{
  inline MDB_env *field4_env = nullptr;
  inline MDB_dbi field4_dbi  = 0;
  inline MDB_txn *field4_txn = nullptr;
  inline std::array<std::array<std::uint8_t, 3>, 256> f4decodeOpe = {};

  bool field4_init(const char *db_path);
  void field4_deinit();

  bool resolveField4(bf::BitField& f);
  std::vector<std::array<std::uint8_t, 3>> getField4(bf::BitField f);
  std::vector<std::uint8_t> encodeField4(bf::BitField& f);


  std::vector<std::array<std::uint8_t, 3>> getField4(Field f);
  std::vector<std::uint8_t> encodeField4(Field& f);

  //LUT化
  std::array<std::uint8_t, 3> decodeOperate(const std::uint8_t& ope);

}

#endif
