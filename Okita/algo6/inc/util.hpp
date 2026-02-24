#ifndef CPU_UTIL_HPP_
#define CPU_UTIL_HPP_

#include<vector>
#include<stdint.h>
#include<string>

#define SELLS 576
#define HSELLS 288
#define FSIZE 288

// offsets: datasの初めのindex. keyはfrom_index. 
// datas: to_indexのlist(ソート済み)
struct MatrixMap1 {
  std::vector<uint16_t> offsets;
  std::vector<uint16_t> datas;

  MatrixMap1::MatrixMap1(std::string path);

  std::array<int, 3> getOpe(const int x1, const int y1, const int x2, const int y2) const;
  std::array<int, 3> getOpe(const uint16_t fi, const uint16_t ti) const;
  bool canMove(uint16_t fi, uint16_t ti) const;
  std::vector<uint16_t> MatrixMap1::canMoveList(uint16_t fi) const;


};

// offsets: dataの初めのindex. keyはfrom_index * 24 * 24 + to_index
// datas: 経由するfield_indexのlist(ソート済み)
struct MatrixMap2 {
  std::vector<uint32_t> offsets;
  std::vector<uint16_t> datas;

  MatrixMap2::MatrixMap2(std::string path);

};


#endif
