#include <fsdb.hpp>
#include <state.hpp>
#include <field.hpp>
#include <algorithm>
#include <random>
#include <vector>
#include <iostream>
#include <chrono>

#define FIELD_SIZ 24
#define DB_FILE "../db/algo1_5_2_24.db"

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), progress(0), score(0), end_flag(false), last_type(FLAT), log(v_pair_ii(0)), ok_pair(0), ope_sum(0), pile_dir(HORIZON) {}

using namespace fsdb;

Field randomfield(const std::uint8_t fsize, size_t seed = 0) {
  const std::uint16_t num_size = fsize * fsize / 2;
  std::vector<std::uint16_t> vec;
  vec.reserve(fsize * fsize);
  for (int i = 0; i < num_size; ++i) {
    vec.push_back(i);
    vec.push_back(i);
  }
  std::random_device rd;
  if(seed == 0){
    seed = rd();
  }
  /* std::cout << "seed: " << seed << std::endl; */
  std::mt19937 g(seed);
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


struct FieldData {
  Field f;
  std::vector<Ope> hist;
  int progress;
};

double total_ms = 0.0;
double max_ms = 0.0;
double min_ms = 999999.0;
size_t total_c = 0;

int think(Field& f) {
  using namespace std::chrono;
  int x;
  std::random_device rd;
  std::mt19937 gen(rd());
  State *s = new State();
  std::vector<FieldData> buffer; 
  std::vector<FieldData> buffer2; 


  buffer.push_back({f, {}, 1});

  int max_progress = 1;
  while(buffer.size() > 0 && buffer[0].progress < FIELD_SIZ * 2 - 2){
    /* buffer[0].f.printField(); */
    /* std::cout << "progress: " << buffer[0].progress << std::endl; */
    /* std::cout << "buffer size: " << buffer.size() << std::endl; */
    max_progress = buffer[0].progress;
    for(auto& b : buffer) {
      s->f = b.f;
      s->progress = b.progress;
      auto start = high_resolution_clock::now();
      Routes r = getOperation(s);
      auto end = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(end - start).count() / 1000.0;
      if(duration > max_ms) max_ms = duration;
      else if(duration < min_ms) min_ms = duration;
      total_ms += duration;
      total_c += 1;

      if(r.size <= 0){
        continue;
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

      for(auto& opes: r.getAllOperation()) {
        FieldData nf{};
        nf.progress = b.progress + 2;
        nf.hist = b.hist;
        nf.f = Field(b.f.size);
        b.f.getClone(&(nf.f));
        for(auto& ope: opes) {
          nf.f.rotate(ope);
          nf.hist.push_back(ope);
        }
        buffer2.push_back(nf);
      }
    }

    std::swap(buffer, buffer2);
    buffer2.clear();
  }

  for(auto& b: buffer) {
    b.f.printField();
    std::cout << "hist size:" << b.hist.size() << std::endl;
  }
  std::cout << "result progress:" << max_progress << std::endl;
  delete s;
  std::cout << "getOperation count    : " << total_c << std::endl;
  std::cout << "getOperation time mean: " << total_ms / total_c << "[ms]" << std::endl;
  std::cout << "getOperation time max : " << max_ms << "[ms]" << std::endl;
  std::cout << "getOperation time min : " << min_ms << "[ms]" << std::endl;
  if(buffer.size() == 0) return 1;
  return 0;

}

int main(void) {
  std::chrono::system_clock::time_point startTime, endTime;
  startTime = std::chrono::system_clock::now(); // 計測開始時間
  if(! fsdb_init(DB_FILE)){
    std::cerr << "init error" << std::endl;
    return 0;
  }
  endTime = std::chrono::system_clock::now();  // 計測終了時間
  std::cout << "time[ms]: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   endTime - startTime)
                   .count() <<std::endl;

  const std::uint8_t fsize = FIELD_SIZ;

  // 4190757347, 2911936039, 237185340, 3380594103
  Field f = randomfield(fsize, 2326189944);
  size_t count = 0;
  while(think(f) != 0) {
    f = randomfield(fsize, 0);
    if(count >= 100000)break;
    count++;

    /* break; */
  }

}
