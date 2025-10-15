#include "../Inc/all.hpp"

/**
 * 上からi番目, 左からj番目のマスをマス(i, j)とする。
 * 24x24の盤面においてi番目に揃えたい場所は一意に定まるため各操作において各マスを揃えたい場所に動かすのに適したルートをいくつか列挙する。
 */

int main(int argc, char** argv){
    char* output_file_name = NULL;

    // コマンドライン引数の処理
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-o") == 0){
            i++;
            output_file_name = argv[i];
        } else{
            printf("The argument \"%s\" is not available.\n", argv[i]);
        }
    }

    vvv_ope grid_ope;  // あるマスが含まれる操作リスト(grid_ope[i][j]:マス(i, j)を移動させる操作のリストが入る)
    vvv_bool is_can_ope(N, vv_bool(N, v_bool(N, false)));  // 各操作を可能かどうか(grid_ope[x][y][n]:操作(x, y, n)で既にそろったところを回転させない場合はtrue)
    
    // grid_opeとis_can_opeをセット
    preprocess(grid_ope, is_can_ope);

    vvvvv_ope result;  // 結果の配列(result[i][j][k][m]:i番目の操作でマス(j, k)を目的地に動かすルートのうちm番目に短いルートの配列)
    
    // result配列を作成
    solve1(grid_ope, is_can_ope, result);

    /*rep (i, (int)result.size()){
        cout << i << endl;
        rep (j, N) rep (k, N){
            if (result[i][j][k].size() == 0) continue;
            printf("(%d, %d)\n", k, j);
            rep (l, (int)result[i][j][k].size()){
                for (Ope ope : result[i][j][k][l]){
                    cout << ope.x << " " << ope.y << " " << ope.n << endl;
                }
                cout << endl;
            }
            cout << endl;
        }
        cout << endl << endl;
    }*/

    // 保存
    if (output_file_name != NULL){
        save_result(result, output_file_name);
    }
    return 0;
}