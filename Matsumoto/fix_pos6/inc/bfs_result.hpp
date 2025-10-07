#ifndef BFS_RESULT_HPP
#define BFS_RESULT_HPP 0
using namespace std;
#include "state.hpp"

#include <cstdio>

#define BFS_RESULT_FILE_NAME "../data/bfs_result6_m5.bin"

/**
 * BFSの結果を管理するクラス
 * 基本的にインスタンスを作成せず静的メソッドを利用する
 */
class BFS_result {
public:
  static FILE *fp;
  static char now_file_name[256];
  static vvvvv_ope
      bfs_result; // bfsの結果を格納している配列(bfs_result[i][j][k][l]:
                  // i番目にタイプjでマスkを目的地に持っていくl番目の手順リスト)

  static void open(char *file_name);
  static void close();
  static v_ope getOperation(int size, int progress, Pos target_pos, int type,
                            int idx);
  static int getOperationCount(int size, int progress, Pos target_pos,
                               int type);

private:
  static v_int
      idx_memo; // 現在の盤面のサイズと揃えた進捗からbfs_resultに対応するインデックスを求める際のメモ

  static int calcOpeIndex(int size, int progress);
  static Pos calcTargetPos(int size, Pos target_pos);
};
#endif