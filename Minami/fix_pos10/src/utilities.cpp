#include "../inc/all.hpp"

State loadProblem(char *file_name) {
    State res;
    int N;
    if (file_name == NULL) {
        cin >> N;
        res.f.size = N;
        res.f.ent_mem = new Ent[N * N];
        res.f.pos_mem = new Pos[N * N];
        int cnt[N * N / 2];
        rep (i, N * N / 2) cnt[i] = 0;
        rep (i, N) rep (j, N){
            int val;
            cin >> val;
            res.f.getEnt(i, j).val = val;
            res.f.getEnt(i, j).num = cnt[val];
            res.f.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};
            cnt[val]++;
        }
    } else {
        FILE* fp = fopen(file_name, "r");
        if (fp == NULL){
            cout << "File Open Error" << endl;
            exit(1);
        }
        fscanf(fp, "%d", &N);
        res.f.size = N;
        res.f.ent_mem = new Ent[N * N];
        res.f.pos_mem = new Pos[N * N];

        int cnt[N * N / 2];
        rep (i, N * N / 2) cnt[i] = 0;
        rep (i, N) rep (j, N){
            int val;
            fscanf(fp, "%d", &val);
            res.f.getEnt(i, j).val = val;
            res.f.getEnt(i, j).num = cnt[val];
            res.f.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};
            cnt[val]++;
        }
        fclose(fp);
    }
    return res;
}

void print_ans(v_ope &ans, char* file_name){
    if (file_name == NULL){
        cout << ans.size() << endl;
        for (Ope ope : ans){
            cout << ope.x << " " << ope.y << " " << ope.n << endl;
        }
    } else {
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

int manhattan(Pos p1, Pos p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

Ope rotateOpe(Ope ope, int fsize, int r) {
    if (r == 1){
        int px = ope.x;
        int py = ope.y;
        ope.x = py;
        ope.y = fsize - px - ope.n;
    } else if (r == 2) {
        ope.x = fsize - ope.x - ope.n;
        ope.y = fsize - ope.y - ope.n;
    } else if (r == 3) {
        int px = ope.x;
        int py = ope.y;
        ope.x = fsize - py - ope.n;
        ope.y = px;
    }
    return ope;
}

Pos getRotatePos(Pos p, Ope ope) {
    if (!(p.x >= ope.x && p.x < ope.x + ope.n && p.y >= ope.y && p.y < ope.y + ope.n)) return p;
    Pos tmp = Pos(p.x - ope.x, p.y - ope.y);
    Pos tmp_cp = tmp;
    tmp.x = ope.n - tmp_cp.y - 1;
    tmp.y = tmp_cp.x;
    p.x = ope.x + tmp.x;
    p.y = ope.y + tmp.y;
    return p;
}

void shuffle(v_ope &ope_list){
    for (int i = (int)ope_list.size() - 1; i > 0; i--){
        int idx = rand() % (i + 1);
        Ope tmp = ope_list[i];
        ope_list[i] = ope_list[idx];
        ope_list[idx] = tmp;
    }
}