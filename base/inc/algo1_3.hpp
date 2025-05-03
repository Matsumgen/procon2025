#ifndef ALGO1_3_HPP_
#define ALGO1_3_HPP_ 0

#include <Field.hpp>
#include <vector>
#include <queue>

#define rep(i, n) for (int i = 0; i < n; i++)

#define DATA_PATH "Data/bfs_result.bin"

typedef struct _ope {
    int x;
    int y;
    int n;
} Ope;

typedef struct _pos{
    int x;
    int y;

    _pos operator + (_pos other) const {
        return (_pos){x + other.x, y + other.y};
    }

    _pos operator - (_pos other) const {
        return (_pos){x - other.x, y - other.y};
    }

    bool operator == (_pos other) const {
        return x == other.x && y == other.y;
    }

    bool operator != (_pos other) const {
        return x != other.x || y != other.y;
    }
} Pos;

typedef struct _solve_data{
    Pos base_ent_pos;
    int type;
} SolveData;

typedef std::vector<Ope> v_ope;
typedef std::vector<v_ope> vv_ope;
typedef std::vector<vv_ope> vvv_ope;
typedef std::vector<vvv_ope> vvvv_ope;
typedef std::vector<vvvv_ope> vvvvv_ope;

typedef std::vector<Pos> v_pos;
typedef std::vector<v_pos> vv_pos;

typedef std::vector<int> v_int;
typedef std::vector<v_int> vv_int;
typedef std::vector<vv_int> vvv_int;
typedef std::vector<vvv_int> vvvv_int;
typedef std::vector<vvvv_int> vvvvv_int;

typedef std::vector<SolveData> v_solve_data;

typedef std::pair<int, int> p_ii;

void alg1_3(Field& f);
void input_bfs_result(vvvvv_ope &out);

void convert_bfs_result(int N, vvvvv_ope &bfs_result);
void solve(Field &f, vvvvv_ope &bfs_result);
void set_solve_data_recode(int recode, int N, v_solve_data &solve_data);
void set_solve_data_colum(int colum, int N, v_solve_data &solve_data);
int getNextField(Field *f, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result, int idx);
p_ii dfs(Field &f, v_solve_data &solve_data, int cnt, vvvvv_ope &bfs_result, int depth, int max_depth);
Field beamSearch(Field &f, v_solve_data &solve_data, vvvvv_ope &bfs_result, int beam_width);

template<typename T>
void addPriorityQueue(std::priority_queue<T> &p_queue, T &data, int max_size);
#endif
