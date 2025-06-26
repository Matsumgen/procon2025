#include "../Inc/all.hpp"

/**
 *  揃え方 
 *  | | | | - -
 *  | | | | - -
 */

 /**
  * grid_opeとis_can_opeをセットする関数
  */
void preprocess(vvv_ope &grid_ope, vvv_bool &is_can_ope){
    grid_ope = vvv_ope(N, vv_ope(N, v_ope(0)));
    for (int size = 2; size < N; size++){
        rep (y, N - size + 1){
            rep (x, N - size + 1){
                is_can_ope[x][y][size] = true;
                rep (Y, size){
                    rep (X, size){
                        int from_x = x + X;
                        int from_y = y + Y;
                        grid_ope[from_y][from_x].push_back((Ope){x, y, size});
                    }
                }
            }
        }
    }
}

/**
 * result配列を作成する関数
 */
void solve1(vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result){
    for (int i = 0; i < N / 2 - 1; i++){
        cout << i << endl;
        solve_recode(i * 2, grid_ope, is_can_ope, result);
        solve_colum(N - 2 - i * 2, grid_ope, is_can_ope, result);
    }
}


/**
 * 行を揃えていく関数(左から右に揃える)
 */
void solve_recode(int recode, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result){
    int w = N - recode;
    int x, y;
    y = recode;
    int val;
    Pos base_ent_pos, another_ent_pos, goal_pos, diff;

    // 最後の2列以外は縦に揃える
    for (x = 0; x < w - 2; x++){
        base_ent_pos = (Pos){x, y};
        goal_pos = base_ent_pos + (Pos){0, 1};
        // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

        get_best_answer(base_ent_pos, goal_pos, goal_pos, grid_ope, is_can_ope, result);
    }

    // 最後の2列
    x = w - 2;
    base_ent_pos = (Pos){x, y};
    goal_pos = base_ent_pos + (Pos){0, 1};
    // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

    get_best_answer(base_ent_pos, goal_pos, base_ent_pos + (Pos){1, 0}, grid_ope, is_can_ope, result);

    y++;
    base_ent_pos = (Pos){x, y};
    goal_pos = base_ent_pos + (Pos){0, 1};
    // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

    get_best_answer(base_ent_pos, goal_pos, base_ent_pos + (Pos){1, 0}, grid_ope, is_can_ope, result);
}


/**
 * 列を揃えていく関数(上から下に揃える)
 */
void solve_colum(int colum, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result){
    int h = colum;
    int x, y;
    x = colum + 1;
    int val;
    Pos base_ent_pos, another_ent_pos, goal_pos, diff;

    // 最後の2行以外は縦に揃える
    for (y = N - colum; y < N - 2; y++){
        // 場所を設定
        base_ent_pos = (Pos){x, y};
        goal_pos = base_ent_pos + (Pos){-1, 0};
        // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

        get_best_answer(base_ent_pos, goal_pos, goal_pos, grid_ope, is_can_ope, result);
    }

    // 最後の2列
    // 場所を設定
    y = N - 2;
    base_ent_pos = (Pos){x, y};
    goal_pos = base_ent_pos + (Pos){-1, 0};
    // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

    get_best_answer(base_ent_pos, goal_pos, base_ent_pos + (Pos){0, 1}, grid_ope, is_can_ope, result);

    // 場所を設定
    x--;
    base_ent_pos = (Pos){x, y};
    goal_pos = base_ent_pos + (Pos){-1, 0};
    // printf("(%d, %d), (%d, %d)\n", base_ent_pos.y, base_ent_pos.x, goal_pos.y, goal_pos.x);

    get_best_answer(base_ent_pos, goal_pos, base_ent_pos + (Pos){0, 1}, grid_ope, is_can_ope, result);
}