#ifndef MAIN_HPP
#define MAIN_HPP 0
#include <bits/stdc++.h>
using namespace std;


//#define USE_FILE 0
// #define IS_DEBUG_A 0
#define IS_DEBUG_B 0

#define FILE_PATH ""

#define rep(i, n) for (int i = 0; i < n; i++)

#define N 24
typedef struct _ope{
    int x;
    int y;
    int n;
} Ope;

typedef struct _ent{
    int val;  // 値
    int num;  // 番号
} Ent;

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

typedef struct _queue_node {
    Pos pos;
    vector<Ope> ope;
} QueueNode;

typedef vector<Ope> v_ope;
typedef vector<v_ope> vv_ope;
typedef vector<vv_ope> vvv_ope;
typedef vector<vvv_ope> vvvv_ope;
typedef vector<vvvv_ope> vvvvv_ope;
typedef vector<vvvvv_ope> vvvvvv_ope;

typedef vector<Ent> v_ent;
typedef vector<v_ent> vv_ent;

typedef vector<Pos> v_pos;
typedef vector<v_pos> vv_pos;

typedef vector<int> v_int;
typedef vector<v_int> vv_int;
typedef vector<vv_int> vvv_int;
typedef vector<vvv_int> vvvv_int;
typedef vector<vvvv_int> vvvvv_int;

typedef vector<bool> v_bool;
typedef vector<v_bool> vv_bool;
typedef vector<vv_bool> vvv_bool;
#endif