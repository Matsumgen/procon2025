#include "../inc/bfs_result.hpp"
#include "../inc/field.hpp"
#include "../inc/utilities.hpp"

// その他の必要な標準ライブラリ
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>

using namespace std;

FILE *BFS_result::fp = NULL;
char BFS_result::now_file_name[256] = "";
vvvvv_ope BFS_result::bfs_result;
v_int BFS_result::idx_memo(11);

/**
 * ファイルを開き, 結果を読み取る関数。
 * 各変数が設定される。
 */
void BFS_result::open(char *file_name) {
    BFS_result::fp = fopen(file_name, "rb");
    strcpy(BFS_result::now_file_name, file_name);

    if (BFS_result::fp == NULL) {
        cout << "File Open Error\nCannot ope bfs_result file." << endl;
        exit(1);
    }

    // 結果の読み取り
    char tmp_N;
    fread(&tmp_N, sizeof(char), 1, BFS_result::fp);
    int T;
    fread(&T, sizeof(int), 1, BFS_result::fp);
    BFS_result::bfs_result.resize(T);
    rep (i, T){
        BFS_result::bfs_result[i] = vvvv_ope(3, vvv_ope(tmp_N * tmp_N));
        rep (j, 3) rep (k, tmp_N * tmp_N){
            char M;
            fread(&M, sizeof(char), 1, BFS_result::fp);
            BFS_result::bfs_result[i][j][k].resize(M);
            //if (M != 0 && M != 5) cout << (int)M << endl;
            rep (l, M){
                char K;
                fread(&K, sizeof(char), 1, BFS_result::fp);
                BFS_result::bfs_result[i][j][k][l].resize(K);
                rep (m, K){
                    char x, y, n;
                    fread(&x, sizeof(char), 1, BFS_result::fp);
                    fread(&y, sizeof(char), 1, BFS_result::fp);
                    fread(&n, sizeof(char), 1, BFS_result::fp);
                    BFS_result::bfs_result[i][j][k][l][m] = Ope{(short)x, (short)y, (short)n};
                }
            }
        }
    }
    BFS_result::close();

    // idx_memoの設定
    int tmp = 0;
    int size = 24;
    rep (i, 11) {
        BFS_result::idx_memo[i] = tmp;
        tmp += size * 2 - 2;
        size -= 2;
    }

    // bfs_resultファイルは24x24においての盤面で各操作が入っているため, 各ペアをそろえるときの盤面のサイズに合わせた操作になるように調整
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


/**
 * ファイルを閉じる
 */
void BFS_result::close() {
    fclose(BFS_result::fp);
    strcpy(BFS_result::now_file_name, "");
}


/**
 * ターゲットのペアを揃える手順を返す関数
 * 引数:
 *   int size : 今の盤面のサイズ
 *   int progress : どこまでそろえたか
 *   Pos target_pos : ターゲットの現在地
 *   int type : ペア揃えるタイプ
 *   int idx : 複数ある操作列のうちどれを選ぶか
 */
v_ope BFS_result::getOperation(int size, int progress, Pos target_pos, int type, int idx) {
    target_pos = BFS_result::calcTargetPos(size, target_pos);
    return BFS_result::bfs_result[BFS_result::calcOpeIndex(size, progress)][type][target_pos.y * 24 + target_pos.x][idx];
}


/**
 * ターゲットのペアを揃える手順がいくつあるか(bfs_resultファイルにあるぶん)を返す関数
 * 引数:
 *   int size : 今の盤面のサイズ
 *   int progress : どこまでそろえたか
 *   Pos target_pos : ターゲットの現在地
 *   int type : ペア揃えるタイプ
 */
int BFS_result::getOperationCount(int size, int progress, Pos target_pos, int type) {
    target_pos = BFS_result::calcTargetPos(size, target_pos);
    // cout << "B: " << target_pos.x << " " << target_pos.y << endl;
    // cout << "B: " << BFS_result::calcOpeIndex(size, progress) << endl;
    return BFS_result::bfs_result[BFS_result::calcOpeIndex(size, progress)][type][target_pos.y * 24 + target_pos.x].size();
}


/**
 * 現在の盤面のサイズとそろえた進捗から, 次揃えるペアのbfs_result配列に対応するインデックスを計算する関数
 * 引数:
 *   int size : 今の盤面のサイズ
 *   int progress : どこまでそろえたか
 */
int BFS_result::calcOpeIndex(int size, int progress) {
    if (size & 1 || size > 24 || size <= 2) {
        cout << "BFS_result:::calcOpeIndex:\nLogic error\nsize is invaid" << endl;
        exit(1);
    }
    return BFS_result::idx_memo[12 - size / 2] + progress - 1;
}


/**
 * 現在のターゲットの位置を調節する関数(bfs_resultは24x24のサイズのみを想定しているため)
 * 引数:
 *   int size : 今の盤面のサイズ
 */
Pos BFS_result::calcTargetPos(int size, Pos target_pos) {
    target_pos.y += 24 - size;
    return target_pos;
}