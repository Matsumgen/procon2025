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
  Field(const int siz, const int *f);
  Field(const Field& f);
  ~Field();
  Field& operator=(const Field &f);
  void print() const;
  int getSize() const;
  PENT getPair(const int num) const;
  ENT* getPair(const ENT *ent) const;
  ENT* get(const int x, const int y) const;
  void rotate(const int x, const int y, const int siz);
  int toPointCheck(const int *from, const int *to, int *buf) const;
  int toPoint(const int *from, const int *to);
  void setConfirm(const int x, const int y);
  void setConfirm(const int *p);
  void setConfirm(const ENT *ent);
  void unsetConfirm(const int x, const int y);
  void unsetConfirm(const int *p);
  int isConfirm(const int x, const int y) const;
  int isConfirm(const int *p) const;
  std::vector<std::string> getAnswer() const;
  int isEnd() const;
  bool operator <(Field other) const {
    return answer.size() < other.answer.size();
  }
  bool operator >(Field other) const {
    return answer.size() > other.answer.size();
  }

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

