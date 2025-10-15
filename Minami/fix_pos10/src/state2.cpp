#include "../inc/all.hpp"

State2::State2() : log(v_ope(0)), score(0), end_flag(false) {
}

bool State2::isEnd() {
    rep (i, this->f.size * this->f.size / 2) {
        if (manhattan(this->f.getEntPos(i, 0), this->f.getEntPos(i, 1)) != 1) return false;
    }
    return true;
}

int State2::getScore() {
    return this->score;
}

void State2::getClone(State2 *out) {
    this->f.getClone(&out->f);
    out->score = this->getScore();
    out->end_flag = this->end_flag;
    out->log = this->log;
}