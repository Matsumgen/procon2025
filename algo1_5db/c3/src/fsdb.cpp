/* #define DEBUG */
/* #define IGNORE_SKIP1 */

#include <fsdb.hpp>
#include <lmdb.h>
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <utility>

#ifdef DEBUG
#define debug_log(...) _debug_log(__FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_log(...)
#endif

template<typename T>
void stream_with_space(std::ostream& os, T&& arg) {
    os << std::forward<T>(arg);
}

template<typename T, typename... Ts>
void stream_with_space(std::ostream& os, T&& arg, Ts&&... args) {
    os << std::forward<T>(arg) << " ";
    stream_with_space(os, std::forward<Ts>(args)...);
}

template<typename... Args>
void _debug_log(const char* file, int line, Args&&... args) {
    std::ostringstream oss;
    oss << "[DEBUG] " << file << ":" << line << " ";
    stream_with_space(oss, std::forward<Args>(args)...);
    std::cout << oss.str() << std::endl;
}

template<size_t N>
std::string arr_str(std::array<std::uint8_t, N> arr) {
  std::ostringstream oss;
  oss << "(";
  for(size_t i = 0; i < N; ++i) {
    oss << (int)arr[i];
    if(i < N - 1) oss << ", ";
  }
  oss << ")";
  return oss.str();
}

std::string ope_str(Ope ope) {
  std::ostringstream oss;
  oss << "(" << ope.x << ", " << ope.y << ", " << ope.n << ")";
  return oss.str();
}

std::string opes_str(std::vector<Ope> opes) {
  std::ostringstream oss;
  oss << "[ ";
  for(size_t i = 0; i < opes.size(); ++i) {
    oss << ope_str(opes[i]) << " ";
  }
  oss << "]";
  return oss.str();
}

std::string aopes_str(std::vector<std::array<std::uint8_t, 3>> opes) {
  std::ostringstream oss;
  oss << "[ ";
  for(size_t i = 0; i < opes.size(); ++i) {
    oss << arr_str(opes[i]) << " ";
  }
  oss << "]";
  return oss.str();
}

using namespace fsdb;

size_t getIndex(std::uint8_t deep, std::uint8_t x1, std::uint8_t x2) {
  // 23 * 23
  if(1 <= deep && x1 <= 22 && 24 <= x2 && x2 <= 46){
    return (deep - 1) * 529 + x1 * 23 + (x2 - 24);
  } else {
    std::cerr << "deep, x1, x2 = " << (int)deep << ", " << (int)x1 << ", " << (int)x2 << std::endl;
    throw std::invalid_argument("getIndex: out of range");
  }
}

bool getCache(const MDB_dbi& fs_dbi, MDB_txn *fs_txn) {
  MDB_cursor* cursor;
  if (mdb_cursor_open(fs_txn, fs_dbi, &cursor) != 0) {
    std::cerr << "Failed to open cursor" << std::endl;
    return false;
  }

  Key kd;
  std::vector<std::array<std::uint8_t, 3>> val;
  std::vector<std::uint8_t> key_vec(7);
  std::vector<std::uint8_t> value_vec;
  MDB_val key, data;
  std::uint8_t fsize;
  size_t index;
  debug_log("loading fsdb");
  while(mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
    if (key_vec.size() < key.mv_size) key_vec.resize(key.mv_size);
    if (value_vec.size() < data.mv_size) value_vec.resize(data.mv_size);
    std::memcpy(key_vec.data(), key.mv_data, key.mv_size);
    std::memcpy(value_vec.data(), data.mv_data, data.mv_size);
    decodeValue(val, value_vec);
    decodeKey(kd, key_vec);

    if(kd.depth == 0) continue;
    /* debug_log(kd.str()); */
    
    index = getIndex(kd.depth, kd.x1, kd.x2);
    std::vector<Record>& records = FSDB24[index];

    /* debug_log((int)kd.depth, FSIZE - kd.x1 - 2, (int)fsize, index, kd.str()); */

    records.push_back(Record(kd.pk, std::vector<std::array<std::uint8_t, 3>>()));
    std::vector<std::array<std::uint8_t, 3>>& opes = records.back().second;
    fsize = kd.x2 - kd.x1;
    for(auto v: val) {
      if(v[0] + v[2] <= fsize && v[1] + v[2] - 2 <= fsize) { // hor=Trueの時は縦が2長い
        v[0] = v[0] + kd.x1;
        opes.push_back(v);
      }
    }

    /* debug_log("val:", aopes_str(opes)); */

  }
  debug_log("loaded fsdb");

  for(auto& fsdb24 : FSDB24) {
    std::vector<Record>(fsdb24).swap(fsdb24);
    for(Record& rec : fsdb24){
      std::vector<std::array<std::uint8_t, 3>>(rec.second).swap(rec.second);
    }
  }
  return true;
}

bool PKey::operator==(const PKey& other) {
  return this->value == other.value;
}

void PKey::adaptation(const std::uint8_t X1, const bool& hor,const std::uint8_t fsize) {
  for (auto& p : {std::ref(p1), std::ref(p2), std::ref(p3), std::ref(p4)}) {
    p.get()[0] -= X1;
    if (hor) {
      uint8_t b = p.get()[0];
      p.get()[0] = fsize - p.get()[1] - 1;
      p.get()[1] = b + 2;
    }
  }
}

std::string Key::str() const {
  std::ostringstream oss;
  oss << (int)this->depth << " <" << (int)this->x1 << ", " << (int)this->x2 << "> {";
  oss << (int)pk.p1[0] << ", " << (int)pk.p1[1] << "} {";
  oss << (int)pk.p2[0] << ", " << (int)pk.p2[1] << "} {";
  oss << (int)pk.p3[0] << ", " << (int)pk.p3[1] << "} {";
  oss << (int)pk.p4[0] << ", " << (int)pk.p4[1] << "}";
  return oss.str();
}

bool fsdb::fsdb_init(const char *db_path){
  MDB_env *fs_env = nullptr;
  MDB_dbi fs_dbi  = 0;
  MDB_txn *fs_txn = nullptr;
  int ret = mdb_env_create(&fs_env);

  // 環境を初期化
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_create failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // データベースのパスに環境を設定
  ret = mdb_env_open(fs_env, db_path, MDB_RDONLY, 0664);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }
  

  // トランザクション開始
  ret = mdb_txn_begin(fs_env, nullptr, MDB_RDONLY, &fs_txn);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_txn_begin failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // データベースをオープン
  ret = mdb_dbi_open(fs_txn, nullptr, 0, &fs_dbi);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_dbi_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }
  if(!getCache(fs_dbi, fs_txn)) {
    return false;
  }

  for(auto& ptr : GC) {
    ptr = make_SRoutes_ptr();
  }
  return true;
}

void fsdb::decodeValue(std::vector<std::array<std::uint8_t, 3>>& ret, std::vector<std::uint8_t>& val) {
  if(val.size() / 2 > ret.capacity()) ret.reserve(val.size() / 2);
  ret.clear();

  for(size_t i = 0; i < val.size(); i += 2) {
    ret.push_back({
      static_cast<std::uint8_t>((val[i] >> 3) & 31),
      static_cast<std::uint8_t>(((val[i] & 7)  << 2) | ((val[i+1] >> 6) & 3)),
      static_cast<std::uint8_t>((val[i+1] >> 1) & 31)
    });
  }
}

void fsdb::decodeKey(Key& ret, std::vector<std::uint8_t>& key) {
  ret.depth = static_cast<std::uint8_t>((key[0] >> 4) & 0x0f);
  ret.x1 = static_cast<std::uint8_t>(((key[0] & 0x0f) << 2) | ((key[1] >> 6) & 0x03));
  ret.x2 = static_cast<std::uint8_t>(key[1] & 0x3f);
  ret.pk.p1 = {static_cast<std::uint8_t>(((key[2] >> 3) & 0x1f) + ret.x1), static_cast<std::uint8_t>((key[2] & 0x07) << 2 | ((key[3] >> 6) & 0x03))};
  ret.pk.p2 = {static_cast<std::uint8_t>(((key[3] >> 1) & 0x1f) + ret.x1), static_cast<std::uint8_t>(((key[3] & 0x01) << 4) | ((key[4] >> 4) & 0x0f))};
  ret.pk.p3 = {static_cast<std::uint8_t>((((key[4] & 0x0f) << 1) | ((key[5] >> 7) & 0x01)) + ret.x1), static_cast<std::uint8_t>((key[5] & 0x7f) >> 2)};
  ret.pk.p4 = {static_cast<std::uint8_t>((((key[5] & 0x03) << 3) | ((key[6] >> 5) & 0x07)) + ret.x1), static_cast<std::uint8_t>(key[6] & 0x1f)};
}





Routes fsdb::getOperation(const State* s) {
  if(s->progress <= 0) {
    throw std::invalid_argument("getOperation: progress <= 0");
  }else if(s->progress == s->f.size || s->progress >= s->f.size * 2 - 2){
    return Routes();
  }
  std::uint8_t depth, max_depth = 4;
  std::uint8_t X1, X2, x1, x2, b;
  size_t gcm = 1, endd = 1, i, index;
  GC[0]->f = FsField(s->f);
  GC[0]->ope.n = 0;
  GC[0]->next.clear();
  bool horizon = (s->progress > s->f.size), flag1 = false;
  std::array<std::uint8_t, 3> ope;

  if(horizon) {
    X1 = FSIZE - (s->progress - s->f.size - 1) - 2;
    X2 = FSIZE + s->f.size - (s->progress - s->f.size - 1) - 4;
    GC[0]->tg[0] = static_cast<std::uint8_t>(s->f.size - 2);
    GC[0]->tg[1] = static_cast<std::uint8_t>(s->progress - s->f.size + 1);
  }else {
    X1 = FSIZE - (s->progress - 1) - 2;
    X2 = FSIZE + s->f.size - (s->progress - 1) - 2;
    GC[0]->tg[0] = static_cast<std::uint8_t>(s->progress - 1);
    GC[0]->tg[1] = 0;
  }


  SRoutes_ptr root;
  for(i = 0; i < GC_SIZE; ++i){
    if(i >= endd) {
      --max_depth;
      if(max_depth <= 1) {
        break;
      }else if(gcm <= endd){
        debug_log("Not Found.");
        break;
      }
      endd = gcm;
    }
    /* debug_log("test1: ", (int)max_depth, (int)endd, (int)gcm, (int)i, horizon); */
    root = GC[i];
    flag1 = true;
    // depthのforは最初の一回だけでいい？
    for(depth = 1; depth < max_depth && depth < 4 && flag1; ++depth){

      for(x1 = X1; x1 <= 22; ++x1) {
        for(x2 = X2; 24 <= x2; --x2){
          index = getIndex(depth, x1, x2);
          if (FSDB24[index].empty()) continue;

          /* debug_log("test2:", (int)depth, (int)x1, (int)x2, (int)index); */

          std::vector<Record>& fsdb24 = FSDB24[index];
          for(Record& rec: fsdb24) {
            PKey k = rec.first;
            k.adaptation(X1, horizon, s->f.size);

            if(root->isNext(k.p1, k.p2, k.p3, k.p4)) {
              /* debug_log("p:", (int)k.p1[0], (int)k.p1[1], (int)k.p2[0], (int)k.p2[1], (int)k.p3[0], (int)k.p3[1], (int)k.p4[0], (int)k.p4[1]); */
              for(std::array<std::uint8_t, 3>& _ope : rec.second) {
                ope = _ope;
                ope[0] -= X1;
                if(horizon){
                  b = ope[0];
                  ope[0] = s->f.size - (ope[1] + ope[2] - 1) - 1;
                  ope[1] = b + 2;
                }
                if(!root->inField(ope) || root->inOpe(ope)){
                  /* debug_log("\t", "not in Field", (int)ope[0], (int)ope[1], (int)ope[2]); */
                  continue; 
                }
                /* debug_log("\t", "find:", (int)ope[0], (int)ope[1], (int)ope[2]); */
                root->toNext(GC[gcm], Ope(ope[0], ope[1], ope[2]));
                root->next.push_back(GC[gcm]);
                ++gcm;
                flag1 = false;
              }
            }
          }
        }
      }
    }
    max_depth = depth;
  }

  GC[0]->check();
  return *(GC[0]->toRoutes());

}
