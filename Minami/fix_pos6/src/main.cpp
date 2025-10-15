#include "../inc/all.hpp"

int debug_val1 = 0;

int main(int argc, char **argv) {
    srand(0);
    int start_clock = clock();
    char* input_file_name = NULL;
    char* output_file_name = NULL;
    int max_time = 10;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-i") == 0){
            i++;
            input_file_name = argv[i];
        } else if (strcmp(argv[i], "-o") == 0){
            i++;
            output_file_name = argv[i];
        } else if (strcmp(argv[i], "-t") == 0){
            i++;
            max_time = atoi(argv[i]);
        } else{
            printf("The argument \"%s\" is not available.\n", argv[i]);
        }
    }

    State first = loadProblem(input_file_name);
    BFS_result::open((char*)BFS_RESULT_FILE_NAME);  // bfs_resultを設定
    BeamSearch bs = BeamSearch(&first, 2000, 500);  // ビームサーチの情報設定
    v_pair_ii ans_log = bs.beamsearch();  // ビームサーチする
    v_ope ans;
    first.getAnswer(ans_log, 0, ans);
    print_ans(ans, output_file_name);
    return 0;
}