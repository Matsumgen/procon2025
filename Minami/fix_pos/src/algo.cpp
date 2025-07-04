#include "../inc/all.hpp"

void convert_bfs_result(vvvvv_ope &bfs_result){
    if (N == 24) return;
    vvvvv_ope tmp_result = bfs_result;
    int T = N * N / 2 - 2;
    int X = 0, Y = 24 - N;
    int start_idx = tmp_result.size() - T;
    bfs_result.resize(T);
    rep (i, T){
        bfs_result[i] = vvvv_ope(N, vvv_ope(N));
        rep (y, N) rep (x, N){
            bfs_result[i][y][x].resize(tmp_result[i + start_idx][y + Y][x + X].size());
            rep (j, (int)bfs_result[i][y][x].size()){
                bfs_result[i][y][x][j].resize(tmp_result[i + start_idx][y + Y][x + X][j].size());
                rep (k, (int)bfs_result[i][y][x][j].size()){
                    Ope &tmp_ope = tmp_result[i + start_idx][y + Y][x + X][j][k];
                    bfs_result[i][y][x][j][k] = (Ope){tmp_ope.x - X, tmp_ope.y - Y, tmp_ope.n};
                }
            }
        }
    }
}

v_ope solve(State &s, vvvvv_ope &bfs_result, int start_clock, int max_time){
    v_ope ope_log;
    int bfs_idx = 0;
    v_solve_data solve_data;
    for (int i = 0; i < N / 2 - 1; i++){
        solve_data.push_back((SolveData){(Pos){0, i * 2}, 5, -1});
        set_solve_data_recode(i * 2, &bfs_idx, solve_data);
        set_solve_data_colum(N - 2 - i * 2, &bfs_idx, solve_data);
    }
    solve_data.push_back((SolveData){(Pos){}, 4, -1});

    /*
    rep (i, (int)solve_data.size()){
        int best_idx = dfs(s, solve_data, i, bfs_result, 0, 5).second;
        int move_cnt = getNextField(&s, i, solve_data, bfs_result, best_idx, &ope_log);
        // std::cout << i << " " << move_cnt << " " << best_idx << std::endl;
        // rep (i, N) rep (j, N){
        //     std::cout << s.field[i][j].val << " \n"[j == N - 1];
        // }
        // std::cout << std::endl;
        std::cout << i << " " << std::flush;
        if (move_cnt == -1) {
            std::cout << "Error" << " " << best_idx << std::endl;
            exit(1);
        }
    }
    std::cout << std::endl;
    return ope_log;
    */
    // return beamSearch(s, solve_data, bfs_result, 2000);

    
    State tmp_s = s;
    int move_cnt = 0;
    int best_move_cnt = INT_MAX;
    v_int best_idx_list(solve_data.size());
    rep (i, (int)solve_data.size()){
        BeamNode *tmp_node = beamSearch(tmp_s, i, solve_data, bfs_result, 2000);
        int tmp_move_cnt = move_cnt + tmp_node->ope_cnt;
        if (tmp_move_cnt < best_move_cnt){
            best_move_cnt = tmp_move_cnt;
            rep (j, tmp_node->idx_list.size()) best_idx_list[i + j] = tmp_node->idx_list[j];
        }
        delete tmp_node;
        move_cnt += getNextField(&tmp_s, i, solve_data, bfs_result, best_idx_list[i], NULL);
        cout << i << " " << best_move_cnt << " " << tmp_move_cnt << endl;
        if ((double)(clock() - start_clock) / CLOCKS_PER_SEC > max_time) break;
    }
    cout << endl;
    cout << move_cnt << " " << best_move_cnt << endl;
    /*v_ope ope_list;
    rep (i, (int)solve_data.size()) getNextField(&s, i, solve_data, bfs_result, best_idx_list[i], &ope_list);
    return ope_list;*/
    return getAnswer(s, solve_data, bfs_result, best_idx_list);
}

void set_solve_data_recode(int recode, int *bfs_idx, v_solve_data &solve_data){
    int w = N - recode;
    int x, y;
    y = recode;
    Pos base_ent_pos;

    // 最後の2列以外は縦に揃える
    for (x = 0; x < w - 2; x++){
        base_ent_pos = (Pos){x, y};
        solve_data.push_back((SolveData){base_ent_pos, 0, (*bfs_idx)++});
    }

    // 最後の2列
    x = w - 2;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 1, (*bfs_idx)++});

    y++;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 1, (*bfs_idx)++});
}

void set_solve_data_colum(int colum, int *bfs_idx, v_solve_data &solve_data){
    int h = colum;
    int x, y;
    x = colum + 1;
    Pos base_ent_pos;

    // 最後の2行以外は縦に揃える
    for (y = N - colum; y < N - 2; y++){
        base_ent_pos = (Pos){x, y};
        solve_data.push_back((SolveData){base_ent_pos, 2, (*bfs_idx)++});
    }

    // 最後の2列
    y = N - 2;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 3, (*bfs_idx)++});
    
    x--;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 3, (*bfs_idx)++});
}

int getNextFieldCnt(State *s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result){
    SolveData &now_data = solve_data[cnt];
    int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;  // 基準となるエンティティの場所
    int val;
    Pos another_ent_pos, diff;  // ペアのエンティティの場所と基準との差分
    set_pos(&now_data.base_ent_pos, *s, &val, &another_ent_pos, &diff);

    /**
     * 0: 下
     * 1: 右(下経由)
     * 2: 左
     * 3: 下(横経由)
     * 4: 最後の調整
     * 5: 全体回転
     */
    switch (now_data.type){
    case 0:
    case 1:
        return  (diff == (Pos){1, 0} || diff == (Pos){0, 1}) ? 1 : bfs_result[now_data.bfs_idx][another_ent_pos.y][another_ent_pos.x].size();
        break;
    case 2:
    case 3:
        return  (diff == (Pos){0, 1} || diff == (Pos){-1, 0}) ? 1 : bfs_result[now_data.bfs_idx][another_ent_pos.y][another_ent_pos.x].size();
        break;
    case 4:
        return 1;
        break;
    case 5:
        return 4;
        break;
    }
    return -1;
}

int getNextField(State *s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result, int idx, v_ope *ope_log){
    SolveData &now_data = solve_data[cnt];
    int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;  // 基準となるエンティティの場所
    int val;
    Pos another_ent_pos, diff;  // ペアのエンティティの場所と基準との差分
    set_pos(&now_data.base_ent_pos, *s, &val, &another_ent_pos, &diff);

    auto exe_bfs = [&](){
        if (idx >= bfs_result[now_data.bfs_idx][another_ent_pos.y][another_ent_pos.x].size()) return -1;
        v_ope &tmp_log = bfs_result[now_data.bfs_idx][another_ent_pos.y][another_ent_pos.x][idx];
        for (Ope &ope : tmp_log){
            rotate(*s, ope);
            if (ope_log != NULL) ope_log->push_back(ope);
        }
        return (int)tmp_log.size();
    };

    /**
     * 0: 下
     * 1: 右(下経由)
     * 2: 左
     * 3: 下(横経由)
     * 4: 最後の調整
     */
    switch (now_data.type){
    case 0:
        if (diff == (Pos){1, 0}){
            if (idx != 0) return -1;
            // 1つ右なら3回回転させて揃える
            rep (j, 3) {
                Ope ope = (Ope){x, y, 2};
                rotate(*s, ope);
                if (ope_log != NULL) ope_log->push_back(ope);
            }
            return 3;
        } else if (diff != (Pos){0, 1}){
            // 目的の場所にないならbfs            
            return exe_bfs();
        } else {
            // 何もしない
            return idx == 0 ? 0 : -1;
        }
        break;
    case 1:
        if (diff != (Pos){1, 0}){
            // 下に揃えてから回転
            int move_cnt = 0;
            if (diff == (Pos){0, 1}){
                // 中継地点にあるなら何もしない
                if (idx != 0) return -1;
            } else {
                // bfs
                move_cnt = exe_bfs();
                if (move_cnt == -1) return -1;
            }
            // 中継地点 -> 最終目的地
            Ope ope = (Ope){x, y, 2};
            rotate(*s, ope);
            if (ope_log != NULL) ope_log->push_back(ope);
            return move_cnt + 1;
        } else {
            // 最終目的地にあるなら何もしない
            return idx == 0 ? 0 : -1;
        }
        break;
    case 2:
        if (diff == (Pos){0, 1}){
            // 1つ下なら3回回転させて揃える
            if (idx != 0) return -1;
            Ope ope = (Ope){x - 1, y, 2};
            rep (j, 3){
                rotate(*s, ope);
                if (ope_log != NULL) ope_log->push_back(ope);
            }
            return 3;
        } else if (diff != (Pos){-1, 0}){
            // 目的の場所にないならbfs
            return exe_bfs();
        } else{
            // 何もしない
            return idx == 0 ? 0 : -1;
        }
        break;
    case 3:
        if (diff != (Pos){0, 1}){
            // 左に揃えてから回転
            int move_cnt = 0;
            if (diff == (Pos){-1, 0}){
                // 中継地点にあるなら何もしない
                if (idx != 0) return -1;
            } else {
                // bfs
                move_cnt = exe_bfs();
                if (move_cnt == -1) return -1;
            }
            // 中継地点 -> 最終目的地
            Ope ope = (Ope){x - 1, y, 2};
            rotate(*s, ope);
            if (ope_log != NULL) ope_log->push_back(ope);
            return move_cnt + 1;
        } else {
            // 最終目的地にあるなら何もしない
            return idx == 0 ? 0 : -1;
        }
        break;
    case 4:
        if (idx != 0) return -1;
        if (s->field[N - 1][0].val == s->field[N - 2][1].val){
            v_ope last_ope = {(Ope){1, N - 2, 2}, (Ope){2, N - 4, 2}, (Ope){0, N - 4, 3}};
            for (Ope ope : last_ope){
                rotate(*s, ope);
                if (ope_log != NULL) ope_log->push_back(ope);
            }
            return 3;
        }else{
            return 0;
        }
        break;
    case 5:
        if (idx >= 4) return -1;
        rep (i, idx){
            Ope ope = (Ope){x, y, N - y};
            rotate(*s, ope);
            if (ope_log != NULL) ope_log->push_back(ope);
        }
        return 0;
        break;
    }
    return -1;
}

v_ope getAnswer(State &s, v_solve_data &solve_data, vvvvv_ope &bfs_result, v_int &idx_list){
    const int change_range_x[4] = {-2, 2, 2, -2};
    const int change_range_y[4] = {2, 2, -2, -2};
    v_ope ans;
    int rotation = 0;
    int x_range[2] = {0, N - 1}, y_range[2] = {0, N - 1};
    int n = N;
    int tmp_y = 0;
    rep (i, (int)solve_data.size()){
        if (solve_data[i].type == 5){
            if (i != 0){
                x_range[change_range_x[rotation] < 0] += change_range_x[rotation];
                y_range[change_range_y[rotation] < 0] += change_range_y[rotation];
                n -= 2;
                tmp_y += 2;
            }
            rotation = (rotation + idx_list[i]) % 4;
            getNextField(&s, i, solve_data, bfs_result, idx_list[i], NULL);
        }else{
            v_ope tmp;
            getNextField(&s, i, solve_data, bfs_result, idx_list[i], &tmp);
            for (Ope ope : tmp){
                ope.y -= tmp_y;
                switch (rotation){
                case 0:
                    ans.push_back((Ope){x_range[0] + ope.x, y_range[0] + ope.y, ope.n});
                    break;
                case 1:
                    ope.x += ope.n - 1;
                    ans.push_back((Ope){x_range[0] + ope.y, y_range[0] + n - ope.x - 1, ope.n});
                    break;
                case 2:
                    ope.x += ope.n - 1;
                    ope.y += ope.n - 1;
                    ans.push_back((Ope){x_range[0] + n - ope.x - 1, y_range[0] + n - ope.y - 1, ope.n});
                    break;
                case 3:
                    ope.y += ope.n - 1;
                    ans.push_back((Ope){x_range[0] + n - ope.y - 1, y_range[0] + ope.x, ope.n});
                    break;
                }
            }
        }
    }
    return ans;
}

pair<int, int> dfs(State &s, v_solve_data &solve_data, int cnt, vvvvv_ope &bfs_result, int depth, int max_depth){
    if (cnt >= solve_data.size()) return make_pair(0, 0);
    if (depth == max_depth) {
        State tmp_s = s;
        return make_pair(getNextField(&tmp_s, cnt, solve_data, bfs_result, 0, NULL), 0);
    }
    
    int best_idx = -1;
    int best_score = INT_MAX;
    int rep_cnt = getNextFieldCnt(&s, cnt, solve_data, bfs_result);
    if (depth == 0 && rep_cnt == 1) return make_pair(0, 0);
    rep (i, rep_cnt){
        State next_s = s;
        int move_cnt = getNextField(&next_s, cnt, solve_data, bfs_result, i, NULL);
        if (move_cnt == -1){
            if (i == 0) {
                std::cout << "Error2" << std::endl;
                rep (i, N) rep (j, N) std::cout << s.field[i][j].val << "\t\n"[j == N - 1];
                rep (i, (int)s.ent_pos.size()){
                    printf("%d : (%d, %d), (%d, %d)\n", i, s.ent_pos[i][0].x, s.ent_pos[i][0].y, s.ent_pos[i][1].x, s.ent_pos[i][1].y);
                }
                cout << cnt << " " << depth << " " << solve_data[cnt].base_ent_pos.x << " " << solve_data[cnt].base_ent_pos.y << endl;
                exit(1);
            }
            break;
        }
        int tmp_score = move_cnt + dfs(next_s, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
        if (tmp_score < best_score){
            best_score = tmp_score;
            best_idx = i;
        }
        if (best_idx == -1) {
            std::cout << "Error3" << " " << tmp_score << " " << cnt << " " << depth << " " << move_cnt << std::endl;
            rep (i, N) rep (j, N) std::cout << s.field[i][j].val << " \n"[j == N - 1];
            exit(1);
        }
    }
    return make_pair(best_score, best_idx);
}

BeamNode* beamSearch(State &s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result, int beam_width){
    priority_queue<BeamNode2> now_beam;
    now_beam.push((BeamNode2){createNewBeamNode(s)});
    for (int i = cnt; i < (int)solve_data.size(); i++){
        cout << i << " " << flush;
        priority_queue<BeamNode2> next_beam;
        while (!now_beam.empty()){
            BeamNode2 tmp = now_beam.top();
            now_beam.pop();
            int rep_cnt = getNextFieldCnt(&(tmp.p->s), i, solve_data, bfs_result);
            rep (j, rep_cnt){
                BeamNode *tmp2 = getBeamNodeCopy(tmp.p);
                int move_cnt = getNextField(&(tmp2->s), i, solve_data, bfs_result, j, NULL);
                tmp2->ope_cnt += move_cnt;
                tmp2->idx_list.push_back(j);
                addPriorityQueue(next_beam, (BeamNode2){tmp2}, solve_data[i].type == 5 ? INT_MAX : beam_width);
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