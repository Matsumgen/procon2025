#include "../inc/all.hpp"

void input_data(State &s, char* file_name){
    if (file_name == NULL){
        cin >> N;
        s.size = N;
        s.x_hosei = s.y_hosei = 0;
        s.field = new Ent[N * N];
        s.ent_pos = new Pos[N * N];
        int cnt[N * N / 2];
        rep (i, N * N / 2) cnt[i] = 0;
        rep (i, N) rep (j, N){
            int val;
            cin >> val;
            s.getEnt(i, j).val = val;
            s.getEnt(i, j).num = cnt[val];
            s.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};
            cnt[val]++;
        }
    }else{
        input_file(s, file_name);
    }
}

void input_file(State &s, char* file_name){
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL){
        cout << "File Open Error" << endl;
        exit(1);
    }
    fscanf(fp, "%d", &N);
    s.size = N;
    s.x_hosei = s.y_hosei = 0;
    s.field = new Ent[N * N];
    s.ent_pos = new Pos[N * N];

    int cnt[N * N / 2];
    rep (i, N * N / 2) cnt[i] = 0;
    rep (i, N) rep (j, N){
        int val;
        fscanf(fp, "%d", &val);
        s.getEnt(i, j).val = val;
        s.getEnt(i, j).num = cnt[val];
        s.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};
        cnt[val]++;
    }
    fclose(fp);
}

void set_pos(Pos *base_ent_pos, vv_ent &field, vv_pos &ent_pos, int *val, Pos *another_ent_pos, Pos *diff){
    *val = field[base_ent_pos->y][base_ent_pos->x].val;
    *another_ent_pos = ent_pos[*val][(field[base_ent_pos->y][base_ent_pos->x].num + 1) % 2];
    *diff = *another_ent_pos - *base_ent_pos;
}

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
            buf = s.getEnt(ope.y + h1, ope.x + w1);
    
            int dy[4] = {h1, w2, h2, w1};
            int dx[4] = {w1, h1, w2, h2};
            rep (i, 4){
                Pos setting = (Pos){static_cast<uint8_t>(ope.x + dx[i]), static_cast<uint8_t>(ope.y + dy[i])};
                s.getEnt(setting.y, setting.x) = i == 3 ? buf : s.getEnt(ope.y + dy[i + 1], ope.x + dx[i + 1]);
                s.getEntPos(s.getEnt(setting.y, setting.x).val, s.getEnt(setting.y, setting.x).num) = setting;
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
            buf = s.getEnt(h1, mw);
    
            int dy[4] = {h1, mh, h2, mh};
            int dx[4] = {mw, w1, mw, w2};
            rep (j, 4){
                Pos setting = (Pos){static_cast<uint8_t>(dx[j]), static_cast<uint8_t>(dy[j])};
                s.getEnt(setting.y, setting.x) = j == 3 ? buf : s.getEnt(dy[j + 1], dx[j + 1]);
                s.getEntPos(s.getEnt(setting.y, setting.x).val, s.getEnt(setting.y, setting.x).num) = setting;
            }
        }
    }
}

void exe_ope(State &s, v_ope &ope_list, v_bool &is_exe) {
    rep (i, (int)ope_list.size()) {
        if (is_exe[i]) rotate(s, ope_list[i]);
    }
}

void stateToChar(State &s, uint8_t *out) {
    uint8_t convert[N * N / 2];
    rep (i, N * N / 2) convert[i] = 255;
    uint8_t next_group = 0;
    rep (i, N) rep (j, N) {
        int val = s.getEnt(i, j).val;
        if (convert[val] == 255) {
            convert[val] = next_group;
            next_group++;
        }
        out[i * N + j] = convert[val];
    }
}

int getPairCnt(vv_pos &ent_pos){
    int res = 0;
    rep (i, N * N / 2){
        res += manhattan(ent_pos[i][0], ent_pos[i][1]) == 1;
    }
    return res;
}

bool isEnd(State &s) {
    rep (i, s.size * s.size / 2) {
        if (manhattan(s.getEntPos(i, 0), s.getEntPos(i, 1)) != 1) return false;
    }
    return true;
}

int manhattan(Pos &p1, Pos &p2){
    return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

void shuffle(v_ope &ope_list){
    for (int i = (int)ope_list.size() - 1; i > 0; i--){
        int idx = rand() % (i + 1);
        Ope tmp = ope_list[i];
        ope_list[i] = ope_list[idx];
        ope_list[idx] = tmp;
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

int weighted_random(v_int &random_weight){
    int r = rand() % *random_weight.rbegin();
    return upper_bound(random_weight.begin(), random_weight.end(), r) - random_weight.begin();
}

int choice_bfs_result(vv_ope &bfs_result){
    int max_idx = 0;
    while (max_idx + 1 < bfs_result.size() && bfs_result[max_idx + 1].size() == bfs_result[0].size()) max_idx++;
    return rand() % (max_idx + 1);
}

vv_int setOK2x2(State &s) {
    vv_int res(N / 2, v_int(N / 2));
    for (int i = 0; i < N; i += 2) for (int j = 0; j < N; j += 2) {
        bool a, b, c, d;
        a = s.getEnt(i, j).val == s.getEnt(i, j + 1).val;
        b = s.getEnt(i + 1, j).val == s.getEnt(i + 1, j + 1).val;
        c = s.getEnt(i, j).val == s.getEnt(i + 1, j).val;
        d = s.getEnt(i, j + 1).val == s.getEnt(i + 1, j + 1).val;
        res[i / 2][j / 2] = a && b || c && d;
    }
    return res;
}

void setSum(vv_int &array) {
    for (int i = 0; i < array.size(); i++) {
        for (int j = 1; j < array[i].size(); j++) {
            array[i][j] += array[i][j - 1];
        }
    }
    for (int i = 1; i < array.size(); i++) {
        for (int j = 0; j < array[i].size(); j++) {
            array[i][j] += array[i - 1][j];
        }
    }
}

int getSum(vv_int &sum_array, int x1, int y1, int x2, int y2) {
    int tmp1 = (y1 == 0 || x1 == 0) ? 0 : sum_array[y1 - 1][x1 - 1];
    int tmp2 = (y1 == 0) ? 0 : sum_array[y1 - 1][x2];
    int tmp3 = (x1 == 0) ? 0 : sum_array[y2][x1 - 1];
    return sum_array[y2][x2] - tmp2 - tmp3 + tmp1;
}

BeamNode* createNewBeamNode(State &s){
    BeamNode *res = new BeamNode();
    s.getClone(res->s);
    res->score = 0;
    res->ope_list.clear();
    return res;
}

BeamNode* getBeamNodeCopy(BeamNode *origin){
    BeamNode *res = new BeamNode();
    origin->s.getClone(res->s);
    res->score = origin->score;
    res->ope_list = origin->ope_list;
    return res;
}

bool addPriorityQueue(priority_queue<BeamNode2> &p_queue, BeamNode2 data, int max_size){
    if (p_queue.size() < max_size){
        p_queue.push(data);
        return true;
    } else {
        BeamNode2 tmp = p_queue.top();
        if (data < tmp){
            p_queue.pop();
            p_queue.push(data);
            return true;
        } else {
            return false;
        }
    }
}

uint8_t popcount(unsigned long long int x){
    unsigned char cnt=0;
    while (x!=0){
        cnt+=(x&1);
        x=x>>1;
    }
    return cnt;
}

int getMaxBit(unsigned long long int x){
	int cnt=0;
	while (((unsigned long long int)1<<(cnt+1))<=x){
		cnt++;
	}
	return cnt;
}

int getMinBit(unsigned long long int x){
	int cnt=0;
	while (getBit(x, cnt)==0){
		cnt++;
	}
	return cnt;
}
