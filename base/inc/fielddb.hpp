#ifndef FIELD_DB_HPP_
#define FIELD_DB_HPP_

#include <Field.hpp>
#include <lmdb.h>

namespace fdb{
  inline MDB_env *field4_env = nullptr;
  inline MDB_dbi field4_dbi  = 0;
  inline MDB_txn *field4_txn = nullptr;

  bool field4_init(const char *db_path);
  void field4_deinit();

  std::vector<std::array<int, 3>> getField4(Field f);


  std::vector<uint8_t> encodeField4(Field& f);
  std::array<int, 3> decodeOperate(uint8_t ope);

}

#endif
