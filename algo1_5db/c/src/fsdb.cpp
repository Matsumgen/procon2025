/* #define DEBUG */

#include <fsdb.hpp>
#include <lmdb.h>
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <bitset>

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


using namespace fsdb;


std::string Key::str() const {
  std::ostringstream oss;
  oss << (int)this->depth << " <" << (int)this->x1 << ", " << (int)this->x2 << "> {";
  oss << (int)p1[0] << ", " << (int)p1[1] << "} {";
  oss << (int)p2[0] << ", " << (int)p2[1] << "} {";
  oss << (int)p3[0] << ", " << (int)p3[1] << "} {";
  oss << (int)p4[0] << ", " << (int)p4[1] << "}";
  return oss.str();
}

bool Key::correction(std::uint8_t fx1, bool horizon, std::uint8_t fsize) {
  // fiels_size == 16の時
  this->p1[0] -= fx1;
  this->p2[0] -= fx1;
  this->p3[0] -= fx1;
  this->p4[0] -= fx1;

  /* this->p1[0] = this->p1[0] + this->x1 - fx1; */
  /* this->p2[0] = this->p2[0] + this->x1 - fx1; */
  /* this->p3[0] = this->p3[0] + this->x1 - fx1; */
  /* this->p4[0] = this->p4[0] + this->x1 - fx1; */

  if(horizon) {
    std::uint8_t b = this->p1[0];
    this->p1[0] = fsize - this->p1[1] - 1;
    this->p1[1] = b + 2;
    b = this->p2[0];
    this->p2[0] = fsize - this->p2[1] - 1;
    this->p2[1] = b + 2;
    b = this->p3[0];
    this->p3[0] = fsize - this->p3[1] - 1;
    this->p3[1] = b + 2;
    b = this->p4[0];
    this->p4[0] = fsize - this->p4[1] - 1;
    this->p4[1] = b + 2;
  }
  return p1[0] < fsize && p1[1] < fsize && p2[0] < fsize && p2[1] < fsize && p3[0] < fsize && p3[1] < fsize && p4[0] < fsize && p4[1] < fsize;
}

bool fsdb::fsdb_init(const char *db_path, std::uint8_t fsize){
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

  field_size = fsize;
  return true;
}


void fsdb::fsdb_deinit(){
  if (fs_txn) { mdb_txn_abort(fs_txn); }
  if (fs_dbi) { mdb_dbi_close(fs_env, fs_dbi); }
  if (fs_env) { mdb_env_close(fs_env); }
  field_size = 0;
}


std::vector<Ope> fsdb::decodeValue(std::vector<std::uint8_t>& val) {
  std::vector<Ope> ret;
  ret.reserve(val.size() / 2);
  for(size_t i = 0; i < val.size(); i += 2) {
    ret.push_back(Ope( (val[i] >> 3) & 31, ((val[i] & 7)  << 2) | ((val[i+1] >> 6) & 3), (val[i+1] >> 1) & 31));
  }
  return ret;
}

Key fsdb::decodeKey(std::vector<std::uint8_t>& key) {
  Key ret{};
  ret.depth = static_cast<std::uint8_t>((key[0] >> 4) & 0x0f);
  ret.x1 = static_cast<std::uint8_t>(((key[0] & 0x0f) << 2) | ((key[1] >> 6) & 0x03));
  ret.x2 = static_cast<std::uint8_t>(key[1] & 0x3f);
  ret.p1 = {static_cast<std::uint8_t>(((key[2] >> 3) & 0x1f) + ret.x1), static_cast<std::uint8_t>((key[2] & 0x07) << 2 | ((key[3] >> 6) & 0x03))};
  ret.p2 = {static_cast<std::uint8_t>(((key[3] >> 1) & 0x1f) + ret.x1), static_cast<std::uint8_t>(((key[3] & 0x01) << 4) | ((key[4] >> 4) & 0x0f))};
  ret.p3 = {static_cast<std::uint8_t>((((key[4] & 0x0f) << 1) | ((key[5] >> 7) & 0x01)) + ret.x1), static_cast<std::uint8_t>((key[5] & 0x7f) >> 2)};
  ret.p4 = {static_cast<std::uint8_t>((((key[5] & 0x03) << 3) | ((key[6] >> 5) & 0x07)) + ret.x1), static_cast<std::uint8_t>(key[6] & 0x1f)};
  return ret;
}

void setStartKey(MDB_val& key, const std::uint8_t a, const std::uint8_t b, const std::uint8_t c) {
  // 入力の範囲チェック
  if (a >= (1 << 4)) throw std::invalid_argument("a must be a 4-bit value (0–15)");
  if (b >= (1 << 6)) throw std::invalid_argument("b must be a 6-bit value (0–63)");
  if (c >= (1 << 6)) throw std::invalid_argument("c must be a 6-bit value (0–63)");

  // 4 + 6 + 6 = 16ビットを作成し、残りは0で埋める（上位詰め）
  uint64_t key_bits = 0;
  key_bits |= static_cast<uint64_t>(a & 0xF) << 12;
  key_bits |= static_cast<uint64_t>(b & 0x3F) << 6;
  key_bits |= static_cast<uint64_t>(c & 0x3F);

  key_bits <<= (56 - 16); // 残り40bitを0埋め

  // メモリ確保（7バイト）
  uint8_t* buffer = new uint8_t[7];

  // ビッグエンディアンで書き込む
  for (int i = 0; i < 7; ++i) {
      buffer[i] = static_cast<uint8_t>((key_bits >> ((6 - i) * 8)) & 0xFF);
  }

  /* debug_log("startKey", (int)a, (int)b, (int)c, bitset<8>(buffer[0]), bitset<8>(buffer[1])); */
  // MDB_val に設定
  key.mv_size = 7;
  key.mv_data = buffer;
}

void fsdb::getAllOperation(SRoutes_ptr r, const std::uint8_t& x1, const std::uint8_t& x2, const std::uint8_t deep, bool horizon) {
  if(deep == 0) {
    return;
  }
  debug_log("deep =", (int)deep, "x1 =", (int)x1, "x2 =", (int)x2, "hor =", horizon);
  MDB_cursor* cursor;
  MDB_val key, data, start_key;
  std::vector<std::uint8_t> key_vec(7);
  std::vector<std::uint8_t> value_vec;
  std::vector<Ope> opes, _opes;
  std::uint8_t nowdepth = 1;
  Key kd;
  bool endflag = false;
  if (mdb_cursor_open(fs_txn, fs_dbi, &cursor) != 0) {
    std::cerr << "Failed to open cursor" << std::endl;
    return;
  }

  // 初めの一回かどうか
  bool first = (r->next.size() == 0);

  setStartKey(start_key, 1, x1, 0);
  int rc = mdb_cursor_get(cursor, &start_key, &data, MDB_SET_RANGE);
  if (rc == MDB_NOTFOUND) {
    std::cout << "Key not found in database.\n";
    return;
  } else if (rc != 0) {
    std::cerr << "Cursor get failed: " << mdb_strerror(rc) << "\n";
  }
  key = start_key;
  do{
    if (key_vec.size() < key.mv_size) key_vec.resize(key.mv_size);
    std::memcpy(key_vec.data(), key.mv_data, key.mv_size);
    kd = decodeKey(key_vec);
    /* debug_log(bitset<8>(key_vec[0]), bitset<8>(key_vec[1]), (int)x1, (int)x2, kd.str()); */

    if(kd.depth > nowdepth) {
      if(endflag) break;
      nowdepth = kd.depth;
      setStartKey(start_key, nowdepth, x1, 0);
      int rc = mdb_cursor_get(cursor, &start_key, &data, MDB_SET_RANGE);
      if (rc == MDB_NOTFOUND) {
        std::cout << "Key not found in database.\n";
        break;
      } else if (rc != 0) {
        std::cerr << "Cursor get failed: " << mdb_strerror(rc) << "\n";
      }
      key = start_key;
    }

    /* if(horizon )debug_log((int)x1, (int)x2, kd.str()); */
    if(kd.x1 < x1 || x2 < kd.x2) continue;
    if(kd.depth > deep) break;

    if(!kd.correction(x1, horizon, r->f.size)) { /* p1 ~ p4の補正 */
      continue;
    }
    /* if(horizon)debug_log("    ", kd.str()); */

    if (value_vec.size() < data.mv_size) value_vec.resize(data.mv_size);
    std::memcpy(value_vec.data(), data.mv_data, data.mv_size);

    _opes = decodeValue(value_vec);
    /* opesの補正 */
    opes.clear();
    for(auto& ope: _opes) {
      ope.x = ope.x + kd.x1 - x1;
      /* ope.x = ope.x - x1; // fiels_size = 16の時 */
      if(horizon) {
        int b = ope.x;
        ope.x = r->f.size - (ope.y + ope.n - 1) - 1;
        ope.y = b + 2;
      }
      if(ope.x < 0 || ope.y < 0 || ope.x + ope.n > r->f.size || ope.y + ope.n > r->f.size) continue;
      opes.push_back(ope);
    }
    if(opes.size() == 0) continue;

    if(first){
      if(r->isNext(kd.p1, kd.p2, kd.p3, kd.p4)) {
        debug_log(kd.str(), opes_str(opes));
        for(auto& ope: opes){
          if(r->inOpe(ope)) continue;
          SRoutes_ptr next = make_SRoutes_ptr();
          r->toNext(next, ope);
          r->next.push_back(next);
        }
        endflag = true;
      }
    }else{
      for(SRoutes_ptr& ptr : r->next) {
        if(ptr->isNext(kd.p1, kd.p2, kd.p3, kd.p4)){
          debug_log(kd.str(), opes_str(opes));
          for(auto& ope: opes){
            if(ptr->inOpe(ope)) continue;
            SRoutes_ptr next = make_SRoutes_ptr();
            ptr->toNext(next, ope);
            ptr->next.push_back(next);
          }
          endflag = true;
        }
      }
    }

  }while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0);
  mdb_cursor_close(cursor);

  if(nowdepth > 0){
    /* debug_log("depth: ", (int)nowdepth, "next_length:", r->next.size()); */
    if(first) {
      if(r->next.size() != 0) getAllOperation(r, x1, x2, nowdepth - 1, horizon);
    } else{
      for(size_t i = 0; i < r->next.size();){
        if(r->next[i]->next.size() == 0) {
          r->next.erase(r->next.begin() + i);
        }else{
          getAllOperation(r->next[i], x1, x2, nowdepth - 1, horizon);
          if(r->next[i]->next.size() == 0){ r->next.erase(r->next.begin() + i); }
          else ++i;
        }
      }
    }
  }
}

Routes fsdb::getOperation(const State* s) {
  if(s->progress <= 0) {
    throw std::invalid_argument("getOperation: progress <= 0");
  }else if(s->progress > s->f.size * 2 - 2){
    std::cout << "getOperation: progress > f.size * 2 - 2" << std::endl;
    return Routes();
  }
  SRoutes_ptr root = make_SRoutes_ptr();
  root->f = FsField(s->f);
  std::uint8_t x1, x2;
  bool horizon = (s->progress > s->f.size);

  if(horizon) {
    root->tg = {static_cast<std::uint8_t>(s->f.size - 2), static_cast<std::uint8_t>(s->progress - s->f.size + 1)};
    x1 = field_size - (s->progress - s->f.size - 1) - 2;
    x2 = field_size + s->f.size - (s->progress - s->f.size - 1) - 4;
  }else {
    root->tg = {static_cast<std::uint8_t>(s->progress - 1), 0};
    x1 = field_size - (s->progress - 1) - 2;
    x2 = field_size + s->f.size - (s->progress - 1) - 2;
  }
  debug_log(s->progress, s->f.size, "(x1 x2) =", (int)x1, (int)x2, "horizon =", horizon);
  getAllOperation(root, x1, x2, 4, horizon);
  root->check();
  return *(root->toRoutes());
}
