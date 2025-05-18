//debug情報表示する際にコメントを外す
/* #define DEBUG_FIELD */

#include <Field.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>

ENT::ENT(){}
ENT::ENT(const int x, const int y, const int n) : num(n){
  this->p = new int[2]{x, y};
}
ENT::ENT(const ENT& fe) : num(fe.num){
  this->p = new int[2]{fe.p[0], fe.p[1]};
}
ENT::ENT(ENT&& fe) : num(fe.num), p(fe.p){
  fe.p = nullptr;
}
ENT& ENT::operator=(const ENT& fe){
  if(this == &fe) return *this;
  this->p = new int[2]{fe.p[0], fe.p[1]};
  this->num = fe.num;
  return *this;
}
ENT& ENT::operator=(ENT&& fe){
  if(this == &fe) return *this;
  if(fe.p)  delete [] fe.p;
  this->p = fe.p;
  this->num = fe.num;
  fe.p = nullptr;
  return *this;
}
ENT::~ENT(){
  if(this->p) delete [] this->p;
}


PENT::PENT() : p1(nullptr), p2(nullptr){}
PENT::PENT(const PENT& fe) : p1(fe.p1), p2(fe.p2){ }
PENT::PENT(PENT&& fe) : p1(fe.p1), p2(fe.p2){
  fe.p1 = nullptr;
  fe.p2 = nullptr;
}
PENT& PENT::operator=(const PENT& fe){
  if(this == &fe) return *this;
  this->p1 = fe.p1;
  this->p2 = fe.p2;
  return *this;
}
PENT& PENT::operator=(PENT&& fe){
  if(this == &fe) return *this;
  this->p1 = fe.p1;
  this->p2 = fe.p2;
  fe.p1 = nullptr;
  fe.p2 = nullptr;
  return *this;
}
PENT::~PENT(){ }


Field::Field(){}
Field::Field(const int siz, const int *f)
: size(siz)
{
  //コンストラクタ
  //fieldを初期化し、ペアも見つける

  //ペアの数を計算しnum_sizeに代入
  const int num_size = siz * siz / 2;
  int *len = new int[num_size]{};

  this->pentities = new PENT[num_size];

  //ENT*型の2次元配列を用意
  this->field = new ENT**[siz];
  this->confirm = new int*[siz];
  int n;
  ENT *ent;
  //fieldを初期化しながらペアを探す
  for(int y = 0; y < siz; y++){
    this->field[y] = new ENT*[siz];
    this->confirm[y] = new int[siz]{};
    for(int x = 0; x < siz; x++){
      n = *(f + y * siz + x);
      ent = new ENT(x, y, n);
      this->field[y][x] = ent;
      if((len[n]++) == 0){
        //初めて出てきた数字
        this->pentities[n].p1 = ent;
      }else {
        //2回目以降の時
        this->pentities[n].p2 = ent;
      }
    }
  }

  // 解放
  delete [] len;
}

Field::Field(const Field& f)
: size(f.size), answer(f.answer)
{
  //コピーコンストラクタ
  const int num_size = f.size * f.size / 2;
  std::vector<int> len(num_size, 0);
  int x, y, n;
  this->pentities = new PENT[num_size];
  this->field = new ENT**[f.size];
  this->confirm = new int*[f.size];

  for(y = 0; y < f.size; y++){
    this->field[y] = new ENT*[f.size];
    this->confirm[y] = new int[f.size]{};
    for(x = 0; x < f.size; x++){
      ENT *e = new ENT;
      *e = *(f.field[y][x]);
      this->field[y][x] = e;
      this->confirm[y][x] = f.confirm[y][x];
      if((len[e->num]++) == 0){
        this->pentities[e->num].p1 = e;
      }else {
        this->pentities[e->num].p2 = e;
      }
    }
  }
}

Field::Field(Field&& f)
: size(f.size), pentities(f.pentities), field(f.field),
  answer(std::move(f.answer)), confirm(f.confirm)
{
  // ムーブコンストラクタ
  f.pentities = nullptr;
  f.field = nullptr;
  f.confirm = nullptr;
  f.size = 0;
}

Field::~Field(){
  //デストラクター
  this->cleanup();
}

Field& Field::operator=(const Field &f){
  //コピー代入演算子
  if(this == &f)  return (*this); //自分が渡されたら処理しない

  const int fsize = f.getSize();
  const int num_size = fsize * fsize / 2;
  std::vector<int> len(num_size, 0);
  int x, y, n;
  this->size = fsize;
  this->answer = f.getOperate();
  this->pentities = new PENT[num_size];
  this->field = new ENT**[fsize];
  this->confirm = new int*[fsize];

  for(y = 0; y < fsize; y++){
    this->field[y] = new ENT*[fsize];
    this->confirm[y] = new int[fsize]{};
    for(x = 0; x < fsize; x++){
      ENT *e = new ENT;
      *e = *(f.get(x, y));
      this->field[y][x] = e;
      this->confirm[y][x] = f.isConfirm(x, y);
      if((len[e->num]++) == 0){
        this->pentities[e->num].p1 = e;
      }else {
        this->pentities[e->num].p2 = e;
      }
    }
  }
  return (*this);
}

Field& Field::operator=(Field&& f) {
  //ムーブ代入演算子
  if (this == &f) return *this;

  this->cleanup();

  size = f.size;
  pentities = f.pentities;
  field = f.field;
  answer = std::move(f.answer);
  confirm = f.confirm;

  f.pentities = nullptr;
  f.field = nullptr;
  f.confirm = nullptr;
  f.size = 0;
  return *this;
}

void Field::cleanup(){
  //動的確保したメモリの解放
  if(this->pentities){
    delete [] this->pentities;
  }
  if(this->field){
    for(int y = 0; y < this->size; y++){
      for(int x = 0; x < this->size; x++){
        delete this->field[y][x];
      }
      delete [] this->field[y];
    }
    delete [] this->field;
  }

  if(this->confirm){
    for(int y = 0; y < this->size; y++){ delete [] this->confirm[y]; }
    delete [] this->confirm;
  }
}

int Field::getSize() const{
  return this->size;
}

PENT Field::getPair(const int num) const{
  return this->pentities[num];
}

ENT* Field::getPair(const ENT *ent) const{
  PENT pent = this->getPair(ent->num);
  return (pent.p1 == ent) ? pent.p2 : pent.p1;
}

ENT* Field::get(const int x, const int y) const{
  return this->field[y][x];
}

void Field::print() const{
  for(int y = 0; y < this->size; y++){
    for(int x = 0; x < this->size; x++){
      if(this->isConfirm(x, y)){
        std::cout << "\x1b[33m" << this->field[y][x]->num << "\t\x1b[m";
      }else{
        std::cout << this->field[y][x]->num << '\t';
      }
    }
    std::cout << std::endl;
  }

#ifdef DEBUG_FIELD
  // 座標を表示
  for (int i = 0; i < this->size; i++){
    for (int j = 0; j < this->size; j++){
      printf("(%d, %d)%c", this->field[i][j]->p[0], this->field[i][j]->p[1], " \n"[j == this->size - 1]);
    }
  }

  // pentities示
  for(int i = 0; i < this->size * this->size / 2; i++){
    printf("%d: p1={%d, %d, %d}, p2={%d, %d, %d}\n", i, this->pentities[i].p1->p[0], this->pentities[i].p1->p[1], this->pentities[i].p1->num, this->pentities[i].p2->p[0], this->pentities[i].p2->p[1], this->pentities[i].p2->num);
  }
#endif
  std::cout << std::endl;
}

void Field::rotate(const int x, const int y, const int siz){
  if(x < 0 || this->size <= x + siz - 1 || y < 0 || this->size <= y + siz - 1){
    throw std::invalid_argument("rotate: out of range");
  }

  //sizを2で割った時の商
  int a = siz >> 1;
  //sizを2で割った時のあまり(偶数の時は0、奇数の時は1)
  int b = siz & 1;
  ENT *buf;
  int h1, w1, h2, w2;
  int *p_buf;
  //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
#ifdef DEBUG_FIELD
  printf("rotate: x=%d y=%d siz=%d b=%d\n", x, y, siz, b);
#endif
  for(h1 = 0; h1 < a; h1++){
    h2 = siz - h1 - 1;
    for(w1 = 0; w1 < a; w1++){
      w2 = siz - w1 - 1;
      buf = this->get(x + w1, y + h1);
      p_buf = buf->p;
      //座標は左回り
      this->field[y + h1][x + w1]->p = this->field[y + w1][x + h2]->p;
      this->field[y + w1][x + h2]->p = this->field[y + h2][x + w2]->p;
      this->field[y + h2][x + w2]->p = this->field[y + w2][x + h1]->p;
      this->field[y + w2][x + h1]->p = p_buf;

      //fieldの要素は右回り
      this->field[y + h1][x + w1] = this->field[y + w2][x + h1];
      this->field[y + w2][x + h1] = this->field[y + h2][x + w2];
      this->field[y + h2][x + w2] = this->field[y + w1][x + h2];
      this->field[y + w1][x + h2] = buf;
    }
  }
  //奇数の時の真ん中を動かす
  if(b == 1){
    int mw = x + a, mh = y + a;
    for(int i = 0; i < a; i++){
      w1 = x + i;
      w2 = x + siz - i - 1;
      h1 = y + i;
      h2 = y + siz - i - 1;
      buf = this->get(mw, h1);
      p_buf = buf->p;  // 場所を修正

      this->field[h1][mw]->p = this->field[mh][w2]->p;
      this->field[mh][w2]->p = this->field[h2][mw]->p;
      this->field[h2][mw]->p = this->field[mh][w1]->p;
      this->field[mh][w1]->p = p_buf;

      this->field[h1][mw] = this->field[mh][w1];
      this->field[mh][w1] = this->field[h2][mw];
      this->field[h2][mw] = this->field[mh][w2];
      this->field[mh][w2] = buf;
    }
  }

  this->answer.push_back({x, y, siz});
}

//可能なときは1、不可能であれば0
int Field::toPointCheck(const int *from, const int *to, int *buf) const{
  int X = std::abs(to[0] - from[0]);
  int Y = std::abs(to[1] - from[1]);
  int siz = X + Y + 1;
  buf[2] = siz;

  if(from[0] < to[0] && from[1] <= to[1]){
    //fromから見て右下(+fromの右)
    buf[0] = from[0] - Y;
    buf[1] = from[1];
  }else if(to[0] <= from[0] && from[1] < to[1]){
    //fromから見て左下(+fromの下)
    buf[0] = to[0] - Y;
    buf[1] = from[1] - X;
  }else if(from[0] < to[0] && to[1] <= from[1]){
    //fromから見て右上
    buf[0] = from[0];
    buf[1] = to[1];
  }else{
    buf[0] = to[0];
    buf[1] = to[1] - X;
  }
#ifdef DEBUG_FIELD
  printf("toPointCheck: from={%d, %d} to={%d, %d} X=%d Y=%d siz=%d buf={%d, %d, %d}\n", from[0], from[1], to[0], to[1], X, Y, siz, buf[0], buf[1], buf[2]);
#endif

  // 確定した範囲内であるか判定
  for(int dx = 0; dx < siz; dx++) for(int dy = 0; dy < siz; dy++){
    // 回転の中心は移動しない
    if(this->isConfirm(buf[0] + dx, buf[1] + dy) && !(siz%2 == 1 && dx == (siz >> 1) && dy == (siz >> 1))){
      return 0;
    }
  }
  return 1;
}

//成功1、失敗0
int Field::toPoint(const int *from, const int *to){
  int buf[3];
  if(this->toPointCheck(from, to, buf)){
    this->rotate(buf[0], buf[1], buf[2]);
#ifdef DEBUG_FIELD
    std::cout << "Success" << std::endl;
#endif
    return 1;
  }else{
#ifdef DEBUG_FIELD
    std::cout << "False" << std::endl;
#endif
    return 0;
  }
}

void Field::setConfirm(const int x, const int y){
  if(0 <= x && x < this->size && 0 <= y && y < this->size){
    this->confirm[y][x] = 1;
  }else{
    throw std::invalid_argument("setConfirm: out of range");
  }
}
void Field::setConfirm(const int *p){
  this->setConfirm(p[0], p[1]);
}
void Field::setConfirm(const ENT *ent){
  this->setConfirm(ent->p[0], ent->p[1]);
}
void Field::unsetConfirm(const int x, const int y){
  if(0 <= x && x < this->size && 0 <= y && y < this->size){
    this->confirm[y][x] = 0;
  }else{
    throw std::invalid_argument("unsetConfirm: out of range");
  }
}
void Field::unsetConfirm(const int *p){
  this->unsetConfirm(p[0], p[1]);
}

//確定範囲内にあれば1, なければ0を返す
int Field::isConfirm(const int x, const int y) const{
  if(0 <= x && x < this->size && 0 <= y && y < this->size){
    return this->confirm[y][x];
  }
  return 1;
}
int Field::isConfirm(const int *p) const{
  return this->isConfirm(p[0], p[1]);
}

std::vector<std::string> Field::getAnswer() const{
  std::vector<std::string> ret;
  ret.reserve(this->answer.size());
  for(auto& ans : this->answer){
    std::ostringstream oss;
    oss << "{" << ans[0] << ", " << ans[1] << ", " << ans[2] << "}";
    ret.push_back(oss.str());
  }
  return ret;
}

std::vector<std::array<int, 3>> Field::getOperate() const{
  return this->answer;
}

//全て揃っているなら1
int Field::isEnd() const{
  PENT pent;
  for(int i=0; i < this->size * this->size / 2; i++){
    pent = this->getPair(i);
    if(pent.p1->p[0] == pent.p2->p[0]){
      if(pent.p1->p[1] != pent.p2->p[1] + 1 && pent.p1->p[1] != pent.p2->p[1] - 1)  return 0;
    }else if(pent.p1->p[1] == pent.p2->p[1]){
      if(pent.p1->p[0] != pent.p2->p[0] + 1 && pent.p1->p[0] != pent.p2->p[0] - 1)  return 0;
    }else{
      return 0;
    }
  }
  return 1;
}


// 引数のFieldを自分に反映する
void Field::reflection(const Field *f, const int px, const int py, const int as, std::unordered_map<int, int> corr){

  int x, y, n;
  for(int dy = 0; dy < f->getSize(); dy++){
    y = py + dy;
    for(int dx = 0; dx < f->getSize(); dx++){
      x = px + dx;
      *(this->field[y][x]) = *(f->get(dx, dy));
      this->field[y][x]->num = corr[this->field[y][x]->num];
      this->confirm[y][x] = f->isConfirm(dx, dy);
      n = this->field[y][x]->num;
      if(this->pentities[n].p2 == nullptr){
        this->pentities[n].p2 = this->field[y][x];
      }else{
        this->pentities[n].p1 = this->field[y][x];
        this->pentities[n].p2 = nullptr;
      }
    }
  }

  std::vector<std::array<int, 3>> ans = f->getOperate();
  int _as = as == -1 ? this->answer.size() : as;
  int resize_size = ans.size() + _as;

  if(this->answer.size() < resize_size){
    this->answer.resize(resize_size);
  }
  for (int i = 0; i < ans.size(); ++i) {
    this->answer[_as + i] = ans[i];
  }
}

std::shared_ptr<Field> Field::clone() const{
  return std::make_shared<Field>(*this);
}

