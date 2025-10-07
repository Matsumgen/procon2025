#ifndef BEAMSEARCH_HPP
#define BEAMSEARCH_HPP 0

// #include "all.hpp"
#include "utilities.hpp"
#include <iostream>
#include <queue>
using namespace std;

class BeamNode {
public:
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
  bool addPriorityQueue(priority_queue<BeamNode> &beam, BeamNode node);
};
#endif