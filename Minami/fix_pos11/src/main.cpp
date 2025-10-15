#include "../inc/all.hpp"

int debug_val1 = 0;

int main(int argc, char **argv) {
    srand(0);
    int start_clock = clock();
    char* input_file_name = NULL;
    char* output_file_name = NULL;
    int max_time = 300;
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

    char *file_path_list[5];
    FILE *fp = fopen("data_path.txt", "r");
    rep (i, 5) {
        file_path_list[i] = new char[1024];
        fgets(file_path_list[i], 1024, fp);
        rep (j, strlen(file_path_list[i])) {
            if (file_path_list[i][j] == ',') {
                file_path_list[i][j] = '\0';
                break;
            }
        }
    }
    fclose(fp);
    BFS_result::loadData(24, file_path_list);
    fdb::field4_init(file_path_list[3]);
    fsdb::fsdb_init(file_path_list[4], 24);
    rep (i, 4) delete[] file_path_list[i];

    State first = loadProblem(input_file_name);
    if (first.f.size == 4) {
        v_ope ans = fdb::getField4(first.f);
        print_ans(ans, output_file_name);
        return 0;
    }

    State *first_state_mem = new State[MAX_STATE2_COUNT];
    State2 *second_state_mem = new State2[MAX_STATE2_COUNT];
    Ent *second_ent_mem = new Ent[MAX_STATE2_COUNT * 10 * 10];
    Pos *second_pos_mem = new Pos[MAX_STATE2_COUNT * 10 * 10];
    rep (i, MAX_STATE2_COUNT) {
        first_state_mem[i].score = INT_MIN;
        second_state_mem[i].score = INT_MIN;
        second_state_mem[i].f.ent_mem = second_ent_mem + i * 10 * 10;
        second_state_mem[i].f.pos_mem = second_pos_mem + i * 10 * 10;
        second_state_mem[i].end_flag = false;
        second_state_mem[i].log.clear();
    }

    BeamSearch bs = BeamSearch(&first, 100, 500, first_state_mem, second_state_mem);
    v_pair_ii ans_log = bs.beamsearch();
    v_ope ans;
    first.getAnswer(ans_log, 0, ans);
    print_ans(ans, "0th_ans.txt");
    print_ans(ans, "best_ans.txt");

    int best_turn = ans.size();
    int idx_list[MAX_STATE2_COUNT];
    rep (i, MAX_STATE2_COUNT) idx_list[i] = i;
    sort(idx_list, idx_list + MAX_STATE2_COUNT, [&](int x, int y) { return first_state_mem[x].getScore() > first_state_mem[y].getScore(); } );
    rep (i, MAX_STATE2_COUNT) {
        if ((double)(clock() - start_clock) / CLOCKS_PER_SEC > max_time) break;
        int idx = idx_list[i / 2];
        cout << idx << " " << first_state_mem[idx].getScore() << endl;
        if (first_state_mem[idx].getScore() == INT_MIN) break;

        cout << i << endl;
        v_ope first_ope;
        first.getAnswer(first_state_mem[idx].log, 0, first_ope);

        BeamSearch2 bs = BeamSearch2(&second_state_mem[idx], 10000, best_turn - first_ope.size(), getScore2);
        v_ope second_ope;
        bool is_end = bs.beamsearch(second_ope);
        if (is_end) {
            int now_score = first_ope.size() + second_ope.size();
            if (now_score < best_turn) {
                best_turn = now_score;
                for (Ope &ope : second_ope) {
                    Ope tmp = rotateOpe(ope, first_state_mem[idx].f.size, first_state_mem[idx].rotate_hosei);
                    tmp.x += first_state_mem[idx].x_hosei;
                    tmp.y += first_state_mem[idx].y_hosei;
                    first_ope.push_back(tmp);
                }
                char file_name[256];
                sprintf(file_name, "%dth_ans.txt", i + 1);
                print_ans(first_ope, file_name);
                print_ans(first_ope, "best_ans.txt");
                cout << "Answer updated!!" << endl;
            }
        }
    }

    fsdb::fsdb_deinit();
    cout << "Program is successed!!" << endl;
    return 0;
}