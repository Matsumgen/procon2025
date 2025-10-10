/* #define DEBUG */

#include <fsfield.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <stdexcept>


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
    std::cerr << oss.str() << std::endl;
}



using namespace fsdb;

Routes::Routes(Ope ope, std::vector<Routes_ptr> next) : ope(ope), next(next) {
  this->size = 0;
  for(auto& nv: next) {
    this->size += nv->size;
  }
  if(next.size() == 0 and ope.n != 0) this->size = 1;
}
Routes::Routes(std::vector<Routes_ptr> next) : ope(Ope(0, 0, 0)), next(next) {
  this->size = 0;
  for(auto& nv: next) {
    this->size += nv->size;
  }
}
Routes::Routes(Ope ope) : ope(ope), size(1), next(std::vector<Routes_ptr>()) {}
Routes::Routes() : ope(Ope(0, 0, 0)), size(0), next(std::vector<Routes_ptr>()) {}

std::uint8_t Routes::getSize() {
  return this->size;
}

std::vector<Ope> Routes::getOperation(size_t idx) {
  std::vector<Ope> ret, buf;
  ret.reserve(4);
  if(this->ope.n != 0) ret.push_back(this->ope);
  size_t isum = 0;
  for(auto& n: this->next) {
    isum = n->getSize();
    if(idx <= isum - 1) {
      buf = n->getOperation(idx);
      ret.insert(ret.end(), std::make_move_iterator(buf.begin()), std::make_move_iterator(buf.end()));
      break;
    }else {
      idx -= isum;
    }
  }
  return ret;
}

std::vector<std::vector<Ope>> Routes::getAllOperation() {
  std::vector<std::vector<Ope>> ret;
  ret.reserve(this->size);
  for(size_t i = 0; i < this->next.size(); ++i) {
    if(this->ope.n != 0){
      for(auto& opes: this->next[i]->getAllOperation()){
        std::vector<Ope> buf;
        buf.reserve(1 + opes.size());
        buf.push_back(this->ope);
        buf.insert(buf.end(), std::make_move_iterator(opes.begin()), std::make_move_iterator(opes.end()));
        ret.push_back(buf);
      }
    }else{
      std::vector<std::vector<Ope>> buf;
      buf = this->next[i]->getAllOperation();
      ret.insert(ret.end(), std::make_move_iterator(buf.begin()), std::make_move_iterator(buf.end()));
    }
  }
  if(this->next.size() == 0 && this->ope.n != 0) {
    ret.push_back(std::vector<Ope>{this->ope});
  }
  return ret;
}

FsField::FsField(vector<std::uint16_t> field, std::uint8_t size) : field(field), size(size) {};
FsField::FsField(const Field& f) : size(f.size) {
  vector<std::uint16_t> _field;
  field.reserve(f.size);

  for(size_t i = 0; i < f.size * f.size; ++i) {
    _field.push_back(f.ent_mem[i].val);
  }
  this->field = _field;
}

std::uint16_t FsField::get(std::uint8_t x, std::uint8_t y) const {
  return this->field[x + y * this->size];
}

std::uint16_t FsField::get(std::array<std::uint8_t, 2> p) const {
  return this->field[p[0] + p[1] * this->size];
}

bool FsField::inField(const std::uint8_t x, const std::uint8_t y) const {
  if(x < this->size && y < this->size)  return true;
  return false;
}

void FsField::print() {
  for(size_t i = 0; i < this->field.size(); ++i) {
    printf("%3d%c", this->field[i], " \n"[i%this->size == this->size - 1]);
  }
  std::cout << std::endl;
}

void FsField::rotate(Ope ope) {
  if(!this->inField(ope.x, ope.y) || !this->inField(ope.x + ope.n - 1, ope.y + ope.n - 1)){
    printf("%d, %d, %d\n", ope.x, ope.y, ope.n);
    throw std::invalid_argument("rotate: out of range");
  }
  std::uint8_t n_half = ope.n >> 1;

  std::uint8_t h, w;
  std::uint16_t i0, i1, i2, i3, buf;
  for(h = 0; h < n_half; ++h) {
    i0 = ope.x + (ope.y+h) * this->size;
    i1 = ope.x + (ope.n - 1 - h) + ope.y * this->size;
    i2 = (ope.x + (ope.n - 1)) + (ope.y + (ope.n - 1 - h)) * this->size;
    i3 = (ope.x + h) + (ope.y + (ope.n - 1)) * this->size;
    for(w = 0; w < n_half; ++w){
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

  if(ope.n & 1){
    i0 = ope.x + n_half  + (ope.y) * this->size;
    i1 = ope.x + (ope.n - 1) + (ope.y + n_half) * this->size;
    i2 = ope.x + n_half + (ope.y + ope.n - 1) * this->size;
    i3 = ope.x + (ope.y + n_half) * this->size;
    for(h = 0; h < n_half; ++h) {
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


bool SRoutes::isNext(std::array<std::uint8_t, 2> p1, std::array<std::uint8_t, 2> p2, std::array<std::uint8_t, 2> p3, std::array<std::uint8_t, 2> p4) const {
  return f.inField(p1[0], p1[1]) && f.inField(p2[0], p2[1]) && f.inField(p3[0], p3[1]) && f.inField(p4[0], p4[1]) && ((f.get(p1) == f.get(p2) && f.get(p3) == f.get(p4)) || (f.get(p1) == f.get(p3) && f.get(p2) == f.get(p4)));
}

void SRoutes::toNext(SRoutes_ptr& r, Ope ope) const {
  r->f = this->f;
  r->ope = ope;
  r->f.rotate(ope);
  r->tg = this->tg;
  r->next.clear();
}


bool SRoutes::inOpe(const Ope ope) const {
  for(auto& nv: this->next) {
    if(nv->ope.x == ope.x && nv->ope.y == ope.y && nv->ope.n == ope.n)  return true;
  }
  return false;
}

bool SRoutes::inOpe(const std::array<std::uint8_t, 3> ope) const {
  for(auto& nv: this->next) {
    if(nv->ope.x == ope[0] && nv->ope.y == ope[1] && nv->ope.n == ope[2])  return true;
  }
  return false;

}

bool SRoutes::inField(const Ope ope) const {
  return !(ope.x < 0 || this->f.size < ope.x + ope.n || ope.y < 0 || this->f.size < ope.y + ope.n);
}

bool SRoutes::inField(const std::array<std::uint8_t, 3> ope) const {
  return !(ope[0] < 0 || this->f.size < ope[0] + ope[2] || ope[1] < 0 || this->f.size < ope[1] + ope[2]);
}

bool SRoutes::isEnd() const {
  return (this->f.get(tg) == f.get(tg[0]+1, tg[1]) && this->f.get(tg[0], tg[1]+1) == this->f.get(tg[0]+1, tg[1]+1)) || (this->f.get(tg) == f.get(tg[0], tg[1]+1) && this->f.get(tg[0]+1, tg[1]) == this->f.get(tg[0]+1, tg[1]+1));
}

Routes_ptr SRoutes::toRoutes() {
  Routes_ptr rptr;
  std::vector<Routes_ptr> v;
  v.reserve(this->next.size());
  for(auto& nv: this->next) {
    rptr = nv->toRoutes();
    v.push_back(rptr);
  }
  return make_Routes_ptr(this->ope, v);
}
bool SRoutes::check() {
  /* printf("check: next(%d), (%d, %d, %d)\n", this->next.size(), (int)ope.x, (int)ope.y, (int)ope.n); */
  if(this->next.size() == 0) {
    return this->isEnd();
  }
  for(size_t i = 0; i < this->next.size();) {
    /* printf("\t- (%d, %d, %d)\n", (int)(this->next[i]->ope.x), (int)(this->next[i]->ope.y), (int)(this->next[i]->ope.n)); */
    if(!this->next[i]->check()) {
      /* printf("delete - (%d, %d, %d)\n", (int)(this->next[i]->ope.x), (int)(this->next[i]->ope.y), (int)(this->next[i]->ope.n)); */
      this->next.erase(this->next.begin() + i);
    }else {
      ++i;
    }
  }
  if(this->next.size() == 0) {
    return false;
  }
  return true;
}
