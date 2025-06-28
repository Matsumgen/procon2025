#ifndef MAIN_HPP
#define MAIN_HPP 0
#include <bits/stdc++.h>
using namespace std;


//#define USE_FILE 0
// #define IS_DEBUG_A 0
#define IS_DEBUG_B 0

#define DATA_PATH "Data/n_24_max_5.bin"

#define rep(i, n) for (int i = 0; i < n; i++)

typedef vector<int> v_int;
typedef vector<v_int> vv_int;
typedef vector<vv_int> vvv_int;
typedef vector<vvv_int> vvvv_int;
typedef vector<vvvv_int> vvvvv_int;

typedef vector<char> v_char;

typedef vector<bool> v_bool;
typedef vector<v_bool> vv_bool;
typedef vector<vv_bool> vvv_bool;

typedef struct _ope{
    int x;
    int y;
    int n;
} Ope;

typedef vector<Ope> v_ope;
typedef vector<v_ope> vv_ope;
typedef vector<vv_ope> vvv_ope;
typedef vector<vvv_ope> vvvv_ope;
typedef vector<vvvv_ope> vvvvv_ope;

typedef struct _ent{
    int val;  // 値
    int num;  // 番号
} Ent;

typedef vector<Ent> v_ent;
typedef vector<v_ent> vv_ent;

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

typedef vector<Pos> v_pos;
typedef vector<v_pos> vv_pos;

typedef struct _state {
    vv_ent field;
    vv_pos ent_pos;
} State;

typedef struct _beam_node {
    State s;
    v_int idx_list;
    short ope_cnt;
    bool operator < (_beam_node other) const {
        return ope_cnt < other.ope_cnt;
    }
    bool operator > (_beam_node other) const {
        return ope_cnt > other.ope_cnt;
    }
} BeamNode;

typedef struct _beam_node2{
    BeamNode *p;
    bool operator < (_beam_node2 other) const {
        return p->ope_cnt < other.p->ope_cnt;
    }
    bool operator > (_beam_node2 other) const {
        return p->ope_cnt > other.p->ope_cnt;
    }
} BeamNode2;
extern int N;
#endif