#include "../inc/all.hpp"

BeamNode::BeamNode() {
}

BeamNode::BeamNode(State *p) : p(p){
}

bool BeamNode::operator < (const BeamNode &other) const {
    return this->p->getScore() > other.p->getScore();
}

BeamSearch::BeamSearch(State *first, int width, int max_turn, State *first_state_mem, State2 *second_state_mem) : first(first), width(width), max_turn(max_turn), first_state_mem(first_state_mem), second_state_mem(second_state_mem) {
}

v_pair_ii BeamSearch::beamsearch() {
    State *state_mem[2];
    Ent *ent_mem[2];
    Pos *pos_mem[2];
    rep (i, 2) {
        state_mem[i] = new State[600000]{};
        ent_mem[i] = new Ent[600000 * this->first->f.size * this->first->f.size]{};
        pos_mem[i] = new Pos[600000 * this->first->f.size * this->first->f.size]{};
    }

    int second_state_idx = 0;
    if (this->first->f.size <= 10) {
        this->first_state_mem[second_state_idx].score = first->getScore();
        this->first_state_mem[second_state_idx].log = first->log;
        this->first_state_mem[second_state_idx].rotate_hosei = first->rotate_hosei;
        this->first_state_mem[second_state_idx].x_hosei = first->x_hosei;
        this->first_state_mem[second_state_idx].y_hosei = first->y_hosei;
        this->first_state_mem[second_state_idx].f.size = first->f.size;
        first->f.getClone(&(this->second_state_mem[second_state_idx].f));
        second_state_idx++;
    }

    priority_queue<BeamNode> now_beam;
    state_mem[1][0].f.ent_mem = ent_mem[1];
    state_mem[1][0].f.pos_mem = pos_mem[1];
    this->first->getClone(state_mem[1]);
    now_beam.push((BeamNode){state_mem[1]});
    bool is_end = false;
    int best_final_score = INT_MIN;
    v_pair_ii res;
    rep (i, this->max_turn) {
        if (now_beam.empty()) break;
        priority_queue<BeamNode> next_beam;
        int debug = 0;

        int max_score = INT_MIN;
        int mem_idx = 0;
        State *best_state = NULL;

        int beam_cnt = now_beam.size();
        BeamNode tmp_now_beam[beam_cnt];
        rep (j, beam_cnt) {
            tmp_now_beam[j] = now_beam.top();
            now_beam.pop();
        }

        fsdb::Routes tmp_route[beam_cnt];
        #pragma omp parallel for
        rep (j, beam_cnt) {
            if (tmp_now_beam[j].p->end_flag || !tmp_now_beam[j].p->isOKType(15)) continue;
            tmp_route[j] = fsdb::getOperation(tmp_now_beam[j].p);
            tmp_now_beam[j].p->route = &tmp_route[j];
            cout << "." << flush;
        }
        cout << endl;

        rep (j, beam_cnt) {
            // BeamNode tmp = now_beam.top();
            BeamNode tmp = tmp_now_beam[j];
            now_beam.pop();
            if (tmp.p->getScore() > max_score) {
                best_state = tmp.p;
                max_score = tmp.p->getScore();
            }
            if (tmp.p->isEnd()) {
                is_end = true;
                if (tmp.p->getScore() > best_final_score) {
                    best_final_score = tmp.p->getScore();
                    res = tmp.p->log;
                }
                continue;
            }
            if (tmp.p->f.size == 10 && tmp.p->progress == 0 && second_state_idx < MAX_STATE2_COUNT) {
                this->first_state_mem[second_state_idx].score = tmp.p->getScore();
                this->first_state_mem[second_state_idx].log = tmp.p->log;
                this->first_state_mem[second_state_idx].rotate_hosei = tmp.p->rotate_hosei;
                this->first_state_mem[second_state_idx].x_hosei = tmp.p->x_hosei;
                this->first_state_mem[second_state_idx].y_hosei = tmp.p->y_hosei;
                this->first_state_mem[second_state_idx].f.size = tmp.p->f.size;
                tmp.p->f.getClone(&(this->second_state_mem[second_state_idx].f));
                second_state_idx++;
            }

            // int priority_type[6] = {15, 3, 4, 0, 1, 2};
            rep (type, TYPE_CNT1 + TYPE_CNT2 + TYPE_CNT3 + 2) {
            // rep (t, 6) {
                // int type = priority_type[t];
                // cout << j << " " << t << " " << type << " " << flush;
                int next_cnt = tmp.p->getNextCount(type);
                // cout << next_cnt << endl;
                rep (k, next_cnt) {
                    State *next_state = state_mem[i % 2] + mem_idx;
                    next_state->f.ent_mem = ent_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                    next_state->f.pos_mem = pos_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                    tmp.p->getClone(next_state);
                    next_state->moveNextState(type, k);
                    mem_idx += addPriorityQueue(next_beam, BeamNode(next_state));
                }
                // if (next_cnt != 0) break;
            }
            debug++;
        }
        // if (is_end) break;
        now_beam = move(next_beam);
        printf("(%d, %d, %d, %d)\n", i, (int)now_beam.size(), mem_idx, max_score);
        cout << flush;
        // cout << best_state->progress << " " << best_state->last_type << endl;
        // rep (j, best_state->log.size()) {
        //     printf("(%d, %d), ", best_state->log[j].first, best_state->log[j].second);
        // }
        // cout << endl;
        // best_state->f.printField();
    }
    cout << "mem_idx: " << second_state_idx << endl;

    rep (i, 2) {
        delete[] state_mem[i];
        delete[] ent_mem[i];
        delete[] pos_mem[i];
    }

    if (is_end) {
        return res;
    } else {
        cout << "Cannot solve !!" << endl;
        exit(1);
    }
}

bool BeamSearch::addPriorityQueue(priority_queue<BeamNode> &beam, BeamNode node) {
    // if (beam.size() < this->width || node.p->progress == 1) {
    if (beam.size() < this->width) {
        beam.push(node);
        return true;
    } else {
        BeamNode tmp = beam.top();
        if (node < tmp){
            beam.pop();
            beam.push(node);
            return true;
        } else {
            return false;
        }
    }
}


BeamNode2::BeamNode2(State2 *p) : p(p){
}

bool BeamNode2::operator < (const BeamNode2 &other) const {
    return this->p->getScore() > other.p->getScore();
}

BeamSearch2::BeamSearch2(State2 *first, int width, int max_turn, int (*eval_func)(State2 *s)) : first(first), width(width), max_turn(max_turn), eval_func(eval_func) {
    for (short size = 2; size < first->f.size; size++){
        for (short y = 0; y < first->f.size - size + 1; y++) for (short x = 0; x < first->f.size - size + 1; x++){
            this->all_ope.push_back((Ope){x, y, size});
        }
    }
    // cout << this->all_ope.size() << endl;
}

bool BeamSearch2::beamsearch(v_ope &out) {
    State2 *state_mem[2];
    Ent *ent_mem[2];
    Pos *pos_mem[2];
    rep (i, 2) {
        state_mem[i] = new State2[600000]{};
        ent_mem[i] = new Ent[600000 * this->first->f.size * this->first->f.size]{};
        pos_mem[i] = new Pos[600000 * this->first->f.size * this->first->f.size]{};
    }

    priority_queue<BeamNode2> now_beam;
    state_mem[1][0].f.ent_mem = ent_mem[1];
    state_mem[1][0].f.pos_mem = pos_mem[1];
    this->first->getClone(state_mem[1]);
    now_beam.push((BeamNode2){state_mem[1]});

    // set<SetNode> appear_list;
    // uint8_t *big_array = new uint8_t[1LL << 31]{};
    // uint8_t *p_big_array = big_array;
    bool is_end = false;
    int best_final_score = INT_MIN;
    rep (i, this->max_turn) {
        if (now_beam.empty()) break;
        shuffle(this->all_ope);
        priority_queue<BeamNode2> next_beam;
        int max_score = INT_MIN;
        int mem_idx = 0;
        State2 *best_state = NULL;
        while (!now_beam.empty()) {
            BeamNode2 tmp = now_beam.top();
            now_beam.pop();
            if (tmp.p->getScore() > max_score) {
                best_state = tmp.p;
                max_score = tmp.p->getScore();
            }
            if (tmp.p->getScore() == 50 && !tmp.p->isEnd()) {
                tmp.p->f.printField();
            }
            if (tmp.p->isEnd()) {
                is_end = true;
                out = tmp.p->log;
                break;
            }

            // vv_int sum_array = setOK2x2(tmp.p->s);
            // setSum(sum_array);
            for (Ope &ope : this->all_ope) {
                // if (ope.y % 2 == 0 && ope.x % 2 == 0 && ope.n % 2 == 0 && getSum(sum_array, ope.x / 2, ope.y / 2, (ope.x + ope.n) / 2 - 1, (ope.y + ope.n) / 2 - 1) == ope.n * ope.n / 4) continue;
                State2 *next_state = state_mem[i % 2] + mem_idx;
                next_state->f.ent_mem = ent_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                next_state->f.pos_mem = pos_mem[i % 2] + mem_idx * (this->first->f.size * this->first->f.size);
                tmp.p->getClone(next_state);
                // next_state->moveNextState(type, j);
                next_state->f.rotate(ope);
                next_state->log.push_back(ope);
                next_state->score = this->eval_func(next_state);
                mem_idx += addPriorityQueue(next_beam, BeamNode2(next_state));

                /*stateToChar(tmp2->s, p_big_array);
                if (appear_list.find((SetNode){p_big_array}) == appear_list.end()) {
                    appear_list.insert((SetNode){p_big_array});
                    addPriorityQueue(next_beam, (BeamNode2){tmp2}, beam_width);
                    p_big_array += N * N;
                } else {
                    // cout << "find" << " ";
                    // delete p_array;
                    // delete tmp2;
                }*/
            }
        }
        if (is_end) break;
        now_beam = move(next_beam);
        printf("(%d, %d, %d, %d)\n", i, (int)now_beam.size(), mem_idx, max_score);
        cout << flush;
        // cout << best_state->progress << " " << best_state->last_type << endl;
        // rep (j, best_state->log.size()) {
        //     printf("(%d, %d), ", best_state->log[j].first, best_state->log[j].second);
        // }
        // cout << endl;
        // best_state->f.printField();
    }

    rep (i, 2) {
        delete[] state_mem[i];
        delete[] ent_mem[i];
        delete[] pos_mem[i];
    }

    return is_end;
}

bool BeamSearch2::addPriorityQueue(priority_queue<BeamNode2> &beam, BeamNode2 node) {
    // if (beam.size() < this->width || node.p->progress == 1) {
    if (beam.size() < this->width) {
        beam.push(node);
        return true;
    } else {
        BeamNode2 tmp = beam.top();
        if (node < tmp){
            beam.pop();
            beam.push(node);
            return true;
        } else {
            return false;
        }
    }
}