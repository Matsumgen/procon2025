#ifndef BEAMSEARCH_HPP
#define BEAMSEARCH_HPP 0

#include "state.hpp" // Stateやv_pair_iiの型定義に必要
#include <iostream>
#include <queue>


class BeamNode {
public:
  BeamNode() = default; 
  BeamNode(State *p);
  State *p;
  bool operator<(const BeamNode &other) const;
};

class BeamSearch {
public:
  BeamSearch(State *first, int width, int max_turn);
  State *first;
  int width;
  int max_turn;
  v_pair_ii beamsearch();

private:
  bool addPriorityQueue(std::priority_queue<BeamNode> &beam, BeamNode node);
};
#endif