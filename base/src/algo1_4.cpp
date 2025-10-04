#define DEBUG_ALGO1_4
#include <algo1_4.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <random>
#include <unordered_set>
#include <tuple>

using namespace algo1_4;

GCRank::GCRank(VHField& f, std::uint16_t rank) : f(f), rank(rank) {}
GCRank::GCRank(VHField&& f, std::uint16_t rank): f(f), rank(rank) {};


VHField VHField::loadCsv(const std::string& path) {
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

  return VHField(std::move(data), size, size);
}

VHField::VHField(const std::vector<std::uint16_t> f, const std::uint8_t x_size, const std::uint8_t y_size)
: RBField(f, x_size), size_y(y_size) { }

VHField::VHField(const std::vector<std::uint16_t> f, const std::uint8_t size, const std::uint8_t size_y, std::vector<bf::PENT> pent, std::shared_ptr<OperateHist> operate)
: RBField(f, size, pent, operate), size_y(size_y) { }

void VHField::print() const {
  std::cout << "VHField size: (x, y) = (" << (int)this->size << ", " << (int)this->size_y << ")" << std::endl;
  RBField::print(' ');
}

bool VHField::inField(const std::uint8_t x, const std::uint8_t y) const {
  return x < this->size && y < this->size_y;

}

VHField VHField::copy() {
  std::shared_ptr<OperateHist> new_ope = std::make_shared<OperateHist>();
  new_ope->before = this->operate;
  this->operate = std::make_shared<OperateHist>();
  this->operate->before = new_ope->before;
  VHField f(this->field, this->size, this->size_y, this->pent, new_ope);
  return f;
}


bool VHField::isIndexAdjacent(const std::uint16_t i) const {
  std::uint16_t num = this->field[i];
  const size_t x = i % this->size;

  const size_t up    = i >= this->size                            ? i - this->size  : FIELD_SIZE_MAX;
  const size_t down  = i + this->size < this->size * this->size_y ? i + this->size  : FIELD_SIZE_MAX;
  const size_t left  = x != 0                                     ? i - 1           : FIELD_SIZE_MAX;
  const size_t right = x != this->size - 1                        ? i + 1           : FIELD_SIZE_MAX;

  return (up    != FIELD_SIZE_MAX && field[up]    == num) ||
         (down  != FIELD_SIZE_MAX && field[down]  == num) ||
         (left  != FIELD_SIZE_MAX && field[left]  == num) ||
         (right != FIELD_SIZE_MAX && field[right] == num);

}

bool VHField::isProblem() const {
  std::unordered_set<std::uint16_t> a;
  for(std::uint16_t b : this->field) a.insert(b);
  /* for(auto c : a) std::cout << (int)c << " "; */
  /* std::cout << std::endl; */
  return a.size() == this->size * this->size_y / 2;
 
}


std::uint8_t VHField::getSizeY() const {
  return this->size_y;
}


bool debug = false;
// 戻り値：　区間に揃っていない数, 揃っていない数の位置
SeparateResult VHField::isSeparate(const std::uint8_t width, const std::uint8_t height) const {
  std::unordered_set<std::uint16_t> align;
  std::uint16_t i;

  SeparateResult ret{};
  ret.rank = 0;
  if(width == 0) { // 上下での分割
    for(i = 0; i < this->size * height; ++i){
      if(align.size() < this->size * height / 2){
        align.insert(this->get(i));
      }else if(!align.count(this->get(i))){
        ++ret.rank;
        ret.points.push_back(Point(i, this->size));
      }
    }

    if(debug){
      std::cout << "separate_buf" << std::endl;
      std::cout << "align: ";
      for(auto& a : align){
        std::cout << (int)a << " ";
      }
      std::cout << std::endl;
      for(Point& p : ret.points){
        std::cout << "(" << (int)p.x << ", " << (int)p.y << ") ";
      }
      std::cout << std::endl;
      std::cout << "return: " << (int)ret.rank << std::endl;
    }
    return ret;
  }

  std::uint8_t h, w;
  if(height == 0) { // 左右での分割
    for(h = 0; h < this->size_y; ++h) {
      i = h * this->size;
      for(w = 0; w < width; ++w) {
        if(align.size() < this->size_y * width / 2){
          align.insert(this->get(i));
        }else if(!align.count(this->get(i))){
          ++ret.rank;
          ret.points.push_back(Point(i, this->size));
        }
        ++i;
      }
    }
    return ret;
  }else{ // 4分割
    return ret;
  }

}

void normalize_by_order(std::vector<std::uint16_t>& v) {
  std::unordered_map<uint16_t, uint16_t> value_to_id;
  std::uint16_t id = 0;
  for (std::uint16_t& x : v) {
    if (value_to_id.find(x) == value_to_id.end()) {
      value_to_id[x] = id++;  // 初めて出た値にIDを割り振る
    }
    x = value_to_id[x];  // IDで置き換え
  }
}

std::vector<VHField> VHField::separate(const std::uint8_t width, const std::uint8_t height) const {
  
  if(width == 0) {
    std::vector<std::uint16_t> f1(this->field.begin(), this->field.begin() + height * this->size);
    std::vector<std::uint16_t> f2(this->field.begin() + height * this->size, this->field.end());
    normalize_by_order(f1);
    normalize_by_order(f2);

    return std::vector<VHField>{VHField(f1, this->size, height), VHField(f2, this->size, this->size_y - height)};
  }else if(height == 0) {
    std::vector<std::uint16_t> f1, f2;
    f1.reserve(this->size_y * width);
    f2.reserve(this->size_y * (this->size - width));


    for(size_t h = 0; h < this->size_y; ++h) {
      size_t row_start = h * this->size;
      for (size_t col = 0; col < width; ++col)          f1.emplace_back(this->field[row_start + col]); 
      for (size_t col = width; col < this->size; ++col) f2.emplace_back(this->field[row_start + col]); 
    }
    normalize_by_order(f1);
    normalize_by_order(f2);

    /* std::cout << "-------separate---------" << std::endl << "field" << std::endl; */
    /* for(auto a: this->field) std::cout << (int)a << " "; */
    /* std::cout << std::endl << "f1" << std::endl; */
    /* for(auto a: f1) std::cout << (int)a << " "; */
    /* std::cout << std::endl << "f2" << std::endl; */
    /* for(auto a: f2) std::cout << (int)a << " "; */
    /* std::cout << std::endl << "--------------------------" << std::endl; */

    return std::vector<VHField>{VHField(f1, width, this->size_y), VHField(f2, this->size - width, this->size_y)};
  }else {
    std::vector<std::uint16_t> f1, f2, f3, f4;
    f1.reserve(height * width);
    f2.reserve(height * (this->size - width));
    f3.reserve((this->size_y - height) * width);
    f4.reserve((this->size_y - height) * (this->size - width));
    for(size_t h = 0; h < height; ++h) {
      size_t row_start = h * this->size;
      for(size_t w = 0; w < width; ++w)           f1.emplace_back(this->field[row_start + w]); 
      for(size_t w = width; w < this->size; ++w)  f2.emplace_back(this->field[row_start + w]); 
    }
    for(size_t h = height; h < this->size_y; ++h) {
      size_t row_start = h * this->size;
      for(size_t w = 0; w < width; ++w)           f3.emplace_back(this->field[row_start + w]); 
      for(size_t w = width; w < this->size; ++w)  f4.emplace_back(this->field[row_start + w]); 
    }
    normalize_by_order(f1);
    normalize_by_order(f2);
    normalize_by_order(f3);
    normalize_by_order(f4);
    return {
        VHField(f1, width, height),
        VHField(f2, this->size - width, height),
        VHField(f3, width, this->size_y - height),
        VHField(f4, this->size - width, this->size_y - height)
      };
  }

}


std::uint16_t evaluation(SeparateResult sr, std::uint8_t width, std::uint8_t height) {
  std::uint16_t ret = sr.rank;
  if(width == 0) {
    for(Point& p : sr.points) {
      /* std::cout << (int)p.x << ", " << (int)p.y << std::endl; */
      if(p.y > height / 2){
        /* std::cout << (int)ret << std::endl; */
        ret += height - p.y - 1;
      }
    }
  } else if(height == 0) {
    for(Point& p : sr.points) {
      if(p.x > width / 2){
        ret += width - p.x - 1;
      }
    }
  }
  return ret;
}

std::uint16_t evaluation2(const VHField& f, const std::unordered_set<std::uint16_t> align, const std::uint16_t start_index, const std::uint8_t width, const std::uint8_t height) {
  std::uint16_t ret = 0;
  if(width == 0){
    for(std::uint16_t i = start_index; i < height * f.getSize(); ++i){
      if(debug) std::cout << "eva2count: " << i << ", " << (int)f.get(i) << ", " << align.count(f.get(i)) << std::endl;
      if(!align.count(f.get(i))){
        ret += (height - (i / f.getSize()));
        if(debug) std::cout << "eva2: " << i << ", " << (int)f.get(i) << ", " << (int)(height - (i / f.getSize())) << std::endl;
      }
    }
  }else if(height == 0) {
    for(std::uint16_t w = start_index; w < width; ++w){
      for(std::uint16_t h = 0; h < f.getSizeY(); ++h){
        if(debug)std::cout << "eva2count: " << w << ", " << h << ", " << w + h * f.getSize() << ", " << (int)f.get(w + h * f.getSize()) << ", " << align.count(f.get(w + h * f.getSize())) << std::endl;
        if(!align.count(f.get(w + h * f.getSize()))){
          ret += width - w;
          if(debug) std::cout << "eva2: " << w << ", " << h << ", " << (int)f.get(w + h * f.getSize()) << ", " << (int)(width - w) << std::endl;
        }
      }
    }
  }
  return ret;
}


struct OpeTree {
  Ope ope;
  Point p;
  std::shared_ptr<OpeTree> parent;
  OpeTree(std::uint8_t x, std::uint8_t y, std::uint8_t n, Point to, std::shared_ptr<OpeTree> parent) : ope({x, y, n}), p(to), parent(parent) {}
};

// fromはparent.p
bool sss2(std::uint8_t x, std::uint8_t y, const VHField& f, const Point& ep, const std::uint8_t st_x, const std::uint8_t st_y, const std::uint8_t sn,
  std::shared_ptr<OpeTree> parent,
  std::vector<std::vector<std::uint8_t>>& steps,
  std::vector<std::shared_ptr<OpeTree>>& result,
  std::vector<bool>& check
) {
  if(x < st_x || x >= f.getSize() || y < st_y || y >= f.getSizeY() || check[x + y*f.getSize()] || steps[y][x] < sn) return false;
  check[x + y * f.getSize()] = true;
  Ope buf{};
  Point to(x, y);

  /* std::cout << "sss2: sn=" << (int)sn << ", (" << (int)parent->p.x << ", " << (int)parent->p.y << ")->(" << (int)to.x << ", " << (int)to.y << ")" << std::endl; */
  /* for(size_t y=0; y<steps.size();++y) { */
  /*   for(size_t x=0; x<steps[y].size();++x) std::cout << (int)steps[y][x] << " " ; */
  /*   std::cout << std::endl; */
  /* } */

  if(f.toPointCheck(parent->p, to, buf)){
    if(buf[0] < st_x || buf[1] < st_y) return false;
    //
    steps[y][x] = sn;
    std::shared_ptr<OpeTree> o = std::make_shared<OpeTree>(buf[0], buf[1], buf[2], to, parent);
    result.push_back(o);
    if(ep.x == x && ep.y == y){
      return true;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbitwise-instead-of-logical"
    return  sss2(x - 1, y, f, ep, st_x, st_y, sn, parent, steps, result, check)
          | sss2(x + 1, y, f, ep, st_x, st_y, sn, parent, steps, result, check)
          | sss2(x, y - 1, f, ep, st_x, st_y, sn, parent, steps, result, check)
          | sss2(x, y + 1, f, ep, st_x, st_y, sn, parent, steps, result, check);
  }
#pragma GCC diagnostic pop
  return false;

}

void execOpeTree(VHField& f, std::shared_ptr<OpeTree> o) {
  if(o->parent) execOpeTree(f, o->parent);
  if(o->ope[2] == 0) return;
  /* std::cout << "execOpeTree: " << (int)o->ope[0] << ", " << (int)o->ope[1] << ", " << (int)o->ope[2] << std::endl; */
  f.rotate(o->ope);
}

// Fieldでfromをtoへ持っていく
// st_x, st_y: これより大きい場所でのみ行う
void movePoint(VHField& f, const Point& from, const Point& to, std::uint8_t st_x, std::uint8_t st_y) {
  if(from.x == to.x && from.y == to.y) return;


  std::vector<std::shared_ptr<OpeTree>> leaf, leaf_buf;
  leaf.push_back(std::make_shared<OpeTree>(0, 0, 0, from, nullptr));
  std::vector<std::vector<std::uint8_t>> step(f.getSizeY(), std::vector<std::uint8_t>(f.getSize(), 8));
  std::vector<bool> check(f.getSizeY() * f.getSize(), false);

  step[from.y][from.x] = 0;
  bool endFlag = false;
  std::uint8_t sn = 1;
  while(!endFlag && !leaf.empty()){
    for(auto& l : leaf){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbitwise-instead-of-logical"
      std::fill(check.begin(), check.end(), false);
      endFlag |= sss2(l->p.x-1, l->p.y, f, to, st_x, st_y, sn, l, step, leaf_buf, check);
      std::fill(check.begin(), check.end(), false);
      endFlag |= sss2(l->p.x+1, l->p.y, f, to, st_x, st_y, sn, l, step, leaf_buf, check);
      std::fill(check.begin(), check.end(), false);
      endFlag |= sss2(l->p.x, l->p.y-1, f, to, st_x, st_y, sn, l, step, leaf_buf, check);
      std::fill(check.begin(), check.end(), false);
      endFlag |= sss2(l->p.x, l->p.y+1, f, to, st_x, st_y, sn, l, step, leaf_buf, check);
#pragma GCC diagnostic pop
    }
    leaf = std::move(leaf_buf);
    ++sn;
  }

  for(auto& l : leaf){
    if(l->p.x == to.x && l->p.y == to.y){
      leaf_buf.push_back(l);
    }
  }
  if(leaf_buf.size() == 0){
    printf("error\nfrom=(%d, %d), to=(%d, %d)\n", (int)from.x, (int)from.y, (int)to.x, (int)to.y);
    f.print();
  }
  execOpeTree(f, leaf_buf[0]);
}

template <typename T1, typename T2>
std::vector<std::vector<std::pair<T1, T2>>> getAllPermutedPairs(const std::vector<T1>& list1, const std::vector<T2>& list2) {
  using Pair = std::pair<T1, T2>;
  std::vector<std::vector<Pair>> result;

  if (list1.size() != list2.size()) {
    std::cerr << "Error: 要素数が一致していません。\n";
    return result;
  }

  std::vector<T2> perm2 = list2;

  do {
    std::vector<Pair> pairs;
    for (size_t i = 0; i < list1.size(); ++i) {
      pairs.emplace_back(list1[i], perm2[i]);
    }

    std::sort(pairs.begin(), pairs.end());
    do {
      result.push_back(pairs);
    } while (std::next_permutation(pairs.begin(), pairs.end()));

  } while (std::next_permutation(perm2.begin(), perm2.end()));

  return result;
}


//2分割ずつ
// 上半分の下半分のみを基準(24*6) * (2 ~ 18)
// width <= size, height <= size_yとすると効率的
// ある程度揃えたら一つづつ移動させる
VHField algo1_4::beam_search(VHField& f, std::uint8_t width, std::uint8_t height, std::uint8_t deep, size_t beam_width) {
  std::random_device rd;
  std::mt19937 gen(rd());

  size_t i, j, max_loop;
  std::uint8_t y_start, y_end, x_start, x_end;
  std::uint16_t rank_min = 9999, rank_count = 0;

  SeparateResult sep_res;
  std::unordered_set<std::uint16_t> align;
  std::uint16_t si;

  const std::uint8_t fsize_x = f.getSize();
  const std::uint8_t fsize_y = f.getSizeY();
  const std::uint8_t min_fsize = std::min(fsize_x, fsize_y);

  gcm = 0;
  gcb = 0;

  GC[gcm++] = GCRank(f, 255);

  separate_buf1.fill(0);
  if(width == 0){
    y_start = height >> 1;
    y_end = height;
    /* y_end = fsize_y - 1; */
    x_start = 0;
    x_end = fsize_x - 1;

    
    align.reserve(height * fsize_x / 2);
    for(si = 0; si < height * fsize_x; ++si){
      align.insert(GC[0].f.get(si));
      if(align.size() == height * fsize_x / 2) break;
    }
    --si;
    y_start = si / fsize_x;
    si = y_start * fsize_x;


  }else if(height == 0) {
    y_start = 0;
    y_end = fsize_y - 1;
    x_start = width >> 1;
    /* x_end = width; */
    x_end = fsize_x - 1;

    align.reserve(width * fsize_y / 2);
    si = 0;
    for(std::uint16_t w = 0; w < fsize_x; ++w){
      for(std::uint16_t h = 0; h < fsize_y; ++h){
        align.insert(GC[0].f.get(w + h * fsize_x));
        ++si;
        if(align.size() == width * fsize_y / 2) break;
      }
      if(align.size() == width * fsize_y / 2) break;
    }
    x_start = si / fsize_y;
    si = x_start;
  }


  std::vector<std::tuple<int, int, int>> valid_combinations;
  for (std::uint8_t n = 2; n <= min_fsize; ++n) {
    for (std::uint8_t x = x_start; x < x_end; ++x) {
      for (std::uint8_t y = y_start; y < y_end; ++y) {
        if(x + n <= fsize_x && y + n <= fsize_y){
          valid_combinations.emplace_back(x, y, n);
          /* std::cout << (int)x << ", " << (int)y << ", " << (int)n << std::endl; */
        }
      }
    }
  }
  

  uint32_t loop_count = 0;
  auto printgc = [&](){
    std::cout << "loop_count: " << loop_count << std::endl;
    std::cout << "fsize: (" << (int)fsize_x << ", " << (int)fsize_y << "), width = " << (int)width << ", height = " << (int)height << std::endl;
    std::cout << "loop_range: x = " << (int)x_start << "~" << (int)x_end << ", y = " << (int)y_start << "~" << (int)y_end << std::endl;
    std::cout << "gcm: " << gcm << ", max_loop: " << max_loop << std::endl;
    std::cout << "rank: " << GC[0].rank << std::endl << "align: ";
    for (auto v : align) { std::cout << v << " "; } std::cout << std::endl;
    std::cout << "test: " << (int)si << std::endl;
    
    GC[0].f.print();
    /* debug = true; */
    /* auto t = GC[0].f.isSeparate(width, height); */
    /* std::cout << "evaluation: " << evaluation(t, width, height) << std::endl; */
    /* std::cout << "evaluation2: " << evaluation2(GC[0].f, align, si, width, height) << std::endl; */
    /* std::unordered_set<std::uint16_t> a; */
    /* for(size_t i = 0; i < fsize_x * height; i++) a.insert(GC[0].f.get(i)); */
    /* for(size_t w = 0; w < width; w++) for(size_t h = 0; h < fsize_y; ++h) a.insert(GC[0].f.get(w + h * fsize_x)); */
    /* std::cout << "numbers: "; */
    /* for(auto b: a) std::cout << (int)b << " "; */
    /* std::cout << std::endl; */
    /* debug = false; */
    /* for(size_t t = 0; t < max_loop; ++t){ */
    /*   std::cout << t << " :" << GC[t].rank << " "; */
    /*   if(t%20 == 0 && t > 1) std::cout << std::endl; */
    /* } */
    /* std::cout << std::endl; */
    std::cout << std::endl;
  };


  while(1) {
    max_loop = std::min(beam_width, gcm);
    loop_count += 1;
    printgc();

    for(j = 0; j < deep; ++j){
      for(i = 0; i < max_loop; ++i){
        for(const auto& [x, y, n] : valid_combinations){
          GC_buffer[gcb].f = GC[i].f.copy();
          GC_buffer[gcb].f.rotate(x, y, n);
          GC_buffer[gcb].rank = evaluation2(GC_buffer[gcb].f, align, si, width, height);
          ++gcb;
        }
      }
      std::swap(GC, GC_buffer);
      gcm = gcb;
      gcb = 0;
      max_loop = gcm;
    }
    std::shuffle(GC.begin(), GC.begin() + gcm, gen);
    std::sort(GC.begin(), GC.begin() + gcm, [](GCRank a, GCRank b){ return a.rank < b.rank; });

    if(rank_min > GC[0].rank){
      gcb2 = std::min(beam_width, gcm);
      std::transform(GC.begin(), GC.begin() + gcb2, GC_buffer2.begin(), [](GCRank& obj) { return GCRank(obj.f.copy(), obj.rank); });
      rank_min = GC[0].rank;
      rank_count = 0;
    }else{
      ++rank_count;
    }

    if(rank_count >= 3 || GC[0].rank <= 1) {
      std::cout << "end beam search" << std::endl;
      break;
    }

  }
  printgc();

  // 3回以上回転して変わらないなら抜けて一つづつ手動で
  // 高速化を優先
  std::array<std::uint16_t, 576> sb{};
  std::vector<Point> to_list;
  std::vector<std::uint16_t> from_num_list;
  VHField fbuf, fmax, ret;
  std::uint32_t max_count = 99999, num = 9999;
  bool flag2 = false;
  for(auto& f : GC_buffer2) {
    if(f.rank == 0) return f.f;
    else if(f.rank > rank_min) break;
    to_list.clear();
    from_num_list.clear();
    sb.fill(0);
    if(width == 0){
      for(i = 0; i < fsize_x * height; ++i) {
        if(align.count(f.f.get(i))) {
          sb[f.f.get(i)] += 1;
        } else{
          Point to((std::uint16_t)i, fsize_x);
          to_list.push_back(to);
        }
      }
    } else if(height == 0) {
      for(std::uint16_t w = 0; w < width; ++w){
        for(std::uint16_t h = 0; h < fsize_y; ++h){
          i = w + h * fsize_x;
          if(align.count(f.f.get(i))) {
            sb[f.f.get(i)] += 1;
          }else{
            Point to((std::uint16_t)i, fsize_x);
            to_list.push_back(to);
          }
        }
      }
    }
    for(auto& a : align) {
      if(sb[a] != 2) {
        from_num_list.push_back(a);
      }
    }

    /* std::cout << "------------------" << std::endl; */
    /* std::cout << "to_list = "; */
    /* for(auto a : to_list) { std::cout << "(" << (int)a.x << ", " << (int)a.y << "), "; } */
    /* std::cout << std::endl << "from_num_list = "; */
    /* for(auto a : from_num_list) { std::cout << (int)a << ", "; } */
    /* std::cout << std::endl << "align: "; */
    /* for (auto v : align) { std::cout << v << " "; } */
    /* std::cout << std::endl << "rank: " << (int)f.rank << std::endl; */
    /* f.f.print(); */
    /* std::cout << "------------------" << std::endl; */


    std::vector<std::vector<std::pair<Point, std::uint16_t>>> allSequences = getAllPermutedPairs(to_list, from_num_list);
    Point from((std::uint8_t)0, 0);
    num = 9999;
    for (const auto& sequence : allSequences) {
      flag2 = false;
      fbuf = f.f.copy();
      /* std::cout << "-------1------" << std::endl; */
      for (const auto& [to, from_num] : sequence) {
        PENT pent = fbuf.getPent(from_num);
        if(width == 0){
          from = (pent.p1 >= fsize_x * height) ? Point(pent.p1, fsize_x) : Point(pent.p2, fsize_x);
        }else if(height == 0) {
          Point p(pent.p1, fsize_x);
          from = (p.x >= width) ? p : Point(pent.p2, fsize_x);
        }

        Point _to(to.x, to.y);
        bool flag = false;
        if(width == 0) {
          flag = to.x == 0;
          _to.x += flag ? 1 : -1;
          _to.y += 1;
        } else if(height == 0) {
          flag = to.y == (fsize_y - 1);
          _to.x += 1;
          _to.y += flag ? -1 : 1;
        }

/*         printf("from=(%d, %d), to=(%d, %d), _to=(%d, %d), flag=%d\n", (int)from.x, (int)from.y, (int)to.x, (int)to.y, (int)_to.x, (int)_to.y, (int)flag); */
/*         std::cout << "--------------" << std::endl; */
/*         fbuf.print(); */
/*         std::cout << "--------------" << std::endl; */

        if(from.x < width || from.y < height) {
          /* std::cout << "Already available" << std::endl; */
          flag2 = true;
          continue;
        }
        
        try{
        movePoint(fbuf, from, _to, width, height);
        }catch(std::invalid_argument e) {
          printf("error movePoint\nfrom=(%d, %d), to=(%d, %d), _to=(%d, %d), flag=%d\n", (int)from.x, (int)from.y, (int)to.x, (int)to.y, (int)_to.x, (int)_to.y, (int)flag);
          fbuf.print();
        }

/*         fbuf.print(); */
/*         std::cout << "--------------" << std::endl; */

        try{
        if(flag) {
          if(width == 0){
            /* std::cout << "align.count: " << (int)fbuf.get(_to.x, _to.y-1) << ", " << align.count(fbuf.get(_to.x, _to.y-1)) << std::endl; */
            if(align.count(fbuf.get(_to.x, _to.y-1))) fbuf.rotate(_to.x - 1, _to.y - 1, 2);
            fbuf.rotate(_to.x - 1, _to.y - 1, 2);
            fbuf.rotate(_to.x - 1, _to.y - 1, 2);
          }else if(height == 0) {
            /* std::cout << "align.count: " << (int)fbuf.get(_to.x-1, _to.y) << ", " << align.count(fbuf.get(_to.x-1, _to.y)) << std::endl; */
            if(align.count(fbuf.get(_to.x-1, _to.y))) fbuf.rotate(_to.x - 1, _to.y, 2);
            fbuf.rotate(_to.x - 1, _to.y, 2);
            fbuf.rotate(_to.x - 1, _to.y, 2);
          }
        }else {
          if(width == 0){
            /* std::cout << "align.count: " << (int)fbuf.get(_to.x, _to.y-1) << ", " << align.count(fbuf.get(_to.x, _to.y-1)) << std::endl; */
            if(!align.count(fbuf.get(_to.x, _to.y-1))) fbuf.rotate(_to.x, _to.y - 1, 2);
            fbuf.rotate(_to.x, _to.y - 1, 2);
          }else if(height == 0) {
            /* std::cout << "align.count: " << (int)fbuf.get(_to.x-1, _to.y) << ", " << align.count(fbuf.get(_to.x-1, _to.y)) << std::endl; */
            if(!align.count(fbuf.get(_to.x-1, _to.y))) fbuf.rotate(_to.x - 1, _to.y - 1, 2);
            fbuf.rotate(_to.x - 1, _to.y - 1, 2);
          }
        }
        }catch(std::invalid_argument e) {
          printf("error after movePoint\nfrom=(%d, %d), to=(%d, %d), _to=(%d, %d), flag=%d\n", (int)from.x, (int)from.y, (int)to.x, (int)to.y, (int)_to.x, (int)_to.y, (int)flag);
          fbuf.print();
          for(auto& ope : fbuf.getOperate()){
            printf("{%d, %d, %d}\n", (int)ope[0], (int)ope[1], (int)ope[2]);
          }
          throw e;
        }
        /* fbuf.print(); */
        /* std::cout << "--------------" << std::endl; */
        
      }
      
      // getOperate使わないようにしたい
      if(fbuf.getOperate().size() < num) {
        SeparateResult sr = fbuf.isSeparate(width, height);
        if(sr.rank != 0){ // Already availableの時に可能性あり
          if(!flag2) std::cout << "not separate" << std::endl;
        }else{
          num = fbuf.getOperate().size();
          fmax = std::move(fbuf);
        }
      }

    }
    f.f = std::move(fmax);

    if(max_count > f.f.getOperate().size()) {
      ret = f.f;
      max_count = f.f.getOperate().size();
      SeparateResult sr = f.f.isSeparate(width, height);
      if(sr.rank != 0){ std::cout << "not separate" << std::endl; }
      /* std::cout << "rank: " << f.rank <<  ", max_count: " << max_count << std::endl; */
      /* ret.print(); */
    }
  }
  return ret;

}

void algo1_4::algo1_4(BitField& f) {
  const size_t beam_width = 5000;
  ResultTree root;
  VHField field(f.getField(), f.getSize(), f.getSize());
  field.print();

  std::uint8_t w = 0, h = field.getSizeY() >> 1;
  if(h & 1) --h;
  std::uint8_t fw = w, fh = h;

  field = beam_search(field, w, h, 1, beam_width);
  field.print();
  std::vector<VHField> ret = field.separate(w, h);
  std::cout << "--------f-----------" << std::endl;
  std::cout << "ret[0] isProblem:" << (ret[0].isProblem() ? "true" : "false") << std::endl;
  ret[0].print();
  std::cout << "ret[1] isProblem:" << (ret[1].isProblem() ? "true" : "false") << std::endl;
  ret[1].print();
  std::cout << std::endl;
  if(!ret[0].isProblem() || !ret[1].isProblem()) return;

  for(auto& ope : field.getOperate()) {
    f.rotate(ope);
  }

  // 縦分割
  w = field.getSize() >> 1;
  if(w & 1) --w;
  h = 0;
  VHField fup = beam_search(ret[0], w, h, 1, beam_width);
  std::cout << "--------fup-----------" << std::endl;
  fup.print();
  std::vector<VHField> up_ret = fup.separate(w, h);
  std::cout << "up_ret[0] isProblem:" << (up_ret[0].isProblem() ? "true" : "false") << std::endl;
  up_ret[0].print();
  std::cout << "up_ret[1] isProblem:" << (up_ret[1].isProblem() ? "true" : "false") << std::endl;
  up_ret[1].print();
  std::cout << std::endl;
  if(!up_ret[0].isProblem() || !up_ret[1].isProblem()) return;

  for(auto& ope : fup.getOperate()) {
    f.rotate(ope);
  }


  VHField fdown = beam_search(ret[1], w, h, 1, beam_width);
  std::cout << "--------fdown-----------" << std::endl;
  fdown.print();
  std::vector<VHField> down_ret = fdown.separate(w, h);
  std::cout << "down_ret[0] isProblem:" << (down_ret[0].isProblem() ? "true" : "false") << std::endl;
  down_ret[0].print();
  std::cout << "down_ret[1] isProblem:" << (down_ret[1].isProblem() ? "true" : "false") << std::endl;
  down_ret[1].print();
  std::cout << std::endl;

  /* printf("fw=%d, fh=%d\n", fw, fh); */
  for(auto& ope : fdown.getOperate()) {
    /* printf("%d, %d, %d\n", ope[0] + fw, ope[1] + fh, ope[2]); */
    f.rotate(ope[0] + fw, ope[1] + fh, ope[2]);
  }


}
