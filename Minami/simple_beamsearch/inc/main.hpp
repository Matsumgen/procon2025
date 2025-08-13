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
    short val;  // 値
    uint8_t num;  // 番号
} Ent;

typedef struct _pos{
    uint8_t x;
    uint8_t y;

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
    int size;
    Ent *field;
    Pos *ent_pos;

    void getClone(_state &res) {
        res.size = this->size;
        res.field = new Ent[this->size * this->size];
        res.ent_pos = new Pos[this->size * this->size];
        memcpy(res.field, this->field, this->size * this->size * sizeof(Ent));
        memcpy(res.ent_pos, this->ent_pos, this->size * this->size * sizeof(Pos));
    }

    void getClone(_state &res, Ent *ent_mem, Pos *pos_mem) {
        res.size = this->size;
        res.field = ent_mem;
        res.ent_pos = pos_mem;
        memcpy(ent_mem, this->field, this->size * this->size * sizeof(Ent));
        memcpy(pos_mem, this->ent_pos, this->size * this->size * sizeof(Pos));
    }

    void printState() {
        rep (i, this->size) rep (j, this->size) cout << this->getEnt(i, j).val << " \n"[j == N - 1];
        cout << endl;
    }

    void printPosState() {
        rep (i, this->size * this->size / 2) printf("%d: (%d, %d), (%d, %d)\n", i, this->getEntPos(i, 0).y, this->getEntPos(i, 0).x, this->getEntPos(i, 1).y, this->getEntPos(i, 1).x);
        cout << endl;
    }

    Ent &getEnt(int y, int x) {
        return this->field[y * this->size + x];
    }

    Pos &getEntPos(int val, int num) {
        return this->ent_pos[val * 2 + num];
    }
} State;

class BeamNode {
    public:
        State s;
        int score;
        v_ope ope_list;

        BeamNode() {
        }

        void init(State &s) {
            s.getClone(this->s);
            this->score = 0;
            this->ope_list.clear();
        }

        void getClone(BeamNode &out) {
            this->s.getClone(out.s);
            out.score = this->score;
            out.ope_list = this->ope_list;
        }

        void getClone(BeamNode &out, Ent *ent_mem, Pos *pos_mem) {
            this->s.getClone(out.s, ent_mem, pos_mem);
            out.score = this->score;
            out.ope_list = this->ope_list;
        }
};

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
            if (this->p[i] < other.p[i]) {
                return true;
            }
            if (this->p[i] > other.p[i]) {
                return false;
            }
        }
        return false;
    }
};
#endif