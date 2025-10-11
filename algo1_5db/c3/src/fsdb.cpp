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
  std::vector<std::array<std::uint8_t, 3>> opes;
  debug_log("loading fsdb");
  while(mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
    if (key_vec.size() < key.mv_size) key_vec.resize(key.mv_size);
    if (value_vec.size() < data.mv_size) value_vec.resize(data.mv_size);
    std::memcpy(key_vec.data(), key.mv_data, key.mv_size);
    std::memcpy(value_vec.data(), data.mv_data, data.mv_size);
    decodeValue(val, value_vec);
    decodeKey(kd, key_vec);

    if(kd.depth == 0) continue;

    fsize = kd.x2 - kd.x1;
    opes.clear();
    for(auto v: val) {
      if(v[0] + v[2] <= fsize && v[1] + v[2] - 2 <= fsize) { // hor=Trueの時は縦が2長い
        v[0] = v[0] + kd.x1;
        opes.push_back(v);
      }
    }

    if(opes.size() == 0) continue;

    std::uint64_t& k = kd.pk.value;
    if(FSDB24.find(k) == FSDB24.end()) {
      FSDB24[k] = Record{};
    }
    
    index = getIndex(kd.depth, kd.x1, kd.x2);
    if(FSDB24[k].find(index) == FSDB24[k].end()) {
      FSDB24[k][index] = std::vector<std::array<std::uint8_t, 3>>();
      std::vector<std::array<std::uint8_t, 3>>& fopes = FSDB24[k][index];
      fopes.reserve(fopes.size());
      fopes.insert(fopes.end(), opes.begin(), opes.end());
    }else{
      std::vector<std::array<std::uint8_t, 3>>& fopes = FSDB24[k][index];
      fopes.reserve(fopes.size() + opes.size());
      fopes.insert(fopes.end(), opes.begin(), opes.end());
    }



    /* debug_log("val:", aopes_str(opes)); */
  }
  debug_log("loaded fsdb");

  /* PKey pk{}; */
  /* for (auto& [id, record] : FSDB24) { */
  /*   for (auto& [key, vec] : record) { */
  /*     /1* std::vector<std::array<std::uint8_t, 3>>(vec).swap(vec); *1/ */
  /*     pk.value = id; */
  /*     std::cout << pk.str() << "\t" << (int)key << "\t" <<  vec.size() << std::endl; */
  /*   } */
  /* } */
  debug_log("FSDB24 size:", FSDB24.size());

  /* for(int depth = 1; depth < 4; ++depth){ */
  /*   for(int x1 = 0; x1 <= 22; ++x1) { */
  /*     for(int x2 = 24 + x1; 24 <= x2; --x2){ */
  /*       printf("%d %2d %2d %5zu\t%zu\n", depth, x1, x2, getIndex(depth, x1, x2), FSDB24[getIndex(depth, x1, x2)].size()); */
  /*     } */
  /*   } */
  /* } */

  return true;
}

bool PKey::operator==(const PKey& other) {
  return this->value == other.value;
}

void PKey::adaptation(const std::uint8_t X1, const bool& hor,const std::uint8_t fsize) {
  for (auto& p : {std::ref(p1), std::ref(p2), std::ref(p3), std::ref(p4)}) {
    p.get()[0] += X1;
    if (hor) {
      uint8_t b = p.get()[0];
      p.get()[0] = fsize - p.get()[1] - 1;
      p.get()[1] = b + 2;
    }
  }
}

std::array<PKey, 8> PKey::getIsotopes() {
  std::array<PKey, 8> ret;
  ret.fill(*this);
  std::swap(ret[1].p1, ret[1].p2);
  std::swap(ret[2].p3, ret[2].p4);
  std::swap(ret[3].p1, ret[3].p2);
  std::swap(ret[3].p3, ret[3].p4);
  std::swap(ret[4].p1, ret[4].p3);
  std::swap(ret[4].p2, ret[4].p4);
  std::swap(ret[5].p1, ret[5].p4);
  std::swap(ret[5].p2, ret[5].p3);
  std::swap(ret[6].p1, ret[6].p3);
  std::swap(ret[6].p2, ret[6].p4);
  std::swap(ret[6].p3, ret[6].p4);
  std::swap(ret[6].p1, ret[6].p3);
  std::swap(ret[6].p2, ret[6].p4);
  std::swap(ret[6].p1, ret[6].p2);
  return ret;
}

std::string PKey::str() const {
  std::ostringstream oss;
  oss << "PKey: {";
  oss << (int)this->p1[0] << ", " << (int)this->p1[1] << "} {";
  oss << (int)this->p2[0] << ", " << (int)this->p2[1] << "} {";
  oss << (int)this->p3[0] << ", " << (int)this->p3[1] << "} {";
  oss << (int)this->p4[0] << ", " << (int)this->p4[1] << "}";
  return oss.str();
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
  const std::uint8_t fsize = s->f.size;
  if(s->progress <= 0) {
    throw std::invalid_argument("getOperation: progress <= 0");
  }else if(s->progress == fsize || s->progress >= fsize * 2 - 2){
    return Routes();
  }
  GC[0]->f = FsField(s->f);
  GC[0]->ope.n = 0;
  GC[0]->next.clear();
  /* GC[0]->f.print(); */

  const bool horizon = (s->progress > fsize);
  const size_t pairSize = GC[0]->f.pairs.size();
  const std::uint8_t endy = s->progress - 1 - fsize + 2;
  const std::uint8_t endx = s->progress - 1;

  std::uint8_t depth, X1, X2, x1, x2, b, max_depth = 4;
  size_t gcm = 1, endd = 1, i, index;
  std::array<std::uint8_t, 3> ope;
  PKey pk{}, _pk{};

  if(horizon) {
    X1 = FSIZE - endy;
    X2 = FSIZE + fsize - endy - 2;
    GC[0]->tg[0] = static_cast<std::uint8_t>(fsize - 2);
    GC[0]->tg[1] = static_cast<std::uint8_t>(s->progress - fsize + 1);
  }else {
    X1 = FSIZE - endx - 2;
    X2 = FSIZE + fsize - endx - 2;
    GC[0]->tg[0] = endx;
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


    /* debug_log("test1: ", (int)max_depth, (int)endd, (int)gcm, (int)i, horizon, (int)pairSize); */
    root = GC[i];
    /* root->f.print(); */
    FsField& f = root->f;
    for (std::uint16_t i = 0; i < pairSize; ++i) {
      const auto& pi = f.pairs[i];
      _pk.p1[0] = pi[0] % fsize;
      _pk.p1[1] = pi[0] / fsize;
      _pk.p2[0] = pi[1] % fsize;
      _pk.p2[1] = pi[1] / fsize;
      if(horizon){
        if(_pk.p1[1] < 2 || (fsize - 2 < _pk.p1[0] && _pk.p1[1] < endy)) { debug_log("ignore", arr_str(_pk.p1)); break; }
      }else{
        if(_pk.p1[1] < 2 && _pk.p1[0] < endx) { debug_log("ignore", arr_str(_pk.p1)); break; }
      }
      for (std::uint16_t j = i + 1; j < pairSize; ++j) {
        const auto& pj = f.pairs[j];
        _pk.p3[0] = pj[0] % fsize;
        _pk.p3[1] = pj[0] / fsize;
        _pk.p4[0] = pj[1] % fsize;
        _pk.p4[1] = pj[1] / fsize;
        if(horizon){
          if(_pk.p3[1] < 2 || (fsize - 2 < _pk.p3[0] && _pk.p3[1] < endy)) { debug_log("ignore", arr_str(_pk.p3)); break; }
        }else{
          if(_pk.p3[1] < 2 && _pk.p3[0] < endx) { debug_log("ignore", arr_str(_pk.p3)); break; }
        }

        /* debug_log(_pk.str()); */

        pk = _pk;
        pk.adaptation(X1, horizon, fsize);
        
        for(auto& pkiso : pk.getIsotopes()) {
          if(FSDB24.find(pk.value) != FSDB24.end()){
            pk = pkiso;
            break;
          }
        }

        if(FSDB24.find(pk.value) == FSDB24.end()) continue;
        /* debug_log("find:", pk.str()); */

        Record& records = FSDB24[pk.value];
        for(depth = 1; depth < max_depth; ++depth){
          for(x1 = X1; x1 <= 22; ++x1) {
            for(x2 = X2; 24 <= x2; --x2){
              index = getIndex(depth, x1, x2);
              if(records.find(index) == records.end()) continue;
              for(auto& _ope : records[index]){
                ope = _ope;
                ope[0] -= X1;
                if(horizon){
                  b = ope[0];
                  ope[0] = fsize - (ope[1] + ope[2] - 1) - 1;
                  ope[1] = b + 2;
                }
                if(!root->inField(ope) || root->inOpe(ope)){
                  /* debug_log("\t", "not in Field", (int)ope[0], (int)ope[1], (int)ope[2]); */
                  continue; 
                }
                /* debug_log("\t", "find:", (int)depth, (int)x1, (int)x2, (int)ope[0], (int)ope[1], (int)ope[2]); */
                root->toNext(GC[gcm], Ope(ope[0], ope[1], ope[2]));
                root->next.push_back(GC[gcm]);
                ++gcm;
              }
            }
          }
        }
      }
    }
  }

  GC[0]->check();
  return *(GC[0]->toRoutes());

}
