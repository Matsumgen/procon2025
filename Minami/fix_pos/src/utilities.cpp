#include "../inc/all.hpp"

void input_data(State &s, char* file_name){
    if (file_name == NULL){
        cin >> N;
        s.field = vv_ent(N, v_ent(N));
        s.ent_pos = vv_pos(N * N / 2, v_pos(0));
#ifndef IS_DEBUG_A
        rep (i, N) rep (j, N){
            int val;
            cin >> val;
            s.field[i][j].val = val;
            s.field[i][j].num = s.ent_pos[val].size();
            s.ent_pos[val].push_back((Pos){j, i});
        }
#else
        rep (i, N) rep (j, N){
            int val = (i * N + j) / 2;
            field[i][j].val = val;
            field[i][j].num = ent_pos[val].size();
            ent_pos[val].push_back((Pos){j, i});
        }
#endif
    }else{
        input_file(s, file_name);
    }
}

void input_file(State &s, char* file_name){
    /*
    int cnt = 0;
    int* tmp_array = (int*)malloc(sizeof(int) * 1024);
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL){
        cout << "File Open Error" << endl;
        exit(1);
    }
    while (fscanf(fp, "%d", tmp_array + cnt) != EOF) cnt++;
    fclose(fp);
    
    N = (int)sqrt(cnt);
    cnt = 0;
    field = vv_ent(N, v_ent(N));
    ent_pos = vv_pos(N * N / 2);
    rep (i, N) rep (j, N){
        int val = tmp_array[i * N + j];
        field[i][j].val = val;
        field[i][j].num = ent_pos[val].size();
        ent_pos[val].push_back((Pos){j, i});
    }
    free(tmp_array);
    */
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL){
        cout << "File Open Error" << endl;
        exit(1);
    }
    fscanf(fp, "%d", &N);
    s.field = vv_ent(N, v_ent(N));
    s.ent_pos = vv_pos(N * N / 2);
    rep (i, N) rep (j, N){
        int val;
        fscanf(fp, "%d", &val);
        s.field[i][j].val = val;
        s.field[i][j].num = s.ent_pos[val].size();
        s.ent_pos[val].push_back((Pos){j, i});
    }
    fclose(fp);
}

void input_bfs_result(vvvvv_ope &out){
    FILE* fp = fopen(DATA_PATH, "rb");
    if (fp == NULL) {
        cout << "File Open Error" << endl;
        exit(1);
    }
    char tmp_N;
    fread(&tmp_N, sizeof(char), 1, fp);
    int T;
    fread(&T, sizeof(int), 1, fp);
    out.resize(T);
    rep (i, T){
        out[i] = vvvv_ope(tmp_N, vvv_ope(tmp_N));
        rep (j, tmp_N) rep (k, tmp_N){
            char M;
            fread(&M, sizeof(char), 1, fp);
            out[i][j][k].resize(M);
            //if (M != 0 && M != 5) cout << (int)M << endl;
            rep (l, M){
                char K;
                fread(&K, sizeof(char), 1, fp);
                out[i][j][k][l].resize(K);
                rep (m, K){
                    char x, y, n;
                    fread(&x, sizeof(char), 1, fp);
                    fread(&y, sizeof(char), 1, fp);
                    fread(&n, sizeof(char), 1, fp);
                    out[i][j][k][l][m] = (Ope){(int)x, (int)y, (int)n};
                }
            }
        }
    }
    fclose(fp);
}

void set_pos(Pos *base_ent_pos, State &s, int *val, Pos *another_ent_pos, Pos *diff){
    *val = s.field[base_ent_pos->y][base_ent_pos->x].val;
    *another_ent_pos = s.ent_pos[*val][(s.field[base_ent_pos->y][base_ent_pos->x].num + 1) % 2];
    *diff = *another_ent_pos - *base_ent_pos;
}

/*void rotate(State &s, Ope &ope){
    vv_ent copy_field = s.field;
    rep (i, ope.n) rep (j, ope.n){
        s.field[ope.y + i][ope.x + j] = copy_field[ope.y + ope.n - j - 1][ope.x + i];
        s.ent_pos[s.field[ope.y + i][ope.x + j].val][s.field[ope.y + i][ope.x + j].num] = (Pos){ope.x + j, ope.y + i};
    }
}*/

void rotate(State &s, Ope &ope){
    int a = ope.n >> 1;
    int b = ope.n & 1;
    Ent buf;
    int h1, w1, h2, w2;
    //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
    for(h1 = 0; h1 < a; h1++){
        h2 = ope.n - h1 - 1;
        for(w1 = 0; w1 < a; w1++){
            w2 = ope.n - w1 - 1;
            buf = s.field[ope.y + h1][ope.x + w1];
    
            int dy[4] = {h1, w2, h2, w1};
            int dx[4] = {w1, h1, w2, h2};
            rep (i, 4){
                Pos setting = (Pos){ope.x + dx[i], ope.y + dy[i]};
                s.field[setting.y][setting.x] = i == 3 ? buf : s.field[ope.y + dy[i + 1]][ope.x + dx[i + 1]];
                s.ent_pos[s.field[setting.y][setting.x].val][s.field[setting.y][setting.x].num] = setting;
            }
        }
    }
    //奇数の時の真ん中を動かす
    if(b == 1){
        int mw = ope.x + a, mh = ope.y + a;
        for(int i = 0; i < a; i++){
            w1 = ope.x + i;
            w2 = ope.x + ope.n - i - 1;
            h1 = ope.y + i;
            h2 = ope.y + ope.n - i - 1;
            buf = s.field[h1][mw];
    
            int dy[4] = {h1, mh, h2, mh};
            int dx[4] = {mw, w1, mw, w2};
            rep (j, 4){
                Pos setting = (Pos){dx[j], dy[j]};
                s.field[setting.y][setting.x] = j == 3 ? buf : s.field[dy[j + 1]][dx[j + 1]];
                s.ent_pos[s.field[setting.y][setting.x].val][s.field[setting.y][setting.x].num] = setting;
            }
        }
    }
}

void rotate_back(State &s, Ope &ope){
    int a = ope.n >> 1;
    int b = ope.n & 1;
    Ent buf;
    int h1, w1, h2, w2;
    //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
    for(h1 = 0; h1 < a; h1++){
        h2 = ope.n - h1 - 1;
        for(w1 = 0; w1 < a; w1++){
            w2 = ope.n - w1 - 1;
            buf = s.field[ope.y + h1][ope.x + w1];
    
            // int dy[4] = {h1, w2, h2, w1};
            // int dx[4] = {w1, h1, w2, h2};
            int dy[4] = {h1, w1, h2, w2};
            int dx[4] = {w1, h2, w2, h1};
            rep (i, 4){
                Pos setting = (Pos){ope.x + dx[i], ope.y + dy[i]};
                s.field[setting.y][setting.x] = i == 3 ? buf : s.field[ope.y + dy[i + 1]][ope.x + dx[i + 1]];
                s.ent_pos[s.field[setting.y][setting.x].val][s.field[setting.y][setting.x].num] = setting;
            }
        }
    }
    //奇数の時の真ん中を動かす
    if(b == 1){
        int mw = ope.x + a, mh = ope.y + a;
        for(int i = 0; i < a; i++){
            w1 = ope.x + i;
            w2 = ope.x + ope.n - i - 1;
            h1 = ope.y + i;
            h2 = ope.y + ope.n - i - 1;
            buf = s.field[h1][mw];
    
            // int dy[4] = {h1, mh, h2, mh};
            // int dx[4] = {mw, w1, mw, w2};
            int dy[4] = {h1, mh, h2, mh};
            int dx[4] = {mw, w2, mw, w1};
            rep (j, 4){
                Pos setting = (Pos){dx[j], dy[j]};
                s.field[setting.y][setting.x] = j == 3 ? buf : s.field[dy[j + 1]][dx[j + 1]];
                s.ent_pos[s.field[setting.y][setting.x].val][s.field[setting.y][setting.x].num] = setting;
            }
        }
    }
}

void print_ans(v_ope &ans, char* file_name){
    if (file_name == NULL){
        cout << ans.size() << endl;
        for (Ope ope : ans){
            cout << ope.x << " " << ope.y << " " << ope.n << endl;
        }
    }else{
        FILE* fp = fopen(file_name, "w");
        if (fp == NULL){
            cout << "File Open Error." << endl;
            exit(1);
        }
        fprintf(fp, "%d\n", (int)ans.size());
        for (Ope ope : ans){
            fprintf(fp, "%d %d %d\n", ope.x, ope.y, ope.n);
        }
        fclose(fp);
    }
}

void print_field(State &s){
    rep (i, N) rep (j, N) cout << s.field[i][j].val << " \n"[j == N - 1];
    // rep (i, (int)s.ent_pos.size()) printf("%d : (%d, %d), (%d, %d)\n", i, s.ent_pos[i][0].x, s.ent_pos[i][0].y, s.ent_pos[i][1].x, s.ent_pos[i][1].y);
}

int weighted_random(v_int &random_weight){
    int r = rand() % *random_weight.rbegin();
    return upper_bound(random_weight.begin(), random_weight.end(), r) - random_weight.begin();
}

int choice_bfs_result(vv_ope &bfs_result){
    int max_idx = 0;
    while (max_idx + 1 < bfs_result.size() && bfs_result[max_idx + 1].size() == bfs_result[0].size()) max_idx++;
    return rand() % (max_idx + 1);
}

BeamNode* createNewBeamNode(State &s){
    BeamNode *res = new BeamNode();
    res->s = s;
    res->idx_list.clear();
    res->ope_cnt = 0;
    return res;
}

BeamNode* getBeamNodeCopy(BeamNode *origin){
    BeamNode *res = new BeamNode();
    *res = *origin;
    return res;
}

void addPriorityQueue(priority_queue<BeamNode2> &p_queue, BeamNode2 data, int max_size){
    if (p_queue.size() < max_size){
        p_queue.push(data);
    } else {
        BeamNode2 tmp = p_queue.top();
        if (data < tmp){
            p_queue.pop();
            p_queue.push(data);
            delete tmp.p;
        } else {
            delete data.p;
        }
    }
}