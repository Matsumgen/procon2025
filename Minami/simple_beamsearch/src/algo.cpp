#include "../inc/all.hpp"

v_ope solve(State &s){  
    int now_pair_cnt = getScore1(s);
    v_ope ope_list;
    for (int size = 2; size < N; size++){
        rep (y, N - size + 1) rep (x, N - size + 1){
            ope_list.push_back((Ope){x, y, size});
        }
    }
    cout << ope_list.size() << endl;
    
    /*
    // コピーの時間テスト
    rep (i, 10000) {
        for (Ope &ope : ope_list) {
            State tmp_s = s.getClone();
            rotate(tmp_s, ope);
            int tmp = getScore2(s);
            tmp++;
        }
        if (i % 100 == 0) cout << "." << flush;
    }
    exit(0);*/
    
    BeamNode *tmp = beamSearch(s, ope_list, getScore2, 256, 1000);
    cout << endl << tmp->score << " " << tmp->ope_list.size() << endl;
    v_ope ope_log = tmp->ope_list;
    eraseOpe(s, ope_log);

    /*
    // bfs
    v_ope ope_log;
    while (1) {
        int now_score = getScore2(s);
        v_ope res = bfs(s, ope_list, getScore2, min(50, now_score + 1));
        for (Ope &ope : res) {
            rotate(s, ope);
            ope_log.push_back(ope);
        }
        cout << getScore2(s) << " " << res.size() << endl;
        if (isEnd(s)) break;
    }*/
    return ope_log;
}

v_ope greedy(State &s, v_ope &ope_list) {
    v_ope ope_log;
    int now_pair_cnt = getScore2(s);
    int equal_cnt = 0;
    int loop_cnt = 0;
    while (1){
        shuffle(ope_list);
        Ope best_ope;
        int best_pair_cnt = -1;
        for (Ope ope : ope_list){
            State tmp_s = s;
            rotate(tmp_s, ope);
            int tmp_pair_cnt = getScore1(tmp_s);
            if (tmp_pair_cnt > best_pair_cnt){
                best_pair_cnt = tmp_pair_cnt;
                best_ope = ope;
            }
        }
        loop_cnt++;
        if (loop_cnt == 1000) break;
        if (best_pair_cnt == now_pair_cnt) {
            equal_cnt++;
            if (equal_cnt == 50) break;
        } else {
            equal_cnt = 0;
        }
        if (loop_cnt % 10 == 0) cout << loop_cnt << " " << best_pair_cnt << endl;
        ope_log.push_back(best_ope);
        rotate(s, best_ope);
        now_pair_cnt = getScore2(s);
        if (now_pair_cnt == N * N / 2) break;
    }
    cout << getScore1(s) << endl;
    return ope_log;
}

BeamNode* beamSearch(State &s, v_ope &ope_list, int (*score_func)(State &s), int depth, int beam_width){
    BeamNode *beam_node_mem[2];
    Ent *ent_mem[2];
    Pos *pos_mem[2];
    rep (i, 2) {
        beam_node_mem[i] = new BeamNode[600000]{};
        ent_mem[i] = new Ent[600000 * s.size * s.size]{};
        pos_mem[i] = new Pos[600000 * s.size * s.size]{};
    }

    priority_queue<BeamNode2> now_beam;
    beam_node_mem[1]->init(s);
    now_beam.push((BeamNode2){beam_node_mem[1]});
    priority_queue<BeamNode2> next_beam;
    // set<SetNode> appear_list;
    int debug = 0;
    // uint8_t *big_array = new uint8_t[1LL << 31]{};
    // uint8_t *p_big_array = big_array;
    rep (i, depth) {
        shuffle(ope_list);
        int max_score = 0;
        int mem_idx = 0;
        while (!now_beam.empty()){
            BeamNode2 tmp = now_beam.top();
            max_score = max(max_score, -tmp.p->score);
            if (i == 95 && mem_idx == 0) {
                tmp.p->s.printState();
                tmp.p->s.printPosState();
            }
            if (isEnd(tmp.p->s)) {
                BeamNode *res = new BeamNode();
                tmp.p->getClone(*res);
                // delete[] big_array;
                rep (i, 2) {
                    delete[] beam_node_mem[i];
                    delete[] ent_mem[i];
                    delete[] pos_mem[i];
                }
                return res;
            }
            now_beam.pop();
            // vv_int sum_array = setOK2x2(tmp.p->s);
            // setSum(sum_array);
            for (Ope &ope : ope_list) {
                // if (ope.y % 2 == 0 && ope.x % 2 == 0 && ope.n % 2 == 0 && getSum(sum_array, ope.x / 2, ope.y / 2, (ope.x + ope.n) / 2 - 1, (ope.y + ope.n) / 2 - 1) == ope.n * ope.n / 4) continue;
                BeamNode *tmp2 = beam_node_mem[i % 2] + mem_idx;
                tmp.p->getClone(*tmp2, ent_mem[i % 2] + mem_idx * (s.size * s.size), pos_mem[i % 2] + mem_idx * (s.size * s.size));
                rotate(tmp2->s, ope);
                tmp2->ope_list.push_back(ope);
                tmp2->score = -score_func(tmp2->s);
                mem_idx += addPriorityQueue(next_beam, (BeamNode2){tmp2}, beam_width);
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
        now_beam = move(next_beam);
        printf("(%d, %lld, %d) ", i, now_beam.size(), max_score);
        cout << flush;
    }
    while (now_beam.size() > 1){
        now_beam.pop();
    }

    BeamNode *res = new BeamNode();
    now_beam.top().p->getClone(*res);
    // delete[] big_array;
    rep (i, 2) {
        delete[] beam_node_mem[i];
        delete[] ent_mem[i];
        delete[] pos_mem[i];
    }
    return res;
}

v_ope bfs(State &s, v_ope &ope_list, int (*score_func)(State &s), int goal_score) {
    queue<BeamNode2> todo;
    set<SetNode> appear_list;
    static uint8_t *big_array = new uint8_t[1LL << 31]{};
    uint8_t *p_big_array = big_array;
    int insert_cnt = 0, delete_cnt = 0;
    BeamNode *first_node = createNewBeamNode(s);
    first_node->score = score_func(s);
    todo.push((BeamNode2){first_node});
    v_ope res;
    bool is_finish = false;
    int max_dist = 0;
    int cnt = 0;
    while (!todo.empty()) {
        BeamNode2 tmp = todo.front();
        todo.pop();
        if (tmp.p->ope_list.size() > max_dist) {
            max_dist = tmp.p->ope_list.size();
            cout << ". " << todo.size() << " " << appear_list.size() << endl;
            cnt = 0;
        }
        cnt++;
        if (cnt % 1000 == 0) cout << "." << flush;
        if (is_finish) {
            delete tmp.p;
            continue;
        }
        for (Ope &ope : ope_list) {
            BeamNode *tmp2 = getBeamNodeCopy(tmp.p);
            rotate(tmp2->s, ope);
            tmp2->ope_list.push_back(ope);
            tmp2->score = score_func(tmp2->s);
            if (tmp2->score < tmp.p->score) {
                delete tmp2;
                delete_cnt++;
                continue;
            }
            if (tmp2->score >= goal_score) {
                res = tmp2->ope_list;
                is_finish = true;
                delete tmp2;
                continue;
            }
            // todo.push((BeamNode2){tmp2});
            stateToChar(tmp2->s, p_big_array);
            if (appear_list.find((SetNode){p_big_array}) == appear_list.end()) {
                appear_list.insert((SetNode){p_big_array});
                todo.push((BeamNode2){tmp2});
                insert_cnt++;
                p_big_array += N * N;
            } else {
                delete tmp2;
                delete_cnt++;
            }
        }
        delete tmp.p;
    }
    cout << endl;
    return res;
}

void eraseOpe(State &first, v_ope &ope_log) {
    v_bool is_exe(ope_log.size(), true);
    for (int i = ope_log.size() - 1; i >= 0; i--) {
        State tmp;
        first.getClone(tmp);
        is_exe[i] = false;
        exe_ope(tmp, ope_log, is_exe);
        is_exe[i] = !isEnd(tmp);
    }
    int idx = 0;
    rep (i, (int)ope_log.size()) {
        if (is_exe[i]) {
            if (idx != i) ope_log[idx] = ope_log[i];
            idx++;
        }
    }
    ope_log.erase(ope_log.begin() + idx, ope_log.end());
}