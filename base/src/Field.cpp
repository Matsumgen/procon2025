
#include <Field.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

Field::Field(const int siz, int *f){
  //コンストラクタ
  //fieldを初期化し、ペアも見つける

  //ペアの数を計算しnum_sizeに代入
  const int num_size = siz * siz / 2;
  int *len = new int[num_size];

  this->size = siz;
  this->pentities = new PENT[num_size];

  //ENT*型の2次元配列を用意
  this->field = new ENT**[siz];
  for (int i = 0; i < siz; ++i) {
    this->field[i] = new ENT*[siz];
  }
  int n;
  ENT *ent;
  //fieldを初期化しながらペアを探す
  for(int y = 0; y < siz; y++){
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
  std::cout << std::endl;
  for(int y = 0; y < this->size; y++){
    for(int x = 0; x < this->size; x++){
      std::cout << this->field[y][x]->num << '\t';
    }
    std::cout << std::endl;
  }

  /*
  // 座標を表示
  for (int i = 0; i < this->size; i++){
    for (int j = 0; j < this->size; j++){
      printf("(%d, %d)%c", this->field[i][j]->p[0], this->field[i][j]->p[1], " \n"[j == this->size - 1]);
    }
  }
  */
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
  for(h1 = 0; h1 < a; h1++){
    h2 = siz - h1 - 1;
    for(w1 = 0; w1 < a; w1++){
      w2 = siz - w1 - 1;
      buf = this->get(x + w1, y + h1);
      p_buf = buf->p;
      this->field[y + h1][x + w1]->p = this->field[y + w2][x + h1]->p;
      this->field[y + w2][x + h1]->p = this->field[y + h2][x + w2]->p;
      this->field[y + h2][x + w2]->p = this->field[y + w1][x + h2]->p;
      this->field[y + w1][x + h2]->p = p_buf;

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
      this->field[h1][mw]->p = this->field[mh][w1]->p;
      this->field[mh][w1]->p = this->field[h2][mw]->p;
      this->field[h2][mw]->p = this->field[mh][w2]->p;
      this->field[mh][w2]->p = p_buf;

      this->field[h1][mw] = this->field[mh][w1];
      this->field[mh][w1] = this->field[h2][mw];
      this->field[h2][mw] = this->field[mh][w2];
      this->field[mh][w2] = buf;
    }
  }
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
