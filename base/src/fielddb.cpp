/* #define DEBUG_FIELD_DB */
#include <fielddb.hpp>
#include <lmdb.h>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdint>
#include <map>

#ifdef DEBUG_FIELD_DB
#include <bitset>
#endif

using namespace fdb;

bool fdb::field4_init(const char *db_path){
  int ret = mdb_env_create(&field4_env);

  // 環境を初期化
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_create failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // データベースのパスに環境を設定
  ret = mdb_env_open(field4_env, db_path, MDB_RDONLY, 0664);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }
  

  // データベースをオープン
  ret = mdb_txn_begin(field4_env, nullptr, MDB_RDONLY, &field4_txn);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_txn_begin failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  ret = mdb_dbi_open(field4_txn, nullptr, 0, &field4_dbi);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_dbi_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // field4のdecodeOperateを事前に配列に保存
  f4decodeOpe[2] = {0, 0, 2};
  f4decodeOpe[3] = {0, 0, 3};
  f4decodeOpe[6] = {0, 1, 2};
  f4decodeOpe[7] = {0, 1, 3};
  f4decodeOpe[10] = {0, 2, 2};
  f4decodeOpe[18] = {1, 0, 2};
  f4decodeOpe[19] = {1, 0, 3};
  f4decodeOpe[22] = {1, 1, 2};
  f4decodeOpe[23] = {1, 1, 3};
  f4decodeOpe[26] = {1, 2, 2};
  f4decodeOpe[34] = {2, 0, 2};
  f4decodeOpe[38] = {2, 1, 2};
  f4decodeOpe[42] = {2, 2, 2};

  return true;
}

void fdb::field4_deinit(){
  if (field4_txn) { mdb_txn_abort(field4_txn); }
  if (field4_dbi) { mdb_dbi_close(field4_env, field4_dbi); }
  if (field4_env) { mdb_env_close(field4_env); }
}




bool fdb::resolveField4(bf::BitField& f) {
  std::vector<std::uint8_t> key;
  MDB_val mdb_key, mdb_data;
  std::uint8_t value;
  int r;
  while(true){
    key = encodeField4(f);
    mdb_key.mv_size = key.size();
    mdb_key.mv_data = key.data();
    r = mdb_get(field4_txn, field4_dbi, &mdb_key, &mdb_data);
    if (r != MDB_SUCCESS) {
      if (r != MDB_NOTFOUND){
        std::cerr << "mdb_get failed: " << mdb_strerror(r) << std::endl;
      }
      return false;
    }
    value = *(static_cast<std::uint8_t*>(mdb_data.mv_data));
    if(value == 0)  break;
    const auto& ope = f4decodeOpe[value];
#ifdef DEBUG_FIELD_DB
    if(ope[2] == 0) {
      std::cerr << "field4 decodeOperate failed: " << mdb_strerror(r) << std::endl;
      return false;
    }
#endif
    f.rotate(ope[0], ope[1], ope[2]);
  }
  return !f.isEnd();
}

std::vector<std::array<std::uint8_t, 3>> fdb::getField4(bf::BitField f){
  std::vector<std::array<std::uint8_t, 3>> ret;
  ret.reserve(8);
  std::vector<std::uint8_t> key;
  MDB_val mdb_key, mdb_data;
  int r;
  
  while(true){
    key = encodeField4(f);
    mdb_key.mv_size = key.size();
    mdb_key.mv_data = (void*)key.data();

    // データを取得
    r = mdb_get(field4_txn, field4_dbi, &mdb_key, &mdb_data);
    if (r != MDB_SUCCESS) {
      if (r != MDB_NOTFOUND){
        std::cerr << "mdb_get failed: " << mdb_strerror(r) << std::endl;
      }
      return std::vector<std::array<std::uint8_t, 3>>();
    }

    if (mdb_data.mv_size == 1) {
      std::uint8_t value = *((std::uint8_t*)mdb_data.mv_data);
      if(value == 0)  break;
      std::array<std::uint8_t, 3> ope = decodeOperate(value);
      f.rotate(ope[0], ope[1], ope[2]);
      ret.push_back(ope);
    }else{
      std::cerr << "Invalid data size: Expected 1 byte." << std::endl;
      ret.clear();
      return ret;
    }
  }
  return ret;
}

// field4dbで作成されたPDBのみ対応
std::vector<std::uint8_t> fdb::encodeField4(bf::BitField& f) {
  if(f.getSize() != 4){
    std::cerr << "Field size is not 4: " << f.getSize() << std::endl;
    return std::vector<std::uint8_t>();
  }

  f.reallocation();

  // 最初の12要素のカウント
  std::array<std::uint16_t, 8> d{};
  std::array<std::uint16_t, 8> d_index{};

  for(std::uint16_t i = 0; i < 12; ++i){ d[f.get(i)] += 1; }

  auto update_d_index = [&](){
    std::uint16_t count = 0;
    for(auto i = 0; i < 8; ++i) d_index[i] = (d[i] > 1) ? 255 : count++;
  };

  // バイト列を格納するベクター
  std::vector<std::uint8_t> ret;
  ret.reserve(6);

  std::uint8_t bi = (f.get(1) << 7) | (f.get(2) << 5) | (f.get(3) << 3) | f.get(4);
  ret.push_back(bi);

  bi = (f.get(5) << 5) | (f.get(6) << 2) | (f.get(7) >> 1);
  ret.push_back(bi);

  bi = ((f.get(7) & 0b001) << 7) | (f.get(8) << 4) | (f.get(9) << 1) | (f.get(10) >> 2);
  ret.push_back(bi);

  bi = ((f.get(10) & 0b011) << 6) | (f.get(11) << 3);

  update_d_index();
  ++d[f.get(12)];
  bi += (d_index[f.get(12)] << 1);

  update_d_index();
  ++d[f.get(13)];
  bi += (d_index[f.get(13)] >> 1);
  ret.push_back(bi);

  bi = (d_index[f.get(13)] & 0b01) << 7;

  update_d_index();
  ++d[f.get(14)];
  bi += (d_index[f.get(14)] << 6);
  ret.push_back(bi);

#ifdef DEBUG_FIELD_DB
  std::cout << "encodeField4: ";
  for(std::uint8_t a : ret)  std::cout << std::bitset<8>(a) << " ";
  std::cout << std::endl;

  f.print();
#endif

  return ret;
}




std::vector<std::array<std::uint8_t, 3>> fdb::getField4(Field f){
  std::vector<std::array<std::uint8_t, 3>> ret;
  ret.reserve(8);

  std::vector<std::uint8_t> key;
  MDB_val mdb_key, mdb_data;
  int r;
  
  while(true){
    key = encodeField4(f);
    mdb_key.mv_size = key.size();
    mdb_key.mv_data = (void*)key.data();

    // データを取得
    r = mdb_get(field4_txn, field4_dbi, &mdb_key, &mdb_data);
    if (r != MDB_SUCCESS) {
      if (r == MDB_NOTFOUND){
        std::cout << "Key not found in the database." << std::endl;
      }else{
        std::cerr << "mdb_get failed: " << mdb_strerror(r) << std::endl;
      }
      ret.clear();
      return ret;
    }

    if (mdb_data.mv_size == 1) {
      std::uint8_t value = *((std::uint8_t*)mdb_data.mv_data);
      if(value == 0)  break;
      std::array<std::uint8_t, 3> ope = decodeOperate(value);
      f.rotate(ope[0], ope[1], ope[2]);
      ret.push_back(ope);
    }else{
      std::cerr << "Invalid data size: Expected 1 byte." << std::endl;
      ret.clear();
      return ret;
    }
  }
  return ret;
}

std::vector<std::uint8_t> fdb::encodeField4(Field& f){
  if(f.getSize() != 4){
    std::cerr << "Field size is not 4: " << f.getSize() << std::endl;
    return std::vector<std::uint8_t>();
  }
  std::unordered_map<int, int> dic = Field::reallocation_map(f);

  auto g = [&](int x, int y){
    return dic[f.get(x, y)->num];
  };

  // 最初の12要素のカウント
  std::map<int, int> d;
  std::unordered_map<int, int> d_index;
  for(int i=0; i<8; ++i)  d[i] = 0;
  for(int y = 0; y < 3; ++y) for(int x = 0; x < 4; ++x){ d[g(x,y)] += 1; }

  auto update_d_index = [&](){
    // 出現回数が1回の要素だけを残す
    for (auto it = d.begin(); it != d.end(); ) {
      if (it->second > 1) {
        it = d.erase(it);
      } else {
        ++it;
      }
    }

    // d_index の作成（ソートなし）
    int index = 0;
    d_index.clear();
    for (const auto& kv : d) {
      d_index[kv.first] = index++;
    }
  };

  // バイト列を格納するベクター
  std::vector<std::uint8_t> ret;

  // 最初の8ビットを取得
  std::uint8_t bi = (g(1,0) << 7) | (g(2,0) << 5) | (g(3,0) << 3) | g(0,1);
  ret.push_back(bi);

// 次の8ビットを取得
  bi = (g(1,1) << 5) | (g(2,1) << 2) | (g(3,1) >> 1);
  ret.push_back(bi);

  // 次の8ビットを取得
  bi = ((g(3,1) & 0b001) << 7) | (g(0,2) << 4) | (g(1,2) << 1) | (g(2,2) >> 2);
  ret.push_back(bi);

  // 次の8ビットを取得
  bi = ((g(2,2) & 0b011) << 6) | (g(3,2) << 3);
  update_d_index();
  d[g(0,3)] += 1;
  bi += (d_index[g(0,3)] << 1);
  update_d_index();
  d[g(1,3)] += 1;
  bi += (d_index[g(1,3)] >> 1);
  ret.push_back(bi);

  // 最後のビット
  bi = (d_index[g(1,3)] & 0b01) << 7;
  update_d_index();
  d[g(2,3)] += 1;
  bi += (d_index[g(2,3)] << 6);
  ret.push_back(bi);

#ifdef DEBUG_FIELD_DB
  std::cout << "encodeField4: ";
  for(std::uint8_t a : ret)  std::cout << std::bitset<8>(a) << " ";
  std::cout << std::endl;

  f.print();
#endif

  return ret;
  
}





std::array<std::uint8_t, 3> fdb::decodeOperate(const std::uint8_t& ope){
  std::array<std::uint8_t, 3> ret = { static_cast<std::uint8_t>((ope >> 4) & 3), static_cast<std::uint8_t>((ope >> 2) & 3),static_cast<std::uint8_t>(ope & 3) };
#ifdef DEBUG_FIELD_DB
  std::cout << "decodeOperate: " << std::bitset<8>(ope) << " -> ";
  for(auto& r : ret)  std::cout << r << ", ";
  std::cout << std::endl;
#endif
  return ret;
}



