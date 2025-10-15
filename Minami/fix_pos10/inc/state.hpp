#ifndef STATE_HPP
#define STATE_HPP 0

// #include "all.hpp"

#include "field.hpp"
#include <utility>
#include <vector>

#define TYPE_CNT1 3
#define TYPE_CNT2 5
#define TYPE_CNT3 6

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
        int ok_pair;
        int ope_sum;
        bool pile_dir;
        int last_pair_x;

        State();
        bool isEnd();
        int getScore();
        int getNextCount(int type);
        void moveNextState(int type, int idx);
        void getClone(State *out);
        void getAnswer(v_pair_ii &ans_log, int idx, v_ope &out);

    private:
        v_ope getOperation(int type, int idx);
        v_pos getBasePos(int type);
        v_pos getLastPos(int type);
        bool isOKType(int type);
        v_ope getToHorizonOpe();
};

/*enum PairType {
    VERTICAL = 0,
    HORIZON_0,
    HORIZON_1
};*/

enum LastType {
    FLAT = 0,
    OUTSIDE,
    LEFT,
    RIGHT
};

enum PileDir {
    HORIZON = 0,
    VERTICAL
};
#endif