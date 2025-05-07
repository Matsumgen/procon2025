#include <Field.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include<iostream>

// 数字の振り直し
// map[置き換え後] = 置き換え前
std::unordered_map<int, int> Field::reallocation(Field &f){
  std::unordered_map<int, int> dic, dic_rev;
  int i = 0;
  ENT *ent;
  for(int y = 0; y < f.getSize(); y++){
    for(int x = 0; x < f.getSize(); x++){
      ent = f.get(x, y);
      if(dic_rev.find(ent->num) == dic_rev.end()){
        dic_rev[ent->num] = i;
        dic[i] = ent->num;
        ent->num = i;
        i++;
      }else{
        ent->num = dic_rev[ent->num];
      }
    }
  }
  return dic;
}

// csvファイルから問題を読み込み
Field Field::loadProblem(const std::string path){
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
  Field f(siz, field);
  return f;
}


