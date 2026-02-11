#include <algo1.cuh>
#include <vector>
#include <cpu_process.hpp>
#include <chrono>
#include <iostream>
#include <param.hpp>

#define SAMPLE_NUM 100

int main(void) {
  uint32_t fsize = 12;
  double mean_time = 0;
  double max_time = 0;
  double min_time = 1 << 20;
  double mean_ope = 0;
  uint32_t max_ope = 0;
  uint32_t min_ope = 1 << 20;
  for(int i = 0; i < SAMPLE_NUM; i++){
    std::vector<uint16_t> start_field = makeShuffledPairs(fsize);
    auto start_time = std::chrono::high_resolution_clock::now();
    auto result = bs1::algo(start_field.data(), fsize);
    auto end_time = std::chrono::high_resolution_clock::now();
    double time = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() / 1000.0;

    std::vector<uint16_t> field = start_field;
    for(auto& rid: result) {
      rotateField(field, fsize, rid);
    }
    if(!isEnd(field, fsize)){
      printf("ERROR: Don't end\n");
      field = start_field;
      printField(fsize, field.data());
      for(auto& rid: result) {
        uint32_t x, y, n;
        getParamsCpu(rid, fsize, &x, &y, &n);
        rotateField(field, fsize, x, y, n);
        printf("rotate: rid=%d, (%d, %d, %d)\n", rid, x, y, n);
        printField(fsize, field.data());
      }
    }


    if(time < min_time) min_time = time;
    if(max_time < time) max_time = time;
    mean_time += time;

    uint32_t openum = result.size();
    if(openum < min_ope) min_ope = openum;
    if(max_ope < openum) max_ope = openum;
    mean_ope += (double)openum;
    printf("time: %f, operate: %d\n", time, openum);
  }
  mean_time /= SAMPLE_NUM;
  mean_ope /= SAMPLE_NUM;
  printf("fsize = %d\nconfig:\n\tblocksPerGrid:\t\t%d\n\tthreadsPerBlocks:\t%d\n\tBEAM_WIDTH:\t\t%d\n\tFIELDS_PER_THREAD:\t%d\n\tCPU_THREAD_NUM:\t\t%d\n\tQUEUE_SIZE:\t\t%d\n", fsize, BLOCKS_PER_GRID, THREADS_PER_BLOCK, BEAM_WIDTH, FIELDS_PER_THREAD, CPU_THREAD_NUM, QUEUE_SIZE);
  printf("time:\n\tmax:\t%f[sec]\n\tmin:\t%f[sec]\n\tmean:\t%f[sec]\n\noperate:\n\tmax:\t%d\n\tmin:\t%d\n\tmean:\t%f\nmean time per operater %f[sec]\nmean operate per pair:%f\n", max_time, min_time, mean_time, max_ope, min_ope, mean_ope, mean_time/mean_ope, mean_ope / (fsize * fsize / 2));
  
  return 0;
}
