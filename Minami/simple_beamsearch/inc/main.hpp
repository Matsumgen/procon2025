#ifndef MAIN_HPP
#define MAIN_HPP 0
#include <bits/stdc++.h>
using namespace std;

//#define USE_FILE 0
// #define IS_DEBUG_A 0
#define IS_DEBUG_B 0

#define rep(i, n) for (int i = 0; i < n; i++)

extern int N;

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

typedef vector<Ope> v_ope;
typedef vector<v_ope> vv_ope;
typedef vector<vv_ope> vvv_ope;
typedef vector<vvv_ope> vvvv_ope;
typedef vector<vvvv_ope> vvvvv_ope;

typedef vector<Ent> v_ent;
typedef vector<v_ent> vv_ent;

typedef vector<Pos> v_pos;
typedef vector<v_pos> vv_pos;

typedef vector<int> v_int;
typedef vector<v_int> vv_int;
typedef vector<vv_int> vvv_int;
typedef vector<vvv_int> vvvv_int;
typedef vector<vvvv_int> vvvvv_int;

typedef vector<short> v_short;
typedef vector<v_short> vv_short;

typedef vector<uint8_t> v_uint8_t;
typedef vector<v_uint8_t> vv_uint8_t;

typedef vector<bool> v_bool;
typedef vector<v_bool> vv_bool;
typedef vector<vv_bool> vvv_bool;

typedef struct _state {
    vv_ent field;
    vv_pos ent_pos;
} State;

typedef struct _beam_node {
    State s;
    int score;
    v_ope ope_list;
} BeamNode;

typedef struct _beam_node2{
    BeamNode *p;
    bool operator < (const _beam_node2 &other) const {
        return p->score < other.p->score;
    }
    bool operator > (const _beam_node2 &other) const {
        return p->score > other.p->score;
    }
} BeamNode2;

struct SetNode {
    uint8_t *p;
    
    bool operator < (const SetNode &other) const {
        rep (i, N * N) {
            // if (this->p->at(i)[j] == other.p->at(i)[j]) return true;
            if (this->p[i] < other.p[i]) {
                return true;
            }
            if (this->p[i] > other.p[i]) {
                return false;
            }
        }
        return false;
    }
    /*bool operator > (const SetNode &other) const {
        rep (i, N) rep (j, N) {
            if (this->p->at(i)[j] < other.p->at(i)[j]) {
                return false;
            }
            if (this->p->at(i)[j] > other.p->at(i)[j]) {
                return true;
            }
        }
        return false;
    }
    bool operator == (const SetNode &other) const {
        rep (i, N) rep (j, N) {
            if (this->p->at(i)[j] != other.p->at(i)[j]) return false;
        }
        return true;
    }*/
};
#endif