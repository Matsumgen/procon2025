#include <util.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>


bool binarySearch(uint16_t *first, uint16_t *last, uint16_t val) {
  uint16_t *l = last;
  while (first < last) {
    uint16_t *mid = first + ((last - first) >> 1);
    if(*mid < val)  first = mid + 1;
    else            last = mid;
  }
  return (first != l && *first == val);
}

MatrixMap1::MatrixMap1(std::string path) {
  this->offsets.resize(SELLS + 1);
  this->offsets[0] = 0;
  std::ifstream fin1(path, std::ios::binary);
  std::vector<uint16_t> data1;
  if (fin1) {
    fin1.seekg(0, std::ios::end);
    std::streamsize size = fin1.tellg();
    fin1.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    fin1.read(reinterpret_cast<char*>(buffer.data()), size);

    for (size_t i = 0; i + 1 < buffer.size(); i += 2) {
      uint16_t value = (buffer[i] << 8) | buffer[i+1];
      data1.push_back(value);
    }
  } else {
    std::cerr << "Cannot open file: " << path << std::endl;
    return
  }

  size_t i = 0;
  size_t oi = 1;
  while (i < data1.size()) {
    uint16_t fi = data1[i];
    uint16_t length = data1[i+1];

    while(oi <= fi) {
      this->offsets[oi] = this->offsets[oi - 1];
      ++oi;
    }
    this->offsets[oi] = this->offsets[oi-1] + length;
    ++oi;

    i += 2;;
    size_t j = i;
    i += length;
    for (; j < i; ++j) {
      this->datas.push_back(data1[j]);
    }
  }
  this->datas.reserve(this->datas.size());
}


std::array<int, 3> MatrixMap1::getOpe(const int x1, const int y1, const int x2, const int y2) const {
  int cx = x2 - y1 + y2 + x1;
  int cy = y2 + x1 - x2 + y1;
  int N = std::max(std::abs((x1<<1)-cx), std::abs((y1<<1)-cy), std::abs((x2<<1)-cx), std::abs((y2<<1)-cy));
  return {(cx - N)/2, (cy - N)/2, N}
}
std::array<int, 3> MatrixMap1::getOpe(const uint16_t fi, const uint16_t ti) const {
  return this->getOpe(fi%FSIZE, fi/FSIZE, ti%FSIZE, ti/FSIZE);
}

bool MatrixMap1::canMove(uint16_t fi, uint16_t ti) const {
  if(fi < HSELLS) {
    fi = SELLS - fi - 1;
    ti = SELLS - ti - 1;
  }
  
  uint16_t *d = this->datas.data();
  uint16_t *start = d + this->offsets[fi];
  uint16_t *end = d + this->offsets[fi+1];
  return binarySearch(start, end, ti)
}

std::vector<uint16_t> MatrixMap1::canMoveList(uint16_t fi) const {
  bool flag = fi < HSELLS;
  if(flag) { fi = SELLS - fi - 1; }

  std::vector<uint16_t> newVec(this->datas.begin() + this->offset[fi], this->datas.begin() + this->offset[fi+1]);
  if(flag) {
    for(auto& v : newVec) {
      v = SELLS - v - 1;
    }
  }

  return newVec;
}

