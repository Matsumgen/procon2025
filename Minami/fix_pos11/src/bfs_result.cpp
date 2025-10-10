#include "../inc/all.hpp"

vvvvv_ope BFS_result::bfs_result;
uint16_t** BFS_result::bfs_result2;
uint16_t** BFS_result::bfs_result3;
vvvvv_int BFS_result::ope_cnt;
v_ope BFS_result::all_ope;
v_int BFS_result::idx_memo(11);

void BFS_result::loadData(int max_field_size, char **file_path_list) {
    char base_file_name1[1024], base_file_name2[1024];
    strcpy(base_file_name1, file_path_list[1]);
    strcat(base_file_name1, "/bfs_result8_n%d_m5.bin");
    
    strcpy(base_file_name2, file_path_list[2]);
    strcat(base_file_name2, "/bfs_result9_n%d_m5.bin");

    FILE *fp;

    printf("Loading \"%s\".\n", file_path_list[0]);
    fp = fopen(file_path_list[0], "rb");
    if (fp == NULL) {
        cout << "File Open Error\nCannot ope bfs_result file." << endl;
        exit(1);
    }
    char tmp_N;
    fread(&tmp_N, sizeof(char), 1, fp);
    int T;
    fread(&T, sizeof(int), 1, fp);
    BFS_result::bfs_result.resize(T);
    rep (i, T){
        BFS_result::bfs_result[i] = vvvv_ope(3, vvv_ope(tmp_N * tmp_N));
        rep (j, 3) rep (k, tmp_N * tmp_N){
            char M;
            fread(&M, sizeof(char), 1, fp);
            BFS_result::bfs_result[i][j][k].resize(M);
            rep (l, M){
                char K;
                fread(&K, sizeof(char), 1, fp);
                BFS_result::bfs_result[i][j][k][l].resize(K);
                rep (m, K){
                    char x, y, n;
                    fread(&x, sizeof(char), 1, fp);
                    fread(&y, sizeof(char), 1, fp);
                    fread(&n, sizeof(char), 1, fp);
                    BFS_result::bfs_result[i][j][k][l][m] = (Ope){(short)x, (short)y, (short)n};
                }
            }
        }
    }
    fclose(fp);

    int M = 5;
    BFS_result::bfs_result2 = new uint16_t*[11];
    // for (int size = 24; size >= 4; size -= 2) {
    // for (int size = 4; size <= max_field_size; size += 2) {
    for (int size = 4; size <= min(22, max_field_size); size += 2) {
        char file_name[1024];
        sprintf(file_name, base_file_name1, size);
        printf("Loading \"%s\".\n", file_name);
        fp = fopen(file_name, "rb");
        if (fp == NULL) {
            cout << "File Open Error\nCannot ope bfs_result file." << endl;
            exit(1);
        }

        fread(&tmp_N, sizeof(char), 1, fp);
        fread(&T, sizeof(int), 1, fp);
        int64_t len = (int64_t)T * TYPE_CNT2 * size * size * size * size * M;
        cout << len << endl;
        BFS_result::bfs_result2[size / 2 - 1] = new uint16_t[len];
        cout << fread(&bfs_result2[size / 2 - 1][0], sizeof(uint16_t), len, fp) << endl;
        fclose(fp);
    }

    BFS_result::bfs_result3 = new uint16_t*[12];
    for (int size = 4; size <= max_field_size; size += 2) {
        char file_name[1024];
        sprintf(file_name, base_file_name2, size);
        printf("Loading \"%s\".\n", file_name);
        
        fp = fopen(file_name, "rb");
        if (fp == NULL) {
            cout << "File Open Error\nCannot ope bfs_result file." << endl;
            exit(1);
        }

        fread(&tmp_N, sizeof(char), 1, fp);
        fread(&T, sizeof(int), 1, fp);
        int64_t len = (int64_t)T * TYPE_CNT3 * (size - 1) * size * size * M;
        cout << len << endl;
        BFS_result::bfs_result3[size / 2 - 1] = new uint16_t[len];
        // cout << BFS_result::bfs_result3[size / 2 - 1] << endl;
        cout << fread(&bfs_result3[size / 2 - 1][0], sizeof(uint16_t), len, fp) << endl;
        fclose(fp);
    }

    BFS_result::ope_cnt.resize(max_field_size / 2);
    for (int size = 2; size <= max_field_size; size += 2) {
        BFS_result::ope_cnt[size / 2 - 1] = vvvv_int(size * 2 - 2, vvv_int(TYPE_CNT3, vv_int(size - 1, v_int(size * size, 0))));
        rep (i, size * 2 - 2) rep (j, TYPE_CNT3) rep (k, size - 1) rep (l, size * size) {
            // cout << i << " " << j << " " << k << " " << l << endl;
            rep (m, M) {
                if (BFS_result::getParent2(size, i, j, k, l, m) == 0xFFFE) break;
                BFS_result::ope_cnt[size / 2 - 1][i][j][k][l]++;
            }
        }
    }
    // exit(0);

    // int idx = 0;
    BFS_result::all_ope.reserve(23 * (23 + 1) * (2 * 23 + 1) / 6 - 1);
    for (int n = 2; n <= 24; n++) {
        for (int x = 0; x <= 24 - n; x++) {
            for (int y = 0; y <= 24 - n; y++) {
                BFS_result::all_ope.push_back(Ope(x, y, n));
            }
        }
    }
    sort(BFS_result::all_ope.begin(), BFS_result::all_ope.end());

    int tmp = 0;
    int size = 24;
    rep (i, 11) {
        BFS_result::idx_memo[i] = tmp;
        tmp += size * 2 - 2;
        size -= 2;
    }

    for (int i = 1; i < 11; i++) {
        int tmp_size = (12 - i) * 2;
        rep (j, tmp_size * 2 - 2) {
            int idx = idx_memo[i] + j;
            rep (k, 3) rep (l, 24 * 24) {
                rep (m, (int)BFS_result::bfs_result[idx][k][l].size()) {
                    for (Ope &ope :BFS_result::bfs_result[idx][k][l][m]) {
                        ope.y -= i * 2;
                    }
                }
            }
        }
    }
}

// void BFS_result::close() {
//     fclose(BFS_result::fp);
//     strcpy(BFS_result::now_file_name, "");
// }

v_ope BFS_result::getOperation(State *s, v_pos &target_pos, int type, int idx) {
    if (type < TYPE_CNT1) {
        target_pos[0] = BFS_result::calcTargetPos(s->f.size, target_pos[0]);
        return BFS_result::bfs_result[BFS_result::calcOpeIndex(s->f.size, s->progress)][type][target_pos[0].y * 24 + target_pos[0].x][idx];
    } else if (type < TYPE_CNT1 + TYPE_CNT2) {
        type -= TYPE_CNT1;
        v_ope res;
        uint16_t parent;
        v_pos now_target = target_pos;
        int now_idx = idx;
        while ((parent = BFS_result::getParent(s->f.size, s->progress - 1, type, now_target[0].toInt(s->f.size), now_target[1].toInt(s->f.size), now_idx)) != 65535) {
            Ope tmp_ope = BFS_result::all_ope[parent & 0x1FFF];
            res.push_back(tmp_ope);
            rep (i, 2) {
                now_target[i] = getRotatePos(now_target[i], tmp_ope);
            }
            now_idx = parent >> 13;
        }
        return res;
    } else {
        type -= TYPE_CNT1 + TYPE_CNT2;
        v_ope res;
        uint16_t parent;
        Pos now_target = target_pos[0];
        int now_idx = idx;
        while ((parent = BFS_result::getParent2(s->f.size, s->progress - 1, type, s->last_pair_x - 1, now_target.toInt(s->f.size), now_idx)) != 65535) {
            Ope tmp_ope = BFS_result::all_ope[parent & 0x1FFF];
            res.push_back(tmp_ope);
            now_target = getRotatePos(now_target, tmp_ope);
            now_idx = parent >> 13;
        }
        return res;
    }
}

int BFS_result::getOperationCount(State *s, v_pos &target_pos, int type) {
    if (type < TYPE_CNT1) {
        target_pos[0] = BFS_result::calcTargetPos(s->f.size, target_pos[0]);
        return BFS_result::bfs_result[BFS_result::calcOpeIndex(s->f.size, s->progress)][type][target_pos[0].y * 24 + target_pos[0].x].size();
    } else if (type < TYPE_CNT1 + TYPE_CNT2) {
        return 5;
    } else {
        type -= TYPE_CNT1 + TYPE_CNT2;
        return BFS_result::ope_cnt[s->f.size / 2 - 1][s->progress - 1][type][s->last_pair_x - 1][target_pos[0].toInt(s->f.size)];
    }
}

int BFS_result::calcOpeIndex(int size, int progress) {
    if (size & 1 || size > 24 || size <= 2) {
        cout << "BFS_result:::calcOpeIndex:\nLogic error\nsize is invaid" << endl;
        exit(1);
    }
    return BFS_result::idx_memo[12 - size / 2] + progress - 1;
}

Pos BFS_result::calcTargetPos(int size, Pos target_pos) {
    target_pos.y += 24 - size;
    return target_pos;
}

uint16_t BFS_result::getParent(int size, int t, int type, int target1, int target2, int idx) {
    int idxes[5] = {idx, target2, target1, type, t};
    int max_cnt[5] = {5, size * size, size * size, TYPE_CNT2, 1};
    int64_t result_idx = 0;
    int64_t hosei = 1;
    rep (i, 5) {
        result_idx += hosei * idxes[i];
        hosei *= max_cnt[i];
    }
    return BFS_result::bfs_result2[size / 2 - 1][result_idx];
}

uint16_t BFS_result::getParent2(int size, int t, int type, int x, int target, int idx) {
    int idxes[5] = {idx, target, x, type, t};
    int max_cnt[5] = {5, size * size, size - 1, TYPE_CNT3, 1};
    int64_t result_idx = 0;
    int64_t hosei = 1;
    rep (i, 5) {
        result_idx += hosei * idxes[i];
        hosei *= max_cnt[i];
    }
    // cout << BFS_result::bfs_result3[size / 2 - 1] << " " << &BFS_result::bfs_result3[size / 2 - 1][0] << endl;
    return BFS_result::bfs_result3[size / 2 - 1][result_idx];
}