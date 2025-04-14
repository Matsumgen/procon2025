
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
  delete ent;
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
  int h, w, x1, x2, y1, y2;
  int *p_buf;
  //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
  for(h = 0; h < a; h++){
    y1 = y + h;
    y2 = y + siz - h - 1;
    for(w = 0; w < a; w++){
      x1 = x + w;
      x2 = x + siz - w - 1;
      buf = this->get(x1, y1);
      p_buf = buf->p;
      // 座標がおかしくなったため修正(反時計回り)
      this->field[y1][x1]->p = this->field[x1][y2]->p;
      this->field[x1][y2]->p = this->field[y2][x2]->p;
      this->field[y2][x2]->p = this->field[x2][y1]->p;
      this->field[x2][y1]->p = p_buf;

      this->field[y1][x1] = this->field[x2][y1];
      this->field[x2][y1] = this->field[y2][x2];
      this->field[y2][x2] = this->field[x1][y2];
      this->field[x1][y2] = buf;
    }
  }
  //奇数の時の真ん中を動かす
  if(b == 1){
    int mx = x + a, my = y + a;
    for(int i = 0; i < a; i++){
      x1 = x + i;
      x2 = x + siz - i - 1;
      y1 = y + i;
      y2 = y + siz - i - 1;
      buf = this->get(mx, y1);
      p_buf = buf->p;  // 場所を修正
      // 座標がおかしくなったため修正(反時計回り)
      this->field[y1][mx]->p = this->field[my][x2]->p;
      this->field[my][x2]->p = this->field[y2][mx]->p;
      this->field[y2][mx]->p = this->field[my][x1]->p;
      this->field[my][x1]->p = p_buf;

      this->field[y1][mx] = this->field[my][x1];
      this->field[my][x1] = this->field[y2][mx];
      this->field[y2][mx] = this->field[my][x2];
      this->field[my][x2] = buf;
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
