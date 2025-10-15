#ifndef BEAMSEARCH_HPP
#define BEAMSEARCH_HPP 0

// #include "all.hpp"

#include "utilities.hpp"
#include "state2.hpp"
#include <iostream>
#include <queue>

class BeamNode {
    public:
        BeamNode(State *p);
        State *p;
        bool operator < (const BeamNode &other) const;
};

class BeamSearch {
    public:
        BeamSearch(State *first, int width, int max_turn, State *first_state_mem, State2 *second_state_mem);
        State *first;
        int width;
        int max_turn;
        State *first_state_mem;
        State2 *second_state_mem;

        v_pair_ii beamsearch();

    private:
        bool addPriorityQueue(priority_queue<BeamNode> &beam, BeamNode node);
};


class BeamNode2 {
    public:
        BeamNode2(State2 *p);

        State2 *p;
        bool operator < (const BeamNode2 &other) const;
};

class BeamSearch2 {
    public:
        BeamSearch2(State2 *first, int width, int max_turn, int (*eval_func)(State2 *s));
        State2 *first;
        int width;
        int max_turn;
        v_ope all_ope;
        int (*eval_func)(State2 *s);

        bool beamsearch(v_ope &out);

    private:
        bool addPriorityQueue(priority_queue<BeamNode2> &beam, BeamNode2 node);
};
#endif