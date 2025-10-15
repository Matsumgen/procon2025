#include <Field.hpp>
#include <fielddb.hpp>
#include <algo.hpp>
#include <algo3_1.hpp>
#include <chrono>
#include <iostream>
#include <string>



int main(int argc, char *argv[]) {
  fdb::field4_init("./field4.db");
  std::chrono::system_clock::time_point startTime, endTime;
  for (int i = 1; i < argc; i++) {
    startTime = std::chrono::system_clock::now(); // 計測開始時間
    Field f = Field::loadProblem(argv[i]);
    f.print();
    /* alg1_1(f, 3, 3, 25); // 569 */
    /* alg1_1(f, 3, 2, 200); // 538 7030[s] */
    alg1_1(f, 3, 2, 100); // 548 1435[s]
    f.print();
    endTime = std::chrono::system_clock::now();  // 計測終了時間
    for(auto& ans : f.getOperate()){
      std::cout << ans[0] << " " << ans[1] << " " << ans[2] << std::endl;
    }
    if(!f.isEnd()){
      std::cout << "ERROR: is not End" << std::endl;
      f.print();
      return 1;
    }
    // 統計取る場合はこれだけ表示
    std::cout << "file: " << argv[i] << std::endl;
    std::cout << "size: " << f.getSize() << std::endl;
    std::cout << "time[ms]: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     endTime - startTime)
                     .count()
              << std::endl;
    std::cout << "answer step: " << f.getAnswer().size() << std::endl;
    std::cout << std::endl;

    /* std::cout << "answer" << std::endl; */
    /* for(std::string ans : f->getAnswer()){ */
    /*   std::cout << ans << std::endl; */
    /* } */
  }
  fdb::field4_deinit();
  return 0;
}
