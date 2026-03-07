#include <algo1.hpp>
#include <algo2.cuh>
#include <vector>
#include <array>
#include <future>
#include <thread>

// debug
#include <algo2lib.hpp>
using namespace algo2lib;


int main(void) {
  size_t max_threads = 8;

  // Initialization
  MemObj1 mem1 = init1();
  /* MemObj2 mem2{}; */

  std::vector<MemObj2> mem2s(max_threads);

  // get problem
  RawField field;
  uint32_t fsize;
  getProblem(field, fsize);

  // algorithm 1
  std::vector<std::vector<Ope>> opes;
  std::vector<std::vector<uint16_t>> fields;
  std::vector<std::pair<uint8_t, uint8_t>> offsets;
  algorithm1(field, fsize, mem1, opes, fields, offsets);;

  /* // algorithm2 debug */
  /* uint32_t fsize = 12; */
  /* RawField field = createRandomField(fsize); */
  /* fields.push_back(field); */
  /* opes.push_back(std::vector<Ope>{}); */
  /* offsets.push_back(std::pair(0, 0)); */

  // algorithm 2
  /* std::vector<Ope> result = algorithm2(fields, opes, offsets, 12, mem2); */
  /* std::vector<Ope> result = algorithm2(fields, opes, offsets, fsize, mem2); */

  std::vector<Ope> result;
  std::vector<std::future<std::vector<Ope>>> futures;
  size_t max_depth = 9999;

  for(size_t i = 0; i < fields.size(); ++i) {
    size_t tid = i % max_threads;
    futures.emplace_back( std::async(std::launch::async, [&, i, tid]{ return algo2_2::algo2_2( fields[i], opes[i], offsets[i], fsize % 4 == 0 ? 12 : 10, mem2s[tid]); }));
    while(futures.size() >= max_threads) {

      for(auto it = futures.begin(); it != futures.end(); ) {
        if(it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
          auto ret = it->get();
          if(ret.size() < max_depth) {
            max_depth = ret.size();
            result = std::move(ret);
          }
          it = futures.erase(it);
        } else { ++it; }
      }
    }
  }


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
