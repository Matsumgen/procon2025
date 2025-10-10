/* #define DEBUG_FIELD_DB */
#include "../inc/fielddb.hpp"
#include <lmdb.h>
#include <map>             
#include <unordered_map> 

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

v_ope fdb::getField4(Field &f){
  Ent tmp_ent_mem[f.size * f.size];
  Pos tmp_pos_mem[f.size * f.size];
  Field tmp_f = Field(f.size, tmp_ent_mem, tmp_pos_mem);
  f.getClone(&tmp_f);

  v_ope ret;
  ret.reserve(8);

  std::vector<std::uint8_t> key;
  MDB_val mdb_key, mdb_data;
  int r;
  
  while(true){
    key = encodeField4(tmp_f);
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
      exit(1);
      return ret;
    }

    if (mdb_data.mv_size == 1) {
      std::uint8_t value = *((std::uint8_t*)mdb_data.mv_data);
      if(value == 0)  break;
      Ope ope = decodeOperate(value);
      tmp_f.rotate(ope);
      ret.push_back(ope);
    }else{
      std::cerr << "Invalid data size: Expected 1 byte." << std::endl;
      ret.clear();
      exit(1);
      return ret;
    }
  }
  return ret;
}

std::vector<std::uint8_t> fdb::encodeField4(Field& f){
  if(f.size != 4){
    std::cerr << "Field size is not 4: " << f.size << std::endl;
    return std::vector<std::uint8_t>();
  }
//   std::unordered_map<int, int> dic = Field::reallocation_map(f);
  f.reallocation();

  auto g = [&](int x, int y){
    //return dic[f.get(x, y)->num];
    return f.getEnt(y, x).val;
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





Ope fdb::decodeOperate(const std::uint8_t& ope){
  Ope ret = Ope(static_cast<short>((ope >> 4) & 3), static_cast<short>((ope >> 2) & 3), static_cast<short>(ope & 3));
#ifdef DEBUG_FIELD_DB
  std::cout << "decodeOperate: " << std::bitset<8>(ope) << " -> ";
  for(auto& r : ret)  std::cout << r << ", ";
  std::cout << std::endl;
#endif
  return ret;
}



