
#include <Field.hpp>
//1pair最大3回?
//xが等しい列に揃えて回転

int main(void){
  Field *f = loadProblem("../problem/sample.csv");
  f->print();
  f->rotate(1, 0, 4);
  f->print();
  f->rotate(1, 3, 5);
  f->print();
  f->rotate(3, 0, 3);
  f->print();
  return 0;
}
