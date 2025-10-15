#ifndef STATE2_HPP
#define STATE2_HPP 0

// #include "all.hpp"
#include <utility>
#include <vector>

#include "field.hpp"

#define MAX_STATE2_COUNT 10000

class State2 {
    public:
        Field f;
        int score;
        bool end_flag;
        v_ope log;

        State2();
        bool isEnd();
        int getScore();
        // void moveNextState(int type, int idx);
        void getClone(State2 *out);

    private:
};
#endif