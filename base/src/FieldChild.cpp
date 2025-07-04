#include <Field.hpp>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

FieldChild::FieldChild(std::shared_ptr<Field> f, const int x, const int y, const int n)
: parent(f), answer_size(f->getAnswer().size()), px(x), py(y)
{
  this->size = n;
  this->field = new ENT**[n];
  this->confirm = new int*[n];
  for(int dy = 0; dy < n; dy++){
    this->field[dy] = new ENT*[n];
    this->confirm[dy] = new int[n]{};
    for(int dx = 0; dx < n; dx++){
      this->field[dy][dx] = new ENT;
      *(this->field[dy][dx]) = *(f->get(x + dx, y + dy));
      this->field[dy][dx]->p[0] -= px;
      this->field[dy][dx]->p[1] -= py;
      this->confirm[dy][dx] = f->isConfirm(x + dx, y + dy);
    }
  }
  this->correspondence = Field::reallocation(*this);
  const int numSize = correspondence.size();
  if(numSize != n*n/2){
    std::cout << "numSize: " << numSize << "n*n/2: " << n*n/2 << std::endl;
    f->print();
    throw std::logic_error("don't problem");
  }
  this->pentities = new PENT[numSize]{};
  int nn;
  for(int dy = 0; dy < n; dy++){
    for(int dx = 0; dx < n; dx++){
      nn = this->field[dy][dx]->num;
      if(this->pentities[nn].p1 == nullptr){
        this->pentities[nn].p1 = this->field[dy][dx];
      }else if (this->pentities[nn].p2 == nullptr){
        this->pentities[nn].p2 = this->field[dy][dx];
      }else{
        std::cout << "nn: " << nn << std::endl;
        f->print();
        /* throw std::logic_error("don'tproblem"); */
      }
    }
  }

}


/* FieldChild::FieldChild(std::shared_ptr<FieldChild> f, const int x, const int y, const int n) */
/* : FieldChild(std::static_pointer_cast<Field>(f), x + f->px, y + f->py, n){ } */

FieldChild::FieldChild(const FieldChild& other)
  : Field(other), parent(other.parent), answer_size(other.answer_size), px(other.px), py(other.py), correspondence(other.correspondence) { }

// 親Fieldに自身を反映させる
void FieldChild::reflection(){
  this->parent->reflection(this, this->px, this->py, this->answer_size, this->correspondence);
}

std::shared_ptr<Field> FieldChild::clone() const {
  return std::make_shared<FieldChild>(*this);
}

void FieldChild::print() const{
  std::cout << "FieldChild: parent" << std::endl;
  this->parent->print();
  std::cout << "FieldChild: child, px=" << this->px << ", py=" << this->py << std::endl;
  Field::print();
}

std::vector<std::string> FieldChild::getAnswer() const{
  std::vector<std::string> ret = parent->getAnswer();
  ret.reserve(ret.size() + this->answer.size());
  for(auto& ans : this->answer){
    std::ostringstream oss;
    oss << "{" << ans[0] + px << ", " << ans[1] + py << ", " << ans[2] << "}";
    ret.push_back(oss.str());
  }
  return ret;
}
std::vector<std::array<int, 3>> FieldChild::getOperate() const{
  std::vector<std::array<int, 3>> ret = this->parent->getOperate();
  ret.reserve(ret.size() + this->answer.size());
  // pxを再起的に手にいれる
  std::array<int, 2> pxy = this->getPxy();
  for(auto& ans : this->answer){
    ret.push_back({ans[0] + pxy[0], ans[1] + pxy[1], ans[2]});
  }
  return ret;
}

std::array<int, 2> FieldChild::getPxy() const{
  if(std::shared_ptr<FieldChild> fc = std::dynamic_pointer_cast<FieldChild>(this->parent)){
    std::array<int, 2> ppxy = fc->getPxy();
    ppxy[0] += this->px;
    ppxy[1] += this->py;
    return ppxy;
  }else{
    return {this->px, this->py};
  }
}
