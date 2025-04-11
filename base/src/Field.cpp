
#include <Field.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

Field::Field(const int siz, int *f){
  const int num_size = siz * siz / 2;
  int len[num_size];
  memset(len, 0, num_size);

  this->size = siz;
  this->pentities = new PENT[num_size];
  this->field = new ENT**[siz];
  for (int i = 0; i < siz; ++i) {
    this->field[i] = new ENT*[siz];
  }
  int n;
  ENT *ent;
  for(int y = 0; y < siz; y++){
    for(int x = 0; x < siz; x++){
      n = *(f + y * siz + x);
      ent = new ENT;
      ent->p = new int[2]{x, y};
      ent->num = n;
      this->field[y][x] = ent;
      if(len[n] == 0){
        this->pentities->p1 = ent;
        len[n]++;
      }else {
        this->pentities->p2 = ent;
      }
    }
  }
}

PENT Field::getPair(int num){
  return this->pentities[num];
}

ENT* getPair(ENT* ent){
  PENT pent = this->getPair(ent->num);
  return (pent.p1 == ent) ? pent.p1 : pent.p2
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
}

void Field::rotate(int x, int y, int siz){
  int a = siz >> 1;
  int b = siz & 1;
  ENT *buf;
  int h, w, x1, x2, y1, y2;
  int *p_buf;
  for(h = 0; h < a; h++){
    y1 = y + h;
    y2 = y + siz - h - 1;
    for(w = 0; w < a; w++){
      x1 = x + w;
      x2 = x + siz - w - 1;
      buf = this->get(x1, y1);
      p_buf = buf->p;
      this->field[y1][x1]->p = this->field[x2][y1]->p;
      this->field[x2][y1]->p = this->field[y2][x2]->p;
      this->field[y2][x2]->p = this->field[x1][y2]->p;
      this->field[x1][y2]->p = p_buf;

      this->field[y1][x1] = this->field[x2][y1];
      this->field[x2][y1] = this->field[y2][x2];
      this->field[y2][x2] = this->field[x1][y2];
      this->field[x1][y2] = buf;
    }
  }
  if(b == 1){
    int mx = x + a, my = y + a;
    for(int i = 0; i < a; i++){
      x1 = x + i;
      x2 = x + siz - i - 1;
      y1 = y + i;
      y2 = y + siz - i - 1;
      p_buf = buf->p;
      buf = this->get(mx, y1);
      this->field[y1][mx]->p = this->field[my][x1]->p;
      this->field[my][x1]->p = this->field[y2][mx]->p;
      this->field[y2][mx]->p = this->field[my][x2]->p;
      this->field[my][x2]->p = p_buf;

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
