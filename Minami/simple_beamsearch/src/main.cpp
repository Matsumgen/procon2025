#include "../inc/all.hpp"

int N;
vvv_ope grid_ope;
vvvvv_ope move_list;

int main(int argc, char** argv){
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

    vv_ent field;
    vv_pos ent_pos;
    input_data(field, ent_pos, input_file_name);
    
    State s = (State){field, ent_pos};
    v_ope ans = solve(s);
    print_ans(ans, output_file_name);

#ifdef IS_DEBUG_B
    cout << ans.size() << endl;
    rep (i, N) rep (j, N){
        cout << field[i][j].val << " \n"[j == N - 1];
    }
#endif
    return 0;
}