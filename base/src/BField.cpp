#include <BitField.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <fstream>

using namespace bf;


BField BField::loadCsv(const std::string& path) {
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

  return BField(std::move(data), size);

}

// コンストラクタ
BField::BField(const std::vector<std::uint16_t> f, const std::uint8_t size)
: RBField(f, size), confirm(std::vector<bool>(size*size, false)) { }

BField::BField(BitField& f) : BField(f.getField(), f.getSize()) { }
BField::BField(BitField&& f) : BField(f.getField(), f.getSize()) { }

// コピーコンストラクタ
BField::BField(BField& f) : RBField(f), confirm(f.confirm) { }

// コピー代入演算子
BField& BField::operator=(BField& f) {
  if (this == &f) return *this;  // 自己代入チェック

  BitField::operator=(f);       // 基底クラスの代入演算子呼び出し
  this->pent = f.pent;
  this->confirm = f.confirm;

  if (f.operate->operate.size() != 0) {
    this->operate = std::make_shared<OperateHist>();
    this->operate->before = f.operate;
    f.operate = std::make_shared<OperateHist>();
    f.operate->before = this->operate->before;
  } else {
    this->operate = f.operate->clone();
  }

  return *this;
}

// そのうちconfirmがtrueの時背景色を変える
void BField::print(const char sep, const bool show_pair) const {
  std::cout << "BField: field" << std::endl;
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

void BField::setConfirm(const std::uint16_t i) { this->confirm[i] = true; }
void BField::setConfirm(const std::uint8_t x, const std::uint8_t y){ this->setConfirm(x + y*this->size); }
void BField::setConfirm(const Point p) { this->setConfirm(p.toi(this->size)); }

void BField::unsetConfirm(const std::uint16_t i) { this->confirm[i] = false; }
void BField::unsetConfirm(const std::uint8_t x, const std::uint8_t y){ this->unsetConfirm(x + y*this->size); }
void BField::unsetConfirm(const Point p) { this->unsetConfirm(p.toi(this->size)); }

bool BField::isConfirm(const std::uint16_t i) const { return this->confirm[i]; }
bool BField::isConfirm(const std::uint8_t x, const std::uint8_t y) const { return this->isConfirm(x + y*this->size); }
bool BField::isConfirm(const Point p) const { return this->isConfirm(p.toi(this->size)); }

