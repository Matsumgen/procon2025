#include <fsdb.hpp>
#include <state.hpp>
#include <field.hpp>
#include <algorithm>
#include <random>
#include <vector>
#include <iostream>
#include <chrono>

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), progress(0), log(v_pair_ii(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0), pile_dir(HORIZON) {}

using namespace fsdb;

Field randomfield(const std::uint8_t fsize) {
  const std::uint16_t num_size = fsize * fsize / 2;
  std::vector<std::uint16_t> vec;
  vec.reserve(fsize * fsize);
  for (int i = 0; i < num_size; ++i) {
    vec.push_back(i);
    vec.push_back(i);
  }
  std::random_device rd;
  std::mt19937 g(rd());
  /* std::mt19937 g(1234); */
  std::shuffle(vec.begin(), vec.end(), g);
  
  Field f(fsize);
  std::vector<int> nums(fsize * fsize / 2, 0);
  for(size_t i = 0; i < vec.size(); ++i) {
    f.ent_mem[i] = Ent(vec[i], nums[vec[i]]);
    f.pos_mem[i] = Pos(i % fsize, i / fsize);
    nums[vec[i]] += 1;
  }

  return f;
}

int main(void) {
  fsdb_init("../save2/algo1_5_2_8.db", 8);
  const std::uint8_t fsize = 8;
  int x;
  State *s = new State();
  s->f = randomfield(fsize);

  s->progress = 1;

  std::chrono::system_clock::time_point startTime, endTime;
  startTime = std::chrono::system_clock::now(); // 計測開始時間
  std::random_device rd;
  std::mt19937 gen(rd());
  do{
    s->f.printField();
    Routes r = getOperation(s);
    if(r.size <= 0){
      printf("Not Found\n");
      break;
    }
    /* std::cout << "Routes size: " << (int)r.size << std::endl; */
    /* int ii = 0; */
    /* std::cout << "getAllOperation" << std::endl; */
    /* for(auto& opes: r.getAllOperation()) { */
    /*   printf("[%2d] ", ii); */
    /*   for(auto& ope: opes) { */
    /*     printf("(%d, %d, %d) -> ", ope.x, ope.y, ope.n); */
    /*   } */
    /*   printf("\n"); */
    /*   ++ii; */
    /* } */

    std::cout << "getOperation" << std::endl;
    for(size_t i = 0; i < r.size; ++i) {
      printf("[%2d] ", i);
      for(auto& ope: r.getOperation((size_t)i)) {
        printf("(%d, %d, %d) -> ", ope.x, ope.y, ope.n);
      }
      printf("\n");
    }

    std::uniform_int_distribution<> dist(0, r.size - 1);
    x = dist(gen);
    /* std::cin >> x; */
    /* x = 0; */
    std::cout << "selected: " << (int)x << std::endl;
    for(auto& ope: r.getOperation((size_t)x)) {
      printf("(%d, %d, %d) -> ", ope.x, ope.y, ope.n);
      s->f.rotate(ope);
    }
    printf("\n");
    s->progress += 2;
  }while(x >= 0);
  s->f.printField();

  endTime = std::chrono::system_clock::now();  // 計測終了時間
  std::cout << "time[ms]: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   endTime - startTime)
                   .count();


  delete s;
  fsdb_deinit();
}
