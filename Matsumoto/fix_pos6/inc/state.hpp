#ifndef STATE_HPP
#define STATE_HPP 0

#include <utility>
#include <vector>

#include "field.hpp"
using namespace std;

typedef vector<int> v_int;
typedef vector<pair<int, int>> v_pair_ii;

/**
 * 状態管理クラス
 */
class State {
public:
  int x_hosei,
      y_hosei; // 相対座標(最初の盤面を基準とした時今の盤面がどれくらいずれているか)
  int rotate_hosei; // 最初の盤面を基準とした時,
                    // 今の盤面がどれくらい回転しているか
  Field f;          // 盤面の情報
  int progress; // ペアの進捗度(0: 全体回転, 1 ~ f.size: 上端を揃える, f.size +
                // 1 ~ 2 * f.size - 2: 右端を揃える)
  int score;     // 評価
  bool end_flag; // 最後の2x2の盤面に持って行ったかどうかを示す
  int last_type; // 最後に揃えた場所のタイプ
  v_pair_ii log; // これまでの操作履歴

  State();
  bool isEnd();
  int getScore();
  int getNextCount(int type);
  void moveNextState(int type, int idx);
  void getClone(State *out);
  void getAnswer(v_pair_ii &ans_log, int idx, v_ope &out);

private:
  v_ope getOperation(int type, int idx);
  Pos getBasePos(int type);
  Pos getLastPos(int type);
};

/**
 * ペアの揃え方のタイプ
 */
enum PairType {
  VERTICAL = 0, // 進行方向に対して垂直にそろえる
  HORIZON_0,    // 進行方向に対して平行にそろえる(より外側)
  HORIZON_1     // 進行方向に対して平行にそろえる(より内側)
};
#endif