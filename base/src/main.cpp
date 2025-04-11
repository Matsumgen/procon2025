
#include <Field.hpp>
//1pair最大3回?
//xが等しい列に揃えて回転

int main(void){
  Field *f = loadProblem("../problem/sample.csv");
  f->print();
  f->rotate(0, 0, 4);
  f->print();
  return 0;
}
