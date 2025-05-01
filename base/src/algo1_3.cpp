#include <algo1_3.hpp>
#include <Field.hpp>
#include <vector>
#include <iostream>

/**
 * 内容 : 問題を解く関数
 * 引数 : Field& f  <- フィールド
 */
void alg1_3(Field& f){
    vvvvv_ope bfs_result;  // 幅優先探索の結果(手, もう片方のy座標, もう片方のx座標, 列挙した手数の番号, 手数)
    input_bfs_result(bfs_result);
    convert_bfs_result(f.getSize(), bfs_result);
    
    solve(f, bfs_result);
}

/**
 * 内容 : 幅優先探索の結果を読み込む関数
 * 引数 : vvvvv_ope &out <- 結果を格納する配列
 */
void input_bfs_result(vvvvv_ope &out){
    FILE* fp = fopen(DATA_PATH, "rb");
    if (fp == NULL) {
        std::cout << "File Open Error" << std::endl;
        exit(1);
    }
    char tmp_N;  // 盤面のサイズ(24であることを想定)
    fread(&tmp_N, sizeof(char), 1, fp);
    int T;  // 揃えるべき数(24 * 24 / 2 - 2であることを想定)
    fread(&T, sizeof(int), 1, fp);
    out.resize(T);
    rep (i, T){
        out[i] = vvvv_ope(tmp_N, vvv_ope(tmp_N));
        rep (j, tmp_N) rep (k, tmp_N){
            char M;  // 揃え方の候補
            fread(&M, sizeof(char), 1, fp);
            out[i][j][k].resize(M);
            //if (M != 0 && M != 5) cout << (int)M << endl;
            rep (l, M){
                char K;  // 揃える手数
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

/**
 * 内容 : 幅優先探索の結果を問題のサイズに適用する関数
 * 引数 : int N <- フィールドのサイズ
 *        vvvvv_ope &bfs_result <- 幅優先探索の結果
 */
void convert_bfs_result(int N, vvvvv_ope &bfs_result){
    if (N == 24) return;
    vvvvv_ope tmp_result = bfs_result;  // コピー
    int T = N * N / 2 - 2;  // 揃えるべき数
    int X = 0, Y = 24 - N;  // 素のデータの調整用
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

/**
 * 内容 : 問題を実際に解く関数
 * 引数 : Field &f <- フィールド
 *        vvvvv_ope &bfs_result <- 幅優先探索の結果
 */
void solve(Field &f, vvvvv_ope &bfs_result){
    int N = f.getSize();  // フィールドのサイズ
    v_solve_data solve_data;  // 揃えてく順番
    for (int i = 0; i < N / 2 - 1; i++){
        set_solve_data_recode(i * 2, N, solve_data);
        set_solve_data_colum(N - 2 - i * 2, N, solve_data);
    }

    rep (i, (int)solve_data.size()){
        SolveData &now_data = solve_data[i];
        int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;  // 基準となるエンティティの場所
        Pos another_ent_pos, diff;  // ペアのエンティティの場所と基準との差分
        another_ent_pos = (Pos){f.getPair(f.get(x, y))->p[0], f.getPair(f.get(x, y))->p[1]};
        diff = another_ent_pos - now_data.base_ent_pos;

        // 数手先まで読み最も良かったものを実行する
        auto exe_dfs = [&](){
            int best_idx = dfs(f, solve_data, i, bfs_result, 0, 5).second;
            v_ope &tmp_log = bfs_result[i][another_ent_pos.y][another_ent_pos.x][best_idx];
            for (Ope ope : tmp_log){
                f.rotate(ope.x, ope.y, ope.n);
            }
        };

        /**
         * 0: 下
         * 1: 右(下経由)
         * 2: 左
         * 3: 下(横経由)
         */
        switch (now_data.type){
        case 0:
            if (diff == (Pos){1, 0}){
                // 1つ右なら3回回転させて揃える
                rep (j, 3) f.rotate(x, y, 2);
            } else if (diff != (Pos){0, 1}){
                // 目的の場所にないなら再帰
                exe_dfs();
            }
            break;
        case 1:
            if (diff != (Pos){1, 0}){
                // 下に揃えてから回転
                if (diff != (Pos){0, 1}) exe_dfs();
                f.rotate(x, y, 2);
            }
            break;
        case 2:
            if (diff == (Pos){0, 1}){
                // 1つ下なら3回回転させて揃える
                rep (j, 3) f.rotate(x - 1, y, 2);
            } else if (diff != (Pos){-1, 0}){
                // 目的の場所にないなら再帰
                exe_dfs();
            }
            break;
        case 3:
            if (diff != (Pos){0, 1}){
                // 左に揃えてから回転
                if (diff != (Pos){-1, 0}) exe_dfs();
                f.rotate(x - 1, y, 2);
            }
            break;
        }
    }

    // 最後にそろっていない場合は揃える
    if (f.get(0, N - 1)->num == f.get(1, N - 2)->num){
        f.rotate(1, N - 2, 2);
        f.rotate(2, N - 4, 2);
        f.rotate(0, N - 4, 3);
    }
}

/**
 * 内容 : 揃える順番と揃え方を設定する関数(行)(幅優先探索で行った揃え方と合わせる)
 * 引数 : int recode <- 揃える行
 *        int N <- フィールドのサイズ
 *        v_solve_data &solve_data <- 結果を格納する配列
 */
void set_solve_data_recode(int recode, int N, v_solve_data &solve_data){
    int w = N - recode;  // 揃える幅
    int x, y;  // 基準の場所のx, y
    y = recode;
    Pos base_ent_pos;

    // 最後の2列以外は縦に揃える
    for (x = 0; x < w - 2; x++){
        base_ent_pos = (Pos){x, y};
        solve_data.push_back((SolveData){base_ent_pos, 0});
    }

    // 最後の2列
    x = w - 2;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 1});

    y++;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 1});
}

/**
 * 内容 : 揃える順番と揃え方を設定する関数(列)(幅優先探索で行った揃え方と合わせる)
 * 引数 : int recode <- 揃える行
 *        int N <- フィールドのサイズ
 *        v_solve_data &solve_data <- 結果を格納する配列
 */
void set_solve_data_colum(int colum, int N, v_solve_data &solve_data){
    int h = colum;  // 揃える高さ
    int x, y;  // 基準の場所のx, y
    x = colum + 1;
    Pos base_ent_pos;

    // 最後の2行以外は縦に揃える
    for (y = N - colum; y < N - 2; y++){
        base_ent_pos = (Pos){x, y};
        solve_data.push_back((SolveData){base_ent_pos, 2});
    }

    // 最後の2列
    y = N - 2;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 3});
    
    x--;
    base_ent_pos = (Pos){x, y};
    solve_data.push_back((SolveData){base_ent_pos, 3});
}

/**
 * 内容 : 数手先まで経路を全探索して最も良い経路を計算する関数
 * 引数 : Field &f <- フィールド
 *        v_solve_data &solve_data <- 揃える順番
 *        int cnt <- どこまで進んだか
 *        vvvvv_ope &bfs_result <- 幅優先探索の結果
 *        int depth <- 現在の深さ
 *        int max_depth <- 最大の深さ(探索をやめる深さ)
 * 戻り値 : p_ii <- 探索結果(first : 手数, second : 1番いい手)
 */
p_ii dfs(Field &f, v_solve_data &solve_data, int cnt, vvvvv_ope &bfs_result, int depth, int max_depth){
    if (cnt >= solve_data.size()) return std::make_pair(0, -1);
    SolveData &now_data = solve_data[cnt];
    int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;  // 基準となるエンティティの場所
    Pos another_ent_pos, diff;  // ペアのエンティティの場所と基準との差分
    another_ent_pos = (Pos){f.getPair(f.get(x, y))->p[0], f.getPair(f.get(x, y))->p[1]};
    diff = another_ent_pos - now_data.base_ent_pos;

    /**
     * 0: 下
     * 1: 右(下経由)
     * 2: 左
     * 3: 下(横経由)
     */
    switch (now_data.type){
    case 0:
        if (diff == (Pos){1, 0}){
            // 1つ右なら3回回転
            if (depth == max_depth) return std::make_pair(3, 0);
            Field tmp_f = f;  // コピー
            rep (j, 3) tmp_f.rotate(x, y, 2);
            return std::make_pair(3 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
        } else if (diff != (Pos){0, 1}){
            // 目的の場所にないなら幅優先探索の結果を試す
            if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
            int best_idx = -1;
            int best_result = INT_MAX;
            rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                Field tmp_f = f;  // コピー
                for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                if (tmp_result < best_result){
                    best_result = tmp_result;
                    best_idx = i;
                }
            }
            return std::make_pair(best_result, best_idx);
        } else {
            // 既にそろっているなら何もしない
            if (depth == max_depth) return std::make_pair(0, 0);
            Field tmp_f = f;  // コピー
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 1:
        if (diff != (Pos){1, 0}){
            if (diff != (Pos){0, 1}){
                // 中継地点にないなら幅優先探索の結果を試す -> 回転
                if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
                int best_idx;
                int best_result = INT_MAX;
                rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                    Field tmp_f = f;  // コピー
                    for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                    tmp_f.rotate(x, y, 2);
                    int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + 1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                    if (tmp_result < best_result){
                        best_result = tmp_result;
                        best_idx = i;
                    }
                }
                return std::make_pair(best_result, best_idx);
            } else{
                // 既に中継地点にあるなら回転して目標の場所へ
                if (depth == max_depth) return std::make_pair(1, 0);    
                Field tmp_f = f;  // コピー
                tmp_f.rotate(x, y, 2);
                return std::make_pair(1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
            }
        } else {
            // 何もしない
            if (depth == max_depth) return std::make_pair(0, 0);
            Field tmp_f = f;  // コピー
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 2:
        if (diff == (Pos){0, 1}){
            // 1つ下なら3回回転
            if (depth == max_depth) return std::make_pair(3, 0);
            Field tmp_f = f;  // コピー
            rep (j, 3) tmp_f.rotate(x - 1, y, 2);
            return std::make_pair(3 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
        } else if (diff != (Pos){-1, 0}){
            // 目的の場所にないなら幅優先探索の結果を試す
            if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
            int best_idx;
            int best_result = INT_MAX;
            rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                Field tmp_f = f;  // コピー
                for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                if (tmp_result < best_result){
                    best_result = tmp_result;
                    best_idx = i;
                }
            }
            return std::make_pair(best_result, best_idx);
        } else {
            // 既にそろっているなら何もしない
            if (depth == max_depth) return std::make_pair(0, 0);
            Field tmp_f = f;  // コピー
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 3:
        if (diff != (Pos){0, 1}){
            if (diff != (Pos){-1, 0}){
                // 中継地点にないなら幅優先探索の結果を試す -> 回転
                if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
                int best_idx;
                int best_result = INT_MAX;
                rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                    Field tmp_f = f;  // コピー
                    for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                    tmp_f.rotate(x - 1, y, 2);
                    int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + 1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                    if (tmp_result < best_result){
                        best_result = tmp_result;
                        best_idx = i;
                    }
                }
                return std::make_pair(best_result, best_idx);
            } else {
                // 既に中継地点にあるなら回転して目標の場所へ
                if (depth == max_depth) return std::make_pair(1, 0);    
                Field tmp_f = f;  // コピー
                tmp_f.rotate(x - 1, y, 2);
                return std::make_pair(1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
            }
        } else {
            // 何もしない
            if (depth == max_depth) return std::make_pair(0, 0);
            Field tmp_f = f;  // コピー
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    }        
    return std::make_pair(-1, -1);
}