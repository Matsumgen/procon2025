#include <algo1.hpp>
#include <algo2.cuh>
#include <vector>
#include <array>


// debug
#include <algo2lib.hpp>
using namespace algo2lib;


int main(void) {

  srand(42); 

  // Initialization

  // Initialization
  /* MemObj1 mem1 = init1(); */
  MemObj2 mem2{};

  // get problem
  /* RawField field; */
  /* uint32_t fsize; */
  /* getProblem(field, fsize); */

  // algorithm 1
  std::vector<std::vector<Ope>> opes;
  std::vector<std::vector<uint16_t>> fields;
  std::vector<std::pair<uint8_t, uint8_t>> offsets;
  /* algorithm1(field, fsize, mem1, opes, fields, offsets);; */

  // algorithm2 debug
  uint32_t fsize = 12;
  RawField field = createRandomField(fsize);
  fields.push_back(field);
  opes.push_back(std::vector<Ope>{});
  offsets.push_back(std::pair(0, 0));

  // algorithm 2
  /* std::vector<Ope> result = algorithm2(fields, opes, offsets, 12, mem2); */
  std::vector<Ope> result = algorithm2(fields, opes, offsets, fsize, mem2);

  // debug
  RawField f = field;
  for(auto& v : result) { rotateField(f, fsize, v); }
  if(!isEnd(f, fsize)) {
    f = field;
    printField(f, fsize);
    std::cout << std::endl;
    for(auto& v : result) {
      std::cout << (int)v.x() << " " << (int)v.y() << " " << (int)v.n() << std::endl;
    }
    std::cout << std::endl;
    printField(f, fsize);
  }

  // submission
  submission(result);
 
  return 0;
}
