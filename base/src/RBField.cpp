#include <BitField.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <fstream>

using namespace bf;

PENT::PENT() : p1(PENT_NULL), p2(PENT_NULL) {}
PENT::PENT(const std::uint16_t p1, const std::uint16_t p2) : p1(p1), p2(p2) {}

bool OperateHist::operator <(const OperateHist& oh) const {
  return this->size() < oh.size();
}
bool OperateHist::operator >(const OperateHist& oh) const {
  return this->size() < oh.size();
}

std::uint16_t OperateHist::size() const {
  if(this->before) {
    return this->before->size() + this->operate.size();
  }else{
    return this->operate.size();
  }
}

void OperateHist::push_back(Ope ope) {
  this->operate.push_back(ope);
}

void OperateHist::shrink_to_fit() {
  std::vector<Ope>(this->operate).swap(this->operate);
}

std::vector<Ope> OperateHist::getOperate() const {
  if(this->before){
    std::vector<Ope> o = this->before->getOperate();
    o.reserve(o.size() + this->operate.size());
    o.insert(o.end(), this->operate.begin(), this->operate.end());
    return o;
  }else {
    return this->operate;
  }
}

std::shared_ptr<OperateHist> OperateHist::clone() const {
  return std::make_shared<OperateHist>(*this);
}






RBField RBField::loadCsv(const std::string& path) {
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

  return RBField(std::move(data), size);

}

// コンストラクタ
RBField::RBField(const std::vector<std::uint16_t> f, const std::uint8_t size)
: BitField(f, size) {
  std::uint16_t num_size = f.size() / 2;
  this->operate = std::make_shared<OperateHist>();

  std::uint16_t num;
  this->pent = std::vector<PENT>(num_size);
  for(std::uint16_t i = 0; i < this->field.size(); ++i){
    num = this->field[i];
    if(this->pent[num].p1 == PENT_NULL){
      this->pent[num].p1 = i;
    }else{
      this->pent[num].p2 = i;
    }
  }

}
RBField::RBField(const std::vector<std::uint16_t> f, const std::uint8_t size, std::vector<PENT> pent, std::shared_ptr<OperateHist> operate)
: BitField(f, size), pent(pent), operate(operate) { }

RBField::RBField(BitField& f) : RBField(f.getField(), f.getSize()) { }
RBField::RBField(BitField&& f) : RBField(f.getField(), f.getSize()) { }

// コピー関数
RBField RBField::copy() {
  std::shared_ptr<OperateHist> new_ope = std::make_shared<OperateHist>();
  new_ope->before = this->operate;
  this->operate = std::make_shared<OperateHist>();
  this->operate->before = new_ope->before;

  RBField f(this->field, this->size, this->pent, new_ope);

  return f;
}

void RBField::print() const { this->print(' ', false); }
void RBField::print(const char sep) const { this->print(sep, false); }
void RBField::print(const bool show_pair) const { this->print(' ', show_pair); }
void RBField::print(const char sep, const bool show_pair) const {
  std::cout << "RBField: field" << std::endl;
  std::cout << "Operate history size: " << this->operate->size() << std::endl;
  BitField::print(sep);
  if(show_pair){
    std::cout << "Pair entities:" << std::endl;
    for(size_t i = 0; i < this->pent.size(); ++i) {
      std:: cout << i << ": (" << this->pent[i].p1 << ", " << this->pent[i].p2 << ")\t";
      if(i != 0 && i%5 == 4)  std::cout << std::endl;
    }
  }
}


std::vector<std::array<std::uint8_t, 3>> RBField::getOperate() const {
  return this->operate->getOperate();
}
std::vector<std::string> RBField::getAnswer() const {
  std::vector<std::string> ret;
  ret.reserve(this->operate->size());
  for(auto& ope : this->operate->getOperate()){
    std::ostringstream oss;
    oss << "{" << static_cast<int>(ope[0]) << ", " << static_cast<int>(ope[1]) << ", " << static_cast<int>(ope[2]) << "}";
    ret.push_back(oss.str());
  }
  return ret;
}

// 盤面が終了条件を満たしているかを評価
bool RBField::isEnd() const {
  const uint16_t num_len = this->size * this->size / 2;
  for(uint16_t i=0; i < num_len; ++i)
    if(!this->isNumAdjacent(i)) return false;
  return true;
}

// 要素を指定し、その要素が隣接しているかを評価
bool RBField::isNumAdjacent(const std::uint16_t num) const {
  const std::uint16_t p1 = pent[num].p1;
  const std::uint16_t p2 = pent[num].p2;

  const int dx = static_cast<int>(p1 % this->size) - static_cast<int>(p2 % this->size);
  const int dy = static_cast<int>(p1 / this->size) - static_cast<int>(p2 / this->size);
  return (dx * dx + dy * dy == 1);
}

PENT RBField::getPent(const std::uint16_t num) const {
  return this->pent[num];
}

Point RBField::getPair(const Point p) const {
  return Point(this->getPairIndex(p.toi(this->size)), this->size);
}
Point RBField::getPair(const std::uint8_t x, const std::uint8_t y) const {
  return this->getPair(Point(x, y));
}

std::uint16_t RBField::getPairIndex(const std::uint16_t index) const {
  PENT pen = this->pent[this->field[index]];
  return (pen.p1 == index) ? pen.p2 : pen.p1;
}
void RBField::rotate(const std::uint8_t x, const std::uint8_t y, const std::uint8_t siz) {
  if(!this->inField(x, y) || !this->inField(x + siz - 1, y + siz - 1)){
    throw std::invalid_argument("rotate: out of range");
  }

  auto updatePent = [&](std::uint16_t num, std::uint16_t bi, std::uint16_t ai){
    std::uint16_t *p = this->pent[num].p1 == bi ? &(this->pent[num].p1) : &(this->pent[num].p2);
    *p = ai;
  };

  std::uint8_t siz_half = siz >> 1;
  std::uint8_t h, w;
  std::uint16_t i0, i1, i2, i3, buf;
  for(h = 0; h < siz_half; ++h) {
    i0 = x + (y+h) * this->size;
    i1 = x + (siz - 1 - h) + y * this->size;
    i2 = (x + (siz - 1)) + (y + (siz - 1 - h)) * this->size;
    i3 = (x + h) + (y + (siz - 1)) * this->size;
    for(w = 0; w < siz_half; ++w){
      updatePent(this->field[i3], i3, i0);
      updatePent(this->field[i2], i2, i3);
      updatePent(this->field[i1], i1, i2);
      updatePent(this->field[i0], i0, i1);
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
    i0 = x+siz_half + (y+0) * this->size;
    i1 = x + (siz - 1 - 0) + (y + siz_half) * this->size;
    i2 = x + (siz - 1 - siz_half) + (y + siz - 1 - 0) * this->size;
    i3 = x + 0 + (y + siz - 1 - siz_half) * this->size;
    for(h = 0; h < siz_half; ++h) {
      buf = this->field[i0];
      updatePent(this->field[i3], i3, i0);
      updatePent(this->field[i2], i2, i3);
      updatePent(this->field[i1], i1, i2);
      updatePent(this->field[i0], i0, i1);
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

  Ope o{x, y, siz};
  this->operate->push_back(o);
}
void RBField::rotate(const Ope ope) {
  this->rotate(ope[0], ope[1], ope[2]);
}

