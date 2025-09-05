#ifndef FIELD_HPP
#define FIELD_HPP 0

// #include "all.hpp"
#include <bits/stdc++.h>
using namespace std;

class Pos {
    public:
        short x, y;
        Pos();
        Pos(short x, short y);
        Pos operator + (const Pos &other) const;
        bool operator == (const Pos &other) const;
};

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
};
typedef vector<Ope> v_ope;
typedef vector<v_ope> vv_ope;
typedef vector<vv_ope> vvv_ope;
typedef vector<vvv_ope> vvvv_ope;
typedef vector<vvvv_ope> vvvvv_ope;

/**
 * 盤面を管理するクラス
 */
class Field {
    public:
        short size;
        Ent *ent_mem;  // 各場所に対応するエンティティ
        Pos *pos_mem;  // 各エンティティの場所

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