#ifndef FIELD_HPP
#define FIELD_HPP 0

#ifndef _REP_
#define _REP_
#define rep(i, n) for (int i = 0; i < n; i++)
#endif


// #include "all.hpp"
#include <bits/stdc++.h>
using namespace std;


typedef vector<int> v_int;

typedef vector<unsigned short> v_ushort;
typedef vector<v_ushort> vv_ushort;
typedef vector<vv_ushort> vvv_ushort;
typedef vector<vvv_ushort> vvvv_ushort;
typedef vector<vvvv_ushort> vvvvv_ushort;
typedef vector<vvvvv_ushort> vvvvvv_ushort;

typedef vector<pair<int, int>> v_pair_ii;

class Pos {
    public:
        short x, y;
        Pos();
        Pos(short x, short y);
        Pos operator + (const Pos &other) const;
        bool operator == (const Pos &other) const;
        int toInt(int N);
};
typedef vector<Pos> v_pos;

class Ent {
    public:
        short val;
        short num;
        Ent();
        Ent(short val, short num);
};

class Ope {
    public:
        short x;
        short y;
        short n;

        Ope();
        Ope(short x, short y, short n);
        bool operator < (const Ope &other) const;
};
typedef vector<Ope> v_ope;
typedef vector<v_ope> vv_ope;
typedef vector<vv_ope> vvv_ope;
typedef vector<vvv_ope> vvvv_ope;
typedef vector<vvvv_ope> vvvvv_ope;

class Field {
    public:
        short size;
        Ent *ent_mem;
        Pos *pos_mem;

        Field();
        Field(short size);
        Field(short size, Ent *ent_mem, Pos *pos_mem);
        Ent &getEnt(int y, int x);
        Pos &getEntPos(int val, int num);
        Pos getPairPos(const Pos &pos);
        Pos getPairPos(const Ent &ent);
        void getClone(Field *out);
        void rotate(Ope ope);
        void toSmall(int x, int y, int next_size);
        void reallocation();
        void printField();
        void printEntPos();
};
#endif
