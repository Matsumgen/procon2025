#ifndef STATE_HPP
#define STATE_HPP 0

#include "field.hpp" // Pos, Ope, Fieldなどの型定義に必要
#include <utility>
#include <vector>

// 依存する型を定義した後に、関連する型エイリアスを定義
typedef std::vector<int> v_int;
typedef std::vector<std::pair<int, int>> v_pair_ii;

class State {
public:
  int x_hosei, y_hosei;
  int rotate_hosei;
  Field f;
  int progress;
  int score;
  bool end_flag;
  int last_type;
  v_pair_ii log;

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

enum PairType {
  VERTICAL = 0,
  HORIZON_0,
  HORIZON_1
};
#endif