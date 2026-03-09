// --- main/main.cu ---
#include <algo1.cuh>
#include <vector>
#include <cpu_process.hpp>
#include <chrono>
#include <iostream>
#include <cuda_runtime.h>

#define TEST_NUM 5 // 比較を行う回数

int main(void) {
  cudaFree(0);
  uint32_t fsize = 14;

  std::cout << "=== 評価関数 比較テスト (" << fsize << "x" << fsize << ") ===" << std::endl;

  for (int i = 0; i < TEST_NUM; ++i) {
    // 同じ初期盤面を生成
    std::vector<uint16_t> start_field = makeShuffledPairs(fsize);
    std::cout << "\n[Test " << i + 1 << "]" << std::endl;

    // --- 評価関数1 (従来: 隣接のみ) ---
    std::vector<uint16_t> field_copy1 = start_field; // 念のためコピーを渡す
    auto start1 = std::chrono::high_resolution_clock::now();
    auto result1 = bs1::algo(field_copy1.data(), fsize, 1);
    auto end1 = std::chrono::high_resolution_clock::now();
    double time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() / 1000.0;
    
    std::cout << "  [Eval 1] 手数: " << result1.size() << ", 時間: " << time1 << " 秒" << std::endl;

    // --- 評価関数2 (新規: 斜めも考慮) ---
    std::vector<uint16_t> field_copy2 = start_field;
    auto start2 = std::chrono::high_resolution_clock::now();
    auto result2 = bs1::algo(field_copy2.data(), fsize, 2);
    auto end2 = std::chrono::high_resolution_clock::now();
    double time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() / 1000.0;
    
    std::cout << "  [Eval 2] 手数: " << result2.size() << ", 時間: " << time2 << " 秒" << std::endl;
  }
 
  return 0;
}