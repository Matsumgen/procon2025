//debug情報表示する際にコメントを外す
/* #define DEBUG_FIELD */

#include <Field.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>

Field::Field(const int siz, int *f){
  //コンストラクタ
  //fieldを初期化し、ペアも見つける

  //ペアの数を計算しnum_sizeに代入
  const int num_size = siz * siz / 2;
  int *len = new int[num_size]{};

  this->size = siz;
  this->pentities = new PENT[num_size];

  //ENT*型の2次元配列を用意
  this->field = new ENT**[siz];
  this->confirm = new int*[siz];
  int n;
  ENT *ent;
  //fieldを初期化しながらペアを探す
  for(int y = 0; y < siz; y++){
    this->field[y] = new ENT*[siz];
    this->confirm[y] = new int[siz];
    for(int x = 0; x < siz; x++){
      n = *(f + y * siz + x);
      ent = new ENT;
      ent->p = new int[2]{x, y};
      ent->num = n;
      this->field[y][x] = ent;
      if(len[n] == 0){
        //初めて出てきた数字
        this->pentities[n].p1 = ent;  // 最初の要素にしか入ってなさそうだったため修正
        len[n]++;
      }else {
        //2回目以降の時
        this->pentities[n].p2 = ent;  // 最初の要素にしか入ってなさそうだったため修正
      }
    }
  }

  // 解放
  delete [] len;
}

Field::~Field(){
  //デストラクター
  delete [] this->pentities;
  for(int y = 0; y < this->size; y++){
    for(int x = 0; x < this->size; x++){
      delete [] this->field[y][x]->p;
      delete this->field[y][x];
    }
    delete [] this->field[y];
    delete [] confirm[y];
  }
  delete [] this->field;
  delete [] confirm;

}

int Field::getSize(){
  return this->size;
}

PENT Field::getPair(int num){
  return this->pentities[num];
}

ENT* Field::getPair(ENT* ent){
  PENT pent = this->getPair(ent->num);
  return (pent.p1 == ent) ? pent.p2 : pent.p1;  // 両方ともentを返しそうだったので修正
}

ENT* Field::get(int x, int y){
  return this->field[y][x];
}

void Field::print(){
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
#endif
  std::cout << std::endl;
}

void Field::rotate(int x, int y, int siz){
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
  //取り敢えずでの実装
  std::stringstream ss;
  ss << this->answer.size() << ": {" << x << ", " << y << ", " << siz << "}";
  this->answer.push_back(ss.str());
}

//可能なときは1、不可能であれば0
int Field::toPointCheck(int *from, int *to, int *buf){
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

  //確定した範囲内であるか判定
  int _s = siz - 1;
  if(this->isConfirm(buf) || this->isConfirm(buf[0] + _s, buf[1]) || this->isConfirm(buf[0], buf[1] + _s) || this->isConfirm(buf[0] + _s, buf[1] + _s)){
    return 0;
  }
  return 1;
}

//成功1、失敗0
int Field::toPoint(int *from, int *to){
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

void Field::setConfirm(int x, int y){
  this->confirm[y][x] = 1;
}
void Field::setConfirm(int *p){
  this->setConfirm(p[0], p[1]);
}
void Field::setConfirm(ENT *ent){
  this->setConfirm(ent->p[0], ent->p[1]);
}
void Field::unsetConfirm(int x, int y){
  this->confirm[y][x] = 0;
}
void Field::unsetConfirm(int *p){
  this->unsetConfirm(p[0], p[1]);
}

//確定範囲内にあれば1, なければ0を返す
int Field::isConfirm(int x, int y){
  if(0 <= x && x < this->size && 0 <= y && y < this->size){
    return this->confirm[y][x];
  }
  return 1;
}
int Field::isConfirm(int *p){
  return this->isConfirm(p[0], p[1]);
}

std::vector<std::string> Field::getAnswer(){
  return this->answer;
}

/* Field* getProblem(){ */
/* } */

Field* loadProblem(std::string path){
  std::ifstream ifs(path);
  std::string str_buf;
  std::vector<std::string> csvdata;
  while (getline(ifs, str_buf)) {
    csvdata.push_back(str_buf);
  }
  const int siz = csvdata.size();
  int *field = new int[siz * siz];

  for(int y = 0, x; y < siz; y++){
    std::stringstream csvd{csvdata[y]};
    x = 0;
    while(getline(csvd, str_buf, ',')){
      field[y*siz + x] = stoi(str_buf);
      x++;
    }
  }
  return new Field(siz, field);
}

void postAnswer(){
}
