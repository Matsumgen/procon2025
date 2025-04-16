
#include <Field.hpp>
#include <iostream>

int main(void){
  Field *f = loadProblem("../problem/sample.csv");
  f->print();
  if(f->toPoint(f->get(2, 1)->p, f->get(3, 3)->p)){
    std::cout << "成功" << std::endl;
  }else{
    std::cout << "失敗" << std::endl;
  }
  f->print();
  f->toPoint(f->get(1,0)->p, f->get(2, 5)->p);
  f->print();
  f->setConfirm(0, 0);
  f->setConfirm(0, 1);
  f->toPoint(f->get(1,0)->p, f->get(2, 1)->p);
  f->print();
  return 0;
}
