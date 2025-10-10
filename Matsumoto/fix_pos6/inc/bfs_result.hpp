#ifndef BFS_RESULT_HPP
#define BFS_RESULT_HPP 0

#include "state.hpp" // Stateや関連する型定義に必要
#include "field.hpp" // vvvvv_ope型定義に必要

#include <cstdio>
#include <string>

#define BFS_RESULT_FILE_NAME "/Users/sisim/Documents/procon/procon2025/Matsumoto/fix_pos6/data/bfs_result6_m5.bin"

class BFS_result {
public:
  static FILE *fp;
  static char now_file_name[256];
  static vvvvv_ope bfs_result;

  static void open(char *file_name);
  static void close();
  static v_ope getOperation(int size, int progress, Pos target_pos, int type,
                            int idx);
  static int getOperationCount(int size, int progress, Pos target_pos,
                               int type);

private:
  static v_int idx_memo;

  static int calcOpeIndex(int size, int progress);
  static Pos calcTargetPos(int size, Pos target_pos);
};
#endif