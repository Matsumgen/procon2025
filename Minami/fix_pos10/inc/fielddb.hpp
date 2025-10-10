#ifndef FIELD_DB_HPP
#define FIELD_DB_HPP 0

#include "field.hpp"
#include <lmdb.h>
#include <iostream>
#include <array>  
#include <vector>

#define FDB4_PATH (char*)"../../../field4_db"
/*
一度にまとめて呼び出す方が メモリの局所性により速くなる可能性が高い
*/

namespace fdb {
    inline MDB_env *field4_env = nullptr;
    inline MDB_dbi field4_dbi  = 0;
    inline MDB_txn *field4_txn = nullptr;
    inline std::array<std::array<std::uint8_t, 3>, 256> f4decodeOpe = {};

    bool field4_init(const char *db_path);
    void field4_deinit();

    // bool resolveField4(bf::BitField& f);
    // std::vector<std::array<std::uint8_t, 3>> getField4(bf::BitField f);
    // std::vector<std::uint8_t> encodeField4(bf::BitField& f);


    v_ope getField4(Field &f);
    std::vector<std::uint8_t> encodeField4(Field& f);

    //LUT化
    Ope decodeOperate(const std::uint8_t& ope);
}

#endif
