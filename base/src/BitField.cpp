#include <BitField.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <random>


using namespace bf;

BitField BitField::loadCsv(const std::string& path) {
  std::ifstream file(path);
  if (!file) throw std::runtime_error("Can't open file.");

  std::vector<std::uint16_t> data;
  data.reserve(576);

  std::string line, cell;
  std::uint8_t size = 0;
  size_t nc, pos;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    pos = 0;
    while (pos < line.size()) {
      nc = line.find(',', pos);
      if (nc == std::string::npos) nc = line.size();

      cell = line.substr(pos, nc - pos);
      std::uint16_t val = std::stoi(cell);

      data.push_back(val);
      pos = nc + 1;
    }
    ++size;
  }

  return BitField(std::move(data), size);
}

BitField BitField::randomField(const std::uint8_t size) {
  const std::uint16_t num_size = size * size / 2;
  std::vector<std::uint16_t> data;
  data.reserve(size * size);
  for(size_t i = 0; i < num_size; ++i){
    data.push_back(i);
    data.push_back(i);
  }

  static thread_local std::mt19937 rng(std::random_device{}());
  std::shuffle(data.begin(), data.end(), rng);

  return BitField(std::move(data), size);
}


Point::Point(const std::uint8_t x, const std::uint8_t y) : x(x), y(y) {}
Point::Point(const std::uint16_t i, const std::uint8_t siz) : x(i%siz), y(i/siz) {}

bool Point::operator==(const Point& other) const {
  return this->x == other.x && this->y == other.y;
}

bool Point::operator<(const Point& other) const {
  return (x < other.x) || (x == other.x && y < other.y);
}

std::array<std::uint8_t, 2> Point::toArr() const {
  std::array<std::uint8_t, 2> arr{this->x, this->y};
  return arr;
}
std::uint16_t Point::toi(const std::uint8_t siz) const {
  return this->x + this->y * siz;
}

BitField::BitField(const std::vector<std::uint16_t> f, const std::uint8_t size)
: field(std::move(f)), size(size) {
  if(size&1){
    throw std::invalid_argument("Invalid size:" + std::to_string(size));
  }
}

// fieldを表示
// 同じ数字が隣り合っている時は文字色をシアンにする
// confirmの時は背景色を黄色にする
void BitField::print() const { this->print(' '); }
void BitField::print(const char sep) const {
  std::cout << "BitField: field" << std::endl;
  for(size_t i=0; i < this->field.size(); ++i){
    if(isIndexAdjacent(i))
      std::cout << "\x1b[36m" << this->field[i] << "\x1b[m";
    else
      std::cout << this->field[i];
    
    if((i+1)%this->size == 0)
      std::cout << std::endl;
    else
      std::cout << sep;
  }
}

std::vector<std::uint16_t> BitField::getField() const { return this->field; }
std::uint8_t BitField::getSize() const { return this->size; }
std::uint16_t BitField::get(const std::uint16_t i) const { return this->field[i]; }
std::uint16_t BitField::get(const std::uint8_t x, const std::uint8_t y) const {
  if(this->inField(x, y)) return this->get(x + y * this->size);
  std::ostringstream oss;
  oss << "Out of range: (x, y) = (" << x << ", " << y << ")";
  throw std::invalid_argument(oss.str());
};


bool BitField::inField(const std::uint8_t x, const std::uint8_t y) const{
  if(x < this->size && y < this->size)  return true;
  return false;
}

bool BitField::isEnd() const{
  for(std::uint16_t i=0; i < this->field.size(); ++i){
    if(!this->isIndexAdjacent(i)){ return false; }
  }
  return true;
}

// indexの数字と同じ数字が隣接していたらtrue
bool BitField::isIndexAdjacent(const std::uint16_t i) const {
  std::uint16_t num = this->field[i];
  const size_t x = i % this->size;

  const size_t up    = i >= this->size                          ? i - this->size  : FIELD_SIZE_MAX;
  const size_t down  = i + this->size < this->size * this->size ? i + this->size  : FIELD_SIZE_MAX;
  const size_t left  = x != 0                                   ? i - 1           : FIELD_SIZE_MAX;
  const size_t right = x != this->size-1                        ? i + 1           : FIELD_SIZE_MAX;

  return (up    != FIELD_SIZE_MAX && field[up]    == num) ||
         (down  != FIELD_SIZE_MAX && field[down]  == num) ||
         (left  != FIELD_SIZE_MAX && field[left]  == num) ||
         (right != FIELD_SIZE_MAX && field[right] == num);

}
bool BitField::isIndexAdjacent(const std::uint8_t x, const std::uint8_t y) const{
  return this->isIndexAdjacent(x + y * this->size);
}

// fromからtoへの移動が可能かを判定
bool BitField::toPointCheck(const Point from, const Point to, Ope& buf) const {
  bool cond1 = (from.x < to.x);
  bool cond2 = (from.y <= to.y);
  bool cond3 = (to.x <= from.x);
  bool cond4 = (from.y < to.y);
  std::uint8_t X = cond1 ? (to.x - from.x) : (from.x - to.x);
  std::uint8_t Y = cond4 ? (to.y - from.y) : (from.y - to.y);
  std::uint8_t siz = X + Y + 1;
  buf[2] = siz;

  if (cond1 && cond2) {
    buf[0] = from.x - Y;
    buf[1] = from.y;
  } else if (cond3 && cond4) {
    buf[0] = to.x - Y;
    buf[1] = from.y - X;
  } else if (cond1 && !cond2) {
    buf[0] = from.x;
    buf[1] = to.y;
  } else {
    buf[0] = to.x;
    buf[1] = to.y - X;
  }

  return this->inField(buf[0], buf[1]) && this->inField(buf[0] + siz - 1, buf[1] + siz - 1);
}
bool BitField::toPointCheck(const Point from, const Point to) const {
  Ope ope;
  return this->toPointCheck(from, to, ope);
}

bool BitField::toPoint(const Point from, const Point to) {
  Ope buf;
  if(this->toPointCheck(from, to, buf)){
    this->rotate(buf);
    return true;
  }
  return false;
}

void BitField::rotate(const std::uint8_t x, const std::uint8_t y, const std::uint8_t siz) {
  if(!this->inField(x, y) || !this->inField(x + siz - 1, y + siz - 1)){
    throw std::invalid_argument("rotate: out of range");
  }
  std::uint8_t siz_half = siz >> 1;

  std::uint8_t h, w;
  std::uint16_t i0, i1, i2, i3, buf;
  for(h = 0; h < siz_half; ++h) {
    i0 = x + (y+h) * this->size;
    i1 = x + (siz - 1 - h) + y * this->size;
    i2 = (x + (siz - 1)) + (y + (siz - 1 - h)) * this->size;
    i3 = (x + h) + (y + (siz - 1)) * this->size;
    for(w = 0; w < siz_half; ++w){
      buf = this->field[i0];
      this->field[i0] = this->field[i3];
      this->field[i3] = this->field[i2];
      this->field[i2] = this->field[i1];
      this->field[i1] = buf;

      ++i0;
      i1 += this->size;
      --i2;
      i3 -= this->size;
    }
  }

  if(siz & 1){
    i0 = x+siz_half+1 + (y+h) * this->size;
    i1 = x + (siz - 1 - h) + (y + siz_half + 1) * this->size;
    i2 = x + (siz - 2 - siz_half) + (y + siz - 1 - h) * this->size;
    i3 = x + h + (y + siz - 2 - siz_half) * this->size;
    for(h = 0; h < siz_half; ++h) {
      buf = this->field[i0];
      this->field[i0] = this->field[i3];
      this->field[i3] = this->field[i2];
      this->field[i2] = this->field[i1];
      this->field[i1] = buf;
      i0 += this->size;
      --i1;
      i2 -= this->size;
      ++i3;
    }
  }
}

void BitField::rotate(const Ope ope){
  this->rotate(ope[0], ope[1], ope[2]);
}

// 数字の置き換え対応の取得
// map[前] = 後
std::vector<std::uint16_t> BitField::reallocationMap() const {
  std::vector<std::uint16_t> dic(this->size * this->size / 2, FIELD_NUM_SIZE_MAX);
  std::uint16_t count = 0;
  for(size_t i = 0; i < this->field.size(); ++i) {
    std::uint16_t num = this->field[i];
    if(dic[num] == FIELD_NUM_SIZE_MAX){
      dic[num] = count++;
    }
  }
  return dic;
}


// 数字の振り直し
void BitField::reallocation() {
  std::vector<std::uint16_t> dic(this->size * this->size / 2, FIELD_NUM_SIZE_MAX);
  std::uint16_t num, count = 0;
  for(size_t i = 0; i < this->field.size(); ++i){
    num = this->field[i];
    if(dic[num] == FIELD_NUM_SIZE_MAX) {
      dic[num] = count;
      this->field[i] = count++;
    }else{
      this->field[i] = dic[num];
    }
  }
}


// vec[置き換え後] = 置き換え前
std::vector<std::uint16_t> BitField::reallocationD() {
  std::vector<std::uint16_t> dic(this->size * this->size / 2, FIELD_NUM_SIZE_MAX);
  std::vector<std::uint16_t> dic_rev(this->size * this->size / 2, FIELD_NUM_SIZE_MAX);
  std::uint16_t count = 0;
  std::uint16_t num;
  for(size_t i = 0; i < this->field.size(); ++i){
    num = this->field[i];
    if(dic_rev[num] == FIELD_NUM_SIZE_MAX){
      dic_rev[num] = count;
      dic[count] = num;
      this->field[i] = count++;
    }else{
      this->field[i] = dic_rev[num];
    }
  }
  return dic;
}




// サイズがsizeのFiledのある点iをopeに従い90度右回転させた時の点の座標(インデックス)を返す
std::uint16_t bf::rotatePointIndex(const std::uint16_t i, const uint8_t size, const Ope ope) {
  return rotatePointIndex(i, size, ope[0], ope[1], ope[2]);
}
std::uint16_t bf::rotatePointIndex(std::uint16_t i, std::uint8_t size, std::uint8_t x, std::uint8_t y, std::uint8_t n) {
  std::uint8_t a = i % size;
  std::uint8_t b = i / size;

  // 範囲外ならそのまま返す
  if (a < x || a >= x + n || b < y || b >= y + n) return i;

  return x + (n - 1 - b + y) + (y + a - x) * size;
}

Point bf::rotatePoint(const Point p, const Ope ope) {
  return rotatePoint(p, ope[0], ope[1], ope[2]);
}
Point bf::rotatePoint(const Point p, const std::uint8_t x, const std::uint8_t y, const std::uint8_t n) {
  if (p.x < x || p.x >= x + n || p.y < y || p.y >= y + n) return p;
  return Point(static_cast<std::uint8_t>(x + n - 1 - p.y + y), y + p.x - x);
}
