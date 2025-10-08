#ifndef FIELD_HPP
#define FIELD_HPP 0

#include <vector>

class Pos {
public:
  short x, y;
  Pos();
  Pos(short x, short y);
  Pos operator+(const Pos &other) const;
  bool operator==(const Pos &other) const;
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

// Ope型を定義した後に、関連する型エイリアスを定義
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