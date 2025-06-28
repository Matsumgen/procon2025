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

    BeamNode *tmp = beamSearch(s, ope_list, getScore2, 256, 1000);
    cout << endl << tmp->score << " " << tmp->ope_list.size() << endl;
    v_ope ope_log = tmp->ope_list;
    eraseOpe(s, ope_log);
    return ope_log;
}

v_ope greedy(State &s, v_ope &ope_list) {
    v_ope ope_log;
    int now_pair_cnt = getPairCnt(s.ent_pos);
    int equal_cnt = 0;
    int loop_cnt = 0;
    while (1){
        shuffle(ope_list);
        Ope best_ope;
        int best_pair_cnt = -1;
        for (Ope ope : ope_list){
            State tmp_s = s;
            rotate(tmp_s, ope);
            // int tmp_pair_cnt = getPairCnt(tmp_s.ent_pos);
            int tmp_pair_cnt = getScore1(tmp_s);
            if (tmp_pair_cnt > best_pair_cnt){
                // cout << now_pair_cnt << " " << tmp_pair_cnt << endl;
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
        now_pair_cnt = getPairCnt(s.ent_pos);
        if (now_pair_cnt == N * N / 2) break;
    }
    cout << getScore1(s) << endl;
    return ope_log;
}

BeamNode* beamSearch(State &s, v_ope &ope_list, int (*score_func)(State &s), int depth, int beam_width){
    priority_queue<BeamNode2> now_beam;
    now_beam.push((BeamNode2){createNewBeamNode(s)});
    priority_queue<BeamNode2> next_beam;
    set<SetNode> appear_list;
    rep (i, depth) {
        // cout << i << " " << flush;
        printf("(%d, %lld) ", i, appear_list.size());
        cout << flush;
        shuffle(ope_list);
        while (!now_beam.empty()){
            BeamNode2 tmp = now_beam.top();
            if (isEnd(tmp.p->s)) return tmp.p;
            now_beam.pop();
            // vv_int sum_array = setOK2x2(tmp.p->s);
            // setSum(sum_array);
            for (Ope &ope : ope_list) {
                // if (ope.y % 2 == 0 && ope.x % 2 == 0 && ope.n % 2 == 0 && getSum(sum_array, ope.x / 2, ope.y / 2, (ope.x + ope.n) / 2 - 1, (ope.y + ope.n) / 2 - 1) == ope.n * ope.n / 4) continue;
                BeamNode *tmp2 = getBeamNodeCopy(tmp.p);
                rotate(tmp2->s, ope);
                tmp2->ope_list.push_back(ope);
                tmp2->score = -score_func(tmp2->s);
                uint8_t *p_array = stateToChar(tmp2->s);
                // addPriorityQueue(next_beam, (BeamNode2){tmp2}, beam_width);
                if (appear_list.find((SetNode){p_array}) == appear_list.end()) {
                    appear_list.insert((SetNode){p_array});
                    addPriorityQueue(next_beam, (BeamNode2){tmp2}, beam_width);
                } else {
                    // cout << "find" << " ";
                    delete p_array;
                    delete tmp2;
                }
            }
            delete tmp.p;
        }
        now_beam = move(next_beam);
    }
    // cout << endl;
    /*BeamNode *best_state = now_beam.top().p;
    v_ope ope_log;
    rep (i, (int)solve_data.size()) getNextField(&s, i, solve_data, bfs_result, best_state->idx_list[i], &ope_log);
    cout << "Best : " << best_state->ope_cnt << endl;
    return ope_log;*/
    while (now_beam.size() > 1){
        delete now_beam.top().p;
        now_beam.pop();
    }
    return now_beam.top().p;
}

void eraseOpe(State &first, v_ope &ope_log) {
    v_bool is_exe(ope_log.size(), true);
    for (int i = ope_log.size() - 1; i >= 0; i--) {
        State tmp = first;
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