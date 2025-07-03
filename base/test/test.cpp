#include <BitField.hpp>
#include <fielddb.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace bf;


void test_BField(int argc, char *argv[]) {
  if(argc < 1){
    std::cout << "No file passed" << std::endl;
  }
  BField f = BField::loadCsv(argv[1]);
  f.print('\t');
  f.rotate(1,2,4);
  f.print('\t');
  for(auto ans : f.getAnswer())
    std::cout << ans << std::endl;

  BField c1 = f;
  c1.print('\t');
  c1.rotate(0,0,2);
  c1.print('\t');
  for(auto ans : c1.getAnswer())
    std::cout << ans << std::endl;

  BField c2 = f;
  c2.print('\t');
  c2.rotate(1,0,2);
  c2.print('\t', true);
  for(auto ans : c2.getAnswer())
    std::cout << ans << std::endl;

  for(int i=0; i<50; i++){
    std::cout << i << ": " << c2.isNumAdjacent(i);
    if((i+1) % 5 == 0)  std::cout << std::endl;
    else                std::cout << '\t';
  }

  auto v = c2.reallocationD();
  c2.print();
  for(unsigned int i=0; i<v.size(); ++i){
    std::cout << i << ": " << v[i] << std::endl;
  }
}

/*
測定日:       2025/7/3 10:36
max_loop:     10000000
合計時間[ms]: 39595.6
時間平均[ms]: 0.00395956
平均:         5.47318
最大値:       7
最小値:       0
*/
void test_db4(int argc, char *argv[]) {
  unsigned int max_loop;
  if(argc < 1){
    max_loop = 10;
  }else{
    max_loop = std::stoi(argv[1]);
  }
  std::chrono::system_clock::time_point startTime, endTime;
  std::vector<double> times;
  std::vector<std::uint8_t> statistics;
  times.reserve(max_loop);
  statistics.reserve(max_loop);
  fdb::field4_init("./field4.db");
  for(unsigned int i = 0; i < max_loop; ++i ){
    BField f(BitField::randomField(4));
    /* f.print(); */
    startTime = std::chrono::system_clock::now(); // 計測開始時間
    std::vector<std::array<std::uint8_t, 3>> opes = fdb::getField4(f);
    for(auto ans : opes) {
      f.rotate(ans);
    }
    endTime = std::chrono::system_clock::now();  // 計測終了時間

    if(!f.isEnd()){
      std::cout << "Don't end" << std::endl;
      f.print();
      for(auto ans : f.getAnswer())
        std::cout << ans << std::endl;
    }

    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
    times.push_back(elapsed.count());
    statistics.push_back(opes.size());

    /* std::cout << std::endl; */

  }
    double t_average = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    double t_sum = std::accumulate(times.begin(), times.end(), 0.0);
    double average = std::accumulate(statistics.begin(), statistics.end(), 0.0) / statistics.size();
    int max_val = *std::max_element(statistics.begin(), statistics.end());
    int min_val = *std::min_element(statistics.begin(), statistics.end());

    std::cout << "合計時間[ms]: " << t_sum << std::endl;
    std::cout << "時間平均[ms]: " << t_average << std::endl;
    std::cout << "平均:         " << average << std::endl;
    std::cout << "最大値:       " << max_val << std::endl;
    std::cout << "最小値:       " << min_val << std::endl;
  fdb::field4_deinit();
}


int main(int argc, char *argv[]) {
  test_db4(argc, argv);
}
