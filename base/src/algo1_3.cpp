#include <algo1_3.hpp>
#include <Field.hpp>
#include <vector>
#include <iostream>

void alg1_3(Field& f){
    vvvvv_ope bfs_result;
    input_bfs_result(bfs_result);
    convert_bfs_result(f.getSize(), bfs_result);
    
    v_ope ans = solve(f, bfs_result);
    std::cout << ans.size() << std::endl;
}

void input_bfs_result(vvvvv_ope &out){
    FILE* fp = fopen(DATA_PATH, "rb");
    if (fp == NULL) {
        std::cout << "File Open Error" << std::endl;
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

void convert_bfs_result(int N, vvvvv_ope &bfs_result){
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

v_ope solve(Field &f, vvvvv_ope &bfs_result){
    int N = f.getSize();
    v_ope ope_log;
    int cnt = 0;
    v_solve_data solve_data;
    for (int i = 0; i < N / 2 - 1; i++){
        set_solve_data_recode(i * 2, N, solve_data);
        set_solve_data_colum(N - 2 - i * 2, N, solve_data);
    }

    rep (i, (int)solve_data.size()){
        SolveData &now_data = solve_data[i];
        int val;
        int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;
        Pos another_ent_pos, diff;
        another_ent_pos = (Pos){f.getPair(f.get(x, y))->p[0], f.getPair(f.get(x, y))->p[1]};
        diff = another_ent_pos - now_data.base_ent_pos;

        auto exe_dfs = [&](){
            /*rep (i, N) rep (j, N){
                std::cout << f.get(j, i)->num << " \n"[j == N - 1];
                // printf("(%d, %d)%c", f.get(j, i)->p[1], f.get(j, i)->p[0], " \n"[j == N - 1]);
            }
            std::cout << std::endl;*/
            int best_idx = dfs(f, solve_data, i, bfs_result, 0, 5).second;
            /*rep (i, N) rep (j, N){
                std::cout << f.get(j, i)->num << " \n"[j == N - 1];
                // printf("(%d, %d)%c", f.get(j, i)->p[1], f.get(j, i)->p[0], " \n"[j == N - 1]);
            }
            std::cout << std::endl;
            std::cout << best_idx << std::endl;*/
            v_ope &tmp_log = bfs_result[i][another_ent_pos.y][another_ent_pos.x][best_idx];
            for (Ope ope : tmp_log){
                ope_log.push_back(ope);
                //field = rotate(field, ent_pos, ope);
                f.rotate(ope.x, ope.y, ope.n);
            }
        };

        switch (now_data.type){
        case 0:
            if (diff == (Pos){1, 0}){
                Ope ope = (Ope){x, y, 2};
                rep (j, 3){
                    ope_log.push_back(ope);
                    // field = rotate(field, ent_pos, ope);
                    f.rotate(x, y, 2);
                }
            } else if (diff != (Pos){0, 1}){
                exe_dfs();
            }
            break;
        case 1:
            if (diff != (Pos){1, 0}){
                if (diff != (Pos){0, 1}){
                    exe_dfs();
                }
                Ope ope = (Ope){x, y, 2};
                ope_log.push_back(ope);
                // field = rotate(field, ent_pos, ope);
                f.rotate(x, y, 2);
            }
            break;
        case 2:
            if (diff == (Pos){0, 1}){
                Ope ope = (Ope){x - 1, y, 2};
                rep (j, 3){
                    ope_log.push_back(ope);
                    // field = rotate(field, ent_pos, ope);
                    f.rotate(x - 1, y, 2);
                }
            } else if (diff != (Pos){-1, 0}){
                exe_dfs();
            }
            break;
        case 3:
            if (diff != (Pos){0, 1}){
                if (diff != (Pos){-1, 0}){
                    exe_dfs();
                }
                Ope ope = (Ope){x - 1, y, 2};
                ope_log.push_back(ope);
                // field = rotate(field, ent_pos, ope);
                f.rotate(x - 1, y, 2);
            }
            break;
        }
    }

    if (f.get(0, N - 1)->num != f.get(1, N - 1)->num){
        f.rotate(1, N - 2, 2);
        f.rotate(2, N - 4, 2);
        f.rotate(0, N - 4, 3);
    }
    return ope_log;
}

void set_solve_data_recode(int recode, int N, v_solve_data &solve_data){
    int w = N - recode;
    int x, y;
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

void set_solve_data_colum(int colum, int N, v_solve_data &solve_data){
    int h = colum;
    int x, y;
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

p_ii dfs(Field &f, v_solve_data &solve_data, int cnt, vvvvv_ope &bfs_result, int depth, int max_depth){
    if (cnt >= solve_data.size()) return std::make_pair(0, -1);
    SolveData &now_data = solve_data[cnt];
    int val;
    int x = now_data.base_ent_pos.x, y = now_data.base_ent_pos.y;
    Pos another_ent_pos, diff;
    another_ent_pos = (Pos){f.getPair(f.get(x, y))->p[0], f.getPair(f.get(x, y))->p[1]};
    diff = another_ent_pos - now_data.base_ent_pos;

    switch (now_data.type){
    case 0:
        // std::cout << depth << " beginA" << std::endl;
        if (diff == (Pos){1, 0}){
            // std::cout << "beginA_1" << std::endl;
            /*printf("begin_A_1 cnt : %d, (%d, %d) & (%d, %d)\n", cnt, y, x, another_ent_pos.y, another_ent_pos.x);
            rep (i, f.getSize()) rep (j, f.getSize()){
                std::cout << f.get(j, i)->num << " \n"[j == f.getSize() - 1];
            }
            rep (i, f.getSize()) rep (j, f.getSize()){
                // std::cout << f.get(j, i)->num << " \n"[j == f.getSize() - 1];
                printf("(%d, %d)%c", f.get(j, i)->p[1], f.get(j, i)->p[0], " \n"[j == f.getSize() - 1]);
            }
            rep (i, f.getSize() * f.getSize() / 2){
                printf("%d & %d\n", f.pentities[i].p1->num, f.pentities[i].p2->num);
            }
            std::cout << std::endl;*/

            if (depth == max_depth) return std::make_pair(3, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            Ope ope = (Ope){x, y, 2};
            rep (j, 3) tmp_f.rotate(x, y, 2);
            return std::make_pair(3 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
        } else if (diff != (Pos){0, 1}){
            //std::cout << "beginA_2" << " " << another_ent_pos.y << " " << another_ent_pos.x << std::endl;
            //printf("begin_A_2 cnt : %d, (%d, %d) & (%d, %d)\n", cnt, y, x, another_ent_pos.y, another_ent_pos.x);
            /*rep (i, f.getSize()) rep (j, f.getSize()){
                std::cout << f.get(j, i)->num << " \n"[j == f.getSize() - 1];
            }
            rep (i, f.getSize()) rep (j, f.getSize()){
                // std::cout << f.get(j, i)->num << " \n"[j == f.getSize() - 1];
                printf("(%d, %d)%c", f.get(j, i)->p[1], f.get(j, i)->p[0], " \n"[j == f.getSize() - 1]);
            }
            printf("%p\n", f.pentities);
            std::cout << std::flush;
            rep (i, f.getSize() * f.getSize() / 2){
                printf("%d & %d\n", f.pentities[i].p1->num, f.pentities[i].p2->num);
            }
            std::cout << std::endl;*/
            if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
            int best_idx = -1;
            int best_result = INT_MAX;
            rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                //std::cout << i << std::endl;
                // Field tmp_f = f;
                Field tmp_f = Field(f);
                for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                if (tmp_result < best_result){
                    best_result = tmp_result;
                    best_idx = i;
                }
            }
            // std::cout << best_idx << std::endl;
            return std::make_pair(best_result, best_idx);
        } else {
            // std::cout << "beginA_3" << std::endl;
            if (depth == max_depth) return std::make_pair(0, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 1:
        // std::cout << depth << " beginB" << std::endl;
        if (diff != (Pos){1, 0}){
            if (diff != (Pos){0, 1}){
                if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
                int best_idx;
                int best_result = INT_MAX;
                rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                    // Field tmp_f = f;
                    Field tmp_f = Field(f);
                    for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                    Ope ope = (Ope){x, y, 2};
                    tmp_f.rotate(x, y, 2);
                    int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + 1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                    if (tmp_result < best_result){
                        best_result = tmp_result;
                        best_idx = i;
                    }
                }
                return std::make_pair(best_result, best_idx);
            } else{
                if (depth == max_depth) return std::make_pair(1, 0);    
                // Field tmp_f = f;
                Field tmp_f = Field(f);
                Ope ope = (Ope){x, y, 2};
                tmp_f.rotate(x, y, 2);
                return std::make_pair(1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
            }
        } else {
            if (depth == max_depth) return std::make_pair(0, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 2:
        // std::cout << depth << " beginC" << std::endl;
        if (diff == (Pos){0, 1}){
            if (depth == max_depth) return std::make_pair(3, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            Ope ope = (Ope){x - 1, y, 2};
            rep (j, 3) tmp_f.rotate(x - 1, y, 2);
            return std::make_pair(3 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
        } else if (diff != (Pos){-1, 0}){
            if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
            int best_idx;
            int best_result = INT_MAX;
            rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                // Field tmp_f = f;
                Field tmp_f = Field(f);
                for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                if (tmp_result < best_result){
                    best_result = tmp_result;
                    best_idx = i;
                }
            }
            return std::make_pair(best_result, best_idx);
        } else {
            if (depth == max_depth) return std::make_pair(0, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    case 3:
        // std::cout << depth << " beginD" << std::endl;
        if (diff != (Pos){0, 1}){
            if (diff != (Pos){-1, 0}){
                if (depth == max_depth) return std::make_pair(bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][0].size(), 0);
                int best_idx;
                int best_result = INT_MAX;
                rep (i, bfs_result[cnt][another_ent_pos.y][another_ent_pos.x].size()){
                    // Field tmp_f = f;
                    Field tmp_f = Field(f);
                    for (Ope ope : bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i]) tmp_f.rotate(ope.x, ope.y, ope.n);
                    Ope ope = (Ope){x - 1, y, 2};
                    tmp_f.rotate(x - 1, y, 2);
                    int tmp_result = bfs_result[cnt][another_ent_pos.y][another_ent_pos.x][i].size() + 1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first;
                    if (tmp_result < best_result){
                        best_result = tmp_result;
                        best_idx = i;
                    }
                }
                return std::make_pair(best_result, best_idx);
            } else {
                if (depth == max_depth) return std::make_pair(1, 0);    
                // Field tmp_f = f;
                Field tmp_f = Field(f);
                Ope ope = (Ope){x - 1, y, 2};
                tmp_f.rotate(x - 1, y, 2);
                return std::make_pair(1 + dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth).first, 0);
            }
        } else {
            if (depth == max_depth) return std::make_pair(0, 0);
            // Field tmp_f = f;
            Field tmp_f = Field(f);
            return dfs(tmp_f, solve_data, cnt + 1, bfs_result, depth + 1, max_depth);
        }
        break;
    }        
    return std::make_pair(-1, -1);
}