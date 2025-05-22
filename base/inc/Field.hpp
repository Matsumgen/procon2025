#ifndef FIELD_HPP_
#define FIELD_HPP_

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#define PRINT_SEPARATOR ' '

//p = {x, y}
//pを交換
struct ENT {
  int num; 
  int *p;  //座標
  ENT();
  ENT(const int x, const int y, const int n);
  ENT(const ENT& fe);
  ENT(ENT&& fe);
  ~ENT();
  ENT& operator=(const ENT& fe);
  ENT& operator=(ENT&& fe);
};

//ENTが変わっても変更する必要なし
//ペアの数字
struct PENT {
  ENT *p1;
  ENT *p2;
  PENT();
  PENT(const PENT& fe);
  PENT(PENT&& fe);
  ~PENT();
  PENT& operator=(const PENT& fe);
  PENT& operator=(PENT&& fe);
};


//fieldはENTを交換
class Field {
public:
  static std::unordered_map<int, int> reallocation(Field &f);
  static std::unordered_map<int, int> reallocation_map(Field &f);
  static Field loadProblem(std::string path);
  Field();
  Field(const int siz, const int *f);
  Field(const Field& f);
  Field(Field &&f);
  virtual ~Field();
  Field& operator=(const Field& f);
  Field& operator=(Field&& f);
  virtual void print() const;
  int getSize() const;
  PENT getPair(const int num) const;
  ENT* getPair(const ENT *ent) const;
  ENT* get(const int x, const int y) const;
  void rotate(const int x, const int y, const int siz);
  int canRotate(const int x, const int y, const int siz) const;
  int toPointCheck(const int *from, const int *to, int *buf) const;
  int toPoint(const int *from, const int *to);
  void setConfirm(const int x, const int y);
  void setConfirm(const int *p);
  void setConfirm(const ENT *ent);
  void unsetConfirm(const int x, const int y);
  void unsetConfirm(const int *p);
  int isConfirm(const int x, const int y) const;
  int isConfirm(const int *p) const;
  virtual std::vector<std::string> getAnswer() const;
  virtual std::vector<std::array<int, 3>> getOperate() const;
  int isEnd() const;
  bool operator <(Field other) const {
    return answer.size() < other.answer.size();
  }
  bool operator >(Field other) const {
    return answer.size() > other.answer.size();
  }
  void reflection(const Field *f, const int px, const int py, const int as, std::unordered_map<int, int> corr);
  virtual std::shared_ptr<Field> clone() const;

protected:
  int size;
  PENT *pentities;
  ENT ***field;
  std::vector<std::array<int, 3>> answer;
  //confirm[size][size] 確定したら1、そうでないなら0
  int **confirm;

  //デスストラクタから切り出し
  void cleanup();

/* private: */
};

// Fieldの一部を分切り取って扱うクラス
// FieldChild作成からreflection実行まで親Fieldの変更はしないほうがいい
// 切り取った部分に孤立した数字があったらバグる
// parentはdeleteしない
class FieldChild : public Field {
public:
  FieldChild(std::shared_ptr<Field> f, const int x, const int y, const int n);
  FieldChild(std::shared_ptr<FieldChild> f, const int x, const int y, const int n);
  FieldChild(const FieldChild& other);
  void reflection();
  virtual std::shared_ptr<Field> clone() const override;
  virtual void print() const override;
  virtual std::vector<std::string> getAnswer() const override;
  virtual std::vector<std::array<int, 3>> getOperate() const override;
  std::array<int, 2> getPxy() const;
protected:
  const int answer_size;
  const int px, py;
  std::shared_ptr<Field> parent;
  std::unordered_map<int, int> correspondence;

};


#endif
