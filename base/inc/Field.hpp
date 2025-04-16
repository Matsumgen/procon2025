#ifndef FIELD_HPP_
#define FIELD_HPP_

#include <vector>
#include <string>

//p = {x, y}
//pを交換
typedef struct field_entities {
  int num; 
  int *p;  //座標
} ENT;

//ENTが変わっても変更する必要なし
//ペアの数字
typedef struct pair_entities {
  ENT *p1;
  ENT *p2;
} PENT;


//fieldはENTを交換
class Field {
public:
  Field(const int siz, int *f);
  void print();
  int getSize();
  PENT getPair(int num);
  ENT* getPair(ENT *ent);
  ENT* get(int x, int y);
  void rotate(int x, int y, int siz);
  int toPointCheck(int *from, int *to, int* buf);
  int toPoint(int *from, int *to);
  void setConfirm(int x, int y);
  void setConfirm(int *p);
  void setConfirm(ENT *ent);
  int isConfirm(int x, int y);
  int isConfirm(int *p);

protected:
  int size;
  PENT *pentities;
  ENT ***field;
  std::vector<std::string> answer;
  //confirm[size][size] 確定したら1、そうでないなら0
  int **confirm;

/* private: */
};

Field* getProblem();
void postAnswer();
Field* loadProblem(std::string path);



#endif

