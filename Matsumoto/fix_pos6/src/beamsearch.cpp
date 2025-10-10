#include "../inc/beamsearch.hpp"
#include "../inc/state.hpp"
#include "../inc/utilities.hpp"

// その他の必要な標準ライブラリ
#include <iostream>
#include <queue>
#include <utility>
#include <climits> // INT_MIN に必要
#include <algorithm> // std::max に必要

BeamNode::BeamNode(State *p) : p(p){
}

bool BeamNode::operator < (const BeamNode &other) const {
    return this->p->getScore() > other.p->getScore();
}

BeamSearch::BeamSearch(State *first, int width, int max_turn) : first(first), width(width), max_turn(max_turn) {
}

v_pair_ii BeamSearch::beamsearch() {
    State *state_mem[2];  // あらかじめ確保するStateクラスのメモリ
    Ent *ent_mem[2];      // あらかじめ確保するEntクラスのメモリ(Stateのfのent_memに該当)
    Pos *pos_mem[2];      // あらかじめ確保するPosクラスのメモリ(Stateのfのpos_memに該当)
    rep (i, 2) {
        state_mem[i] = new State[600000]{};
        ent_mem[i] = new Ent[600000 * this->first->f.size * this->first->f.size]{};
        pos_mem[i] = new Pos[600000 * this->first->f.size * this->first->f.size]{};
    }

    std::priority_queue<BeamNode> now_beam;
    state_mem[1][0].f.ent_mem = ent_mem[1];
    state_mem[1][0].f.pos_mem = pos_mem[1];
    this->first->getClone(state_mem[1]);
    now_beam.push(BeamNode(state_mem[1]));
    bool is_end = false;
    State *last_state = NULL;
    rep (i, this->max_turn) {
        std::priority_queue<BeamNode> next_beam;
        int debug = 0;

        int max_score = INT_MIN;
        int mem_idx = 0;  // あらかじめ確保したメモリのインデックス(あらかじめ確保したメモリにおいて次どの部分を使用するか)
        State *best_state = NULL;
        while (!now_beam.empty()){
            BeamNode tmp = now_beam.top();
            if (tmp.p->getScore() > max_score) {
                best_state = tmp.p;
            }
            max_score = std::max(max_score, tmp.p->getScore());
            if (tmp.p->isEnd()) {
                is_end = true;
                last_state = tmp.p;
                break;
            }
            now_beam.pop();
            rep (type, 3) {
                int next_cnt = tmp.p->getNextCount(type);  // 次の状態の数
                rep (j, next_cnt) {
                    State *next_state = state_mem[i % 2] + mem_idx;  // 次の状態(ポインタ)
                    next_state->f.ent_mem = ent_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                    next_state->f.pos_mem = pos_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                    tmp.p->getClone(next_state);  // コピーを取る
                    next_state->moveNextState(type, j);  // 次の状態に移行
                    mem_idx += addPriorityQueue(next_beam, BeamNode(next_state));  // 追加するかの判定と追加
                }
            }
            debug++;
        }
        if (is_end) break;
        now_beam = move(next_beam);
        printf("(%d, %lld, %d)\n", i, now_beam.size(), max_score);
        std::cout << std::flush;
        // cout << best_state->progress << endl;
        // best_state->f.printField();
    }

    v_pair_ii res;
    if (is_end) {
        res = last_state->log;
        last_state->f.printField();
    }
    rep (i, 2) {
        delete[] state_mem[i];
        delete[] ent_mem[i];
        delete[] pos_mem[i];
    }

    if (is_end) {
        return res;
    } else {
        std::cout << "Cannot solve !!" << std::endl;
        exit(1);
    }
}

/** 
 * priority_queueにサイズをビーム幅以下に保ったまま, 要素を追加する関数
 * priority_queueは結果が昇順に入っており, 評価は高いほうがいいものとする
 * 追加した場合はtrueを返す。
 */
bool BeamSearch::addPriorityQueue(std::priority_queue<BeamNode> &beam, BeamNode node) {
    if (beam.size() < this->width || node.p->progress == 1) {
        beam.push(node);
        return true;
    } else {
        BeamNode tmp = beam.top();
        // 現時点で最初のもの(スコアが一番悪いもの)と比べる
        if (node < tmp){
            beam.pop();
            beam.push(node);
            return true;
        } else {
            return false;
        }
    }
}