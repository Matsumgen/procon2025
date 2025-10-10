#ifndef BFS_RESULT_HPP
#define BFS_RESULT_HPP 0
#include <bits/stdc++.h>
using namespace std;
#include "state.hpp"

class BFS_result {
    public:
        static vvvvv_ope bfs_result;
        static uint16_t **bfs_result2;
        static uint16_t **bfs_result3;
        static vvvvv_int ope_cnt;
        static v_ope all_ope;

        static void loadData(int max_field_size, char **file_path_list);
        static v_ope getOperation(State *s, v_pos &target_pos, int type, int idx);
        static int getOperationCount(State *s, v_pos &target_pos, int type);
    private:
        static v_int idx_memo;

        static int calcOpeIndex(int size, int progress);
        static Pos calcTargetPos(int size, Pos target_pos);
        static uint16_t getParent(int size, int t, int type, int target1, int target2, int idx);
        static uint16_t getParent2(int size, int t, int type, int x, int target, int idx);
};
#endif