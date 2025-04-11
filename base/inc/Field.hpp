#ifndef FIELD_HPP_
#define FIELD_HPP_

#include <vector>
#include <string>

//p = {x, y}
//pを交換
typedef struct field_entities {
  int num;
  int *p;
} ENT;

//ENTが変わっても変更する必要なし
typedef struct pair_entities {
  ENT *p1;
  ENT *p2;
} PENT;


//fieldはENTを交換
class Field {
public:
  Field(const int siz, int *f);
  void print();
  PENT getPair(int num);
  ENT* getPair(ENT *ent);
  ENT* get(int x, int y);
  void rotate(int x, int y, int siz);

protected:
  int size;
  PENT *pentities;
  ENT ***field;
  std::vector<std::string> answer;

/* private: */
};

Field* getProblem();
void postAnswer();
Field* loadProblem(std::string path);



#endif

