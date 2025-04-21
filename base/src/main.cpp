
#include <Field.hpp>
#include <algo.hpp>
#include <iostream>
#include <string>
#include <chrono>

int main(int argc, char *argv[]){
  std::chrono::system_clock::time_point  startTime, endTime;
  Field *f;
  for(int i = 1; i < argc; i++){
    startTime = std::chrono::system_clock::now(); // 計測開始時間
    f = loadProblem(argv[i]);
    /* f->print(); */
    alg1(f);
    /* f->print(); */
    endTime = std::chrono::system_clock::now();  // 計測終了時間
    if(!f->isEnd()){
      std::cout << "ERROR: is not End" << std::endl;
      f->print();
      return 1;
    }
    //統計取る場合はこれだけ表示
    std::cout << "file: " << argv[i] << std::endl;
    std::cout << "size: " << f->getSize() << std::endl;
    std::cout << "time[ms]: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << std::endl;
    std::cout << "answer step: " << f->getAnswer().size() << std::endl;
    std::cout << std::endl;


    /* std::cout << "answer" << std::endl; */
    /* for(std::string ans : f->getAnswer()){ */
    /*   std::cout << ans << std::endl; */
    /* } */
    delete f;
  }
  return 0;
}
