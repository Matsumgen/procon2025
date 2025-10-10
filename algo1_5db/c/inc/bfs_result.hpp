#ifndef BFS_RESULT_HPP
#define BFS_RESULT_HPP 0
#include <bits/stdc++.h>
using namespace std;
#include "state.hpp"

// #define BFS_RESULT_FILE_NAME "data/bfs_result6_m5.bin"
// #define BFS_RESULT_FILE_NAME "../fix_pos6_2/data/bfs_result6_m10.bin"
#define BFS_RESULT_FILE_NAME_1 "../../make_bfs_result6/bfs_result6_m20.bin"  // いい感じに設定してください

class BFS_result {
    public:
        static vvvvv_ope bfs_result;
        static uint16_t **bfs_result2;
        static v_ope all_ope;

        static void loadData(int max_field_size);
        // static void close();
        static v_ope getOperation(int size, int progress, v_pos &target_pos, int type, int idx);
        static int getOperationCount(int size, int progress, v_pos &target_pos, int type);
    private:
        static v_int idx_memo;

        static int calcOpeIndex(int size, int progress);
        static Pos calcTargetPos(int size, Pos target_pos);
        static uint16_t getParent(int size, int t, int type, int target1, int target2, int idx);
};
#endif