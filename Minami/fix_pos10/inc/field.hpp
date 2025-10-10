#ifndef FIELD_HPP
#define FIELD_HPP 0

// #include "all.hpp"
#include <vector>
#include <cstdint>


typedef std::vector<int> v_int;
typedef std::vector<v_int> vv_int;
typedef std::vector<vv_int> vvv_int;
typedef std::vector<vvv_int> vvvv_int;
typedef std::vector<vvvv_int> vvvvv_int;

typedef std::vector<unsigned short> v_ushort;
typedef std::vector<v_ushort> vv_ushort;
typedef std::vector<vv_ushort> vvv_ushort;
typedef std::vector<vvv_ushort> vvvv_ushort;
typedef std::vector<vvvv_ushort> vvvvv_ushort;
typedef std::vector<vvvvv_ushort> vvvvvv_ushort;

typedef std::vector<std::pair<int, int>> v_pair_ii;

class Pos {
    public:
        short x, y;
        Pos();
        Pos(short x, short y);
        Pos operator + (const Pos &other) const;
        bool operator == (const Pos &other) const;
        int toInt(int N);
};
typedef std::vector<Pos> v_pos;

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
typedef std::vector<Ope> v_ope;
typedef std::vector<v_ope> vv_ope;
typedef std::vector<vv_ope> vvv_ope;
typedef std::vector<vvv_ope> vvvv_ope;
typedef std::vector<vvvv_ope> vvvvv_ope;

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