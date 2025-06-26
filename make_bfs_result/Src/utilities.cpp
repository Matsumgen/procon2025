#include "../Inc/all.hpp"

/**
 * 基準を設定した際に, 基準に書いてある値, 同じ値が書いてあるもう1つの場所, その差を設定する関数
 * In:
 *   Pos *base_ent_pos : 基準の場所
 *   vv_ent &field : 現在のフィールド
 *   vv_pos &ent_pos : 各エンティティの場所
 * Out:
 *   int *val : 基準に書いてある値が入る
 *   Pos *another_ent_pos : 基準と同じ値が書いてある別の場所
 *   Pos *diff : 2つの場所の差
 */
void set_pos(Pos *base_ent_pos, vv_ent &field, vv_pos &ent_pos, int *val, Pos *another_ent_pos, Pos *diff){
    *val = field[base_ent_pos->y][base_ent_pos->x].val;
    *another_ent_pos = ent_pos[*val][(field[base_ent_pos->y][base_ent_pos->x].num + 1) % 2];
    *diff = *another_ent_pos - *base_ent_pos;
}

/**
 * 盤面を回転させる関数
 * 破壊的ではない
 */
vv_ent rotate(vv_ent &field, vv_pos &ent_pos, Ope ope){
    vv_ent res = field;
    rep (i, ope.n) rep (j, ope.n){
        res[ope.y + i][ope.x + j] = field[ope.y + ope.n - j - 1][ope.x + i];
        ent_pos[res[ope.y + i][ope.x + j].val][res[ope.y + i][ope.x + j].num] = (Pos){ope.x + j, ope.y + i};
    }
    return res;
}

/**
 * grid_ope, is_can_pos, resultを更新する関数
 * In:
 *   Pos base_pos : 基準
 *   Pos goal_pos : 動かすものの幅優先探索での目的地
 *   Pos last_pos : 動かしたやつの最終の位置
 *   vvv_ope &grid_ope : 各マスを動かす操作
 *   vvv_bool &is_can_ope : 各操作をできるか
 * Out(更新される):
 *   vvv_bool &is_can_ope : 不可能になる操作がある
 *   vvvvv_ope &result : bfsの結果が追加
 */
void get_best_answer(Pos base_pos, Pos goal_pos, Pos last_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result){
    // 基準を動かせなくする
    update_adj(base_pos, grid_ope, is_can_ope);

    // 幅優先探索で書くところの最善手を確定
    result.push_back(bfs(goal_pos, grid_ope, is_can_ope));

    // 動かした先を動かせなくする
    update_adj(last_pos, grid_ope, is_can_ope);
}

/**
 * 各マスを目的地に動かす経路を幅優先探索で列挙する関数
 * In:
 *   Pos goal_pos : bfsでの目的地
 *   vvv_ope &grid_ope : 各マスを動かす操作
 *   vvv_bool &is_can_ope : 各操作が可能か
 * 戻り値:
 *   vvvv_ope [i][j][k] : 現在の状態でマス(i, j)を目的地に動かす経路のうちk番目に短い経路
 */
vvvv_ope bfs(Pos goal_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope){
    int max_cnt = 5;  // 操作を列挙する数
    vvvv_ope result(N, vvv_ope(N));  // 結果(result[i][j][k] : 現在の状態でマス(i, j)を目的地に動かす経路のうちk番目に短い経路)
    // printf("B (%d, %d)\n", goal_pos.y, goal_pos.x);
    queue<QueueNode> todo;
    todo.push((QueueNode){goal_pos, v_ope(0)});  // 場所と操作リスト
    while (!todo.empty()){
        QueueNode tmp = todo.front();  // tmp.p: 場所, tmp.ope: この場所に来るまでしてきた操作のリスト
        todo.pop();

        // 既に最大数まで列挙されていたら何もしない
        if (result[tmp.pos.y][tmp.pos.x].size() == max_cnt) continue;

        // 列挙した操作に追加
        result[tmp.pos.y][tmp.pos.x].push_back(tmp.ope);

        // あるマスを動かす全ての操作を試す
        for (int i = 0; i < (int)grid_ope[tmp.pos.y][tmp.pos.x].size(); i++){
            Ope ope = grid_ope[tmp.pos.y][tmp.pos.x][i];

            // 使えない操作は消す
            if (!is_can_ope[ope.x][ope.y][ope.n]){
                grid_ope[tmp.pos.y][tmp.pos.x].erase(grid_ope[tmp.pos.y][tmp.pos.x].begin() + i);
                i--;
                continue;
            }
            v_ope tmp_ope = tmp.ope;
            tmp_ope.push_back(ope);
            Pos diff = tmp.pos - (Pos){ope.x, ope.y};
            int next_x = ope.x + diff.y, next_y = ope.y + ope.n - diff.x - 1;  // 次のx座標とy座標を計算
            todo.push((QueueNode){(Pos){next_x, next_y}, tmp_ope});
            // printf("B (%d, %d) -> (%d, %d) (%d, %d, %d)\n", tmp.pos.x, tmp.pos.y, next_x, next_y, ope.x, ope.y, ope.n);
        }
    }
    
    // 全ての操作が逆順で入っているため反転
    rep (i, N) rep (j, N) rep (k, (int)result[i][j].size()) {
        reverse(result[i][j][k].begin(), result[i][j][k].end());
    }
    return result;
}


/**
 * 対応する場所を動かす操作を全て不可能にする関数
 * Pos confirm_pos : 確定した場所(動かせなくする場所)
 * vvv_ope &grid_ope : 各マスを動かす操作リスト
 * vvv_bool &is_can_ope : 各操作が可能か
 */
void update_adj(Pos confirm_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope){
    // cout << confirm_pos.y << " " << confirm_pos.x << endl;
    // 全て不可能にする
    for (Ope &ope : grid_ope[confirm_pos.y][confirm_pos.x]){
        if (!is_can_ope[ope.x][ope.y][ope.n]) continue;
        is_can_ope[ope.x][ope.y][ope.n] = false;
    }
}

/**
 * 結果を保存する関数
 */
void save_result(vvvvv_ope &result, char* file_name){
    FILE* fp = fopen(file_name, "wb");
    char size = N;
    int T = (int)result.size();
    fwrite(&size, sizeof(char), 1, fp);
    fwrite(&T, sizeof(int), 1, fp);
    rep (i, (int)T){
        rep (j, N) rep (k, N){
            char M = (char)result[i][j][k].size();
            fwrite(&M, sizeof(char), 1, fp);
            rep (l, M){
                char K = (char)result[i][j][k][l].size();
                fwrite(&K, sizeof(char), 1, fp);
                for (Ope ope : result[i][j][k][l]){
                    char x = (char)ope.x;
                    char y = (char)ope.y;
                    char n = (char)ope.n;
                    fwrite(&x, sizeof(char), 1, fp);
                    fwrite(&y, sizeof(char), 1, fp);
                    fwrite(&n, sizeof(char), 1, fp);
                }
            }
        }
    }
    fclose(fp);
}