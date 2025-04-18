//デバッグ情報表示したい場合コメントを外す
/* #define DEBUG_ALGO1 */

#include <algo.hpp>
#include <Field.hpp>
#include <vector>
#include <iostream>

//愚直に全探索する
//fromから行ける場所全探索(行ければstepに1)
//結果にtoがなければstepのうち1の場所から全探索(step=0の場所に行ければ2)
//結果にtoがなければstepのうち2の場所から全探索(step=0の場所に行ければ3)
//step数とrotateの引数を返す
//不可能な場合は-1を返す
int serchShortestStep1(Field *f, int *from, int *to, int **ret){
  //step[y][x] = {int num, int from_x, int from_y, int x, int y, int size}
  int fsize = f->getSize();
  //confirmとandをとる？
  std::vector<std::vector<std::vector<int>>> step(fsize, std::vector<std::vector<int>>(fsize, std::vector<int>(6, 10)));
  int to_x, to_y, from_x = from[0], from_y = from[1], s = 0;
  int buf[3];
  int *_from, *_to;
  for(int i = 0; i < fsize; i++){
    for(int j=0; j<fsize; j++){
      step[i][j][0] = f->isConfirm(j, i) ? -1 : 10;
    }
  }

#ifdef DEBUG_ALGO1
  auto printStep = [&](){
    for(int _y = 0; _y < fsize; _y++){
      for(int _x = 0; _x < fsize; _x++){
        /* printf("(%2d,%2d,%2d,%2d,%2d,%2d)", step[_y][_x][0], step[_y][_x][1], step[_y][_x][2], step[_y][_x][3], step[_y][_x][4], step[_y][_x][5]); */
        std::cout << step[_y][_x][0] << '\t';
      }
      std::cout << std::endl;
    }
  };
  printf("serchShortestStep1: from={%d, %d} to={%d, %d}\n", from[0], from[1], to[0], to[1]);
#endif

  step[from[1]][from[0]][0] = 0;
  _from = from;
  while(1){
    for(to_y = 0; to_y < f->getSize(); to_y++){
      for(to_x = 0; to_x < f->getSize(); to_x++){
        _to = f->get(to_x, to_y)->p;
        if(s < step[to_y][to_x][0] && f->toPointCheck(_from, _to, buf)){
          step[to_y][to_x][0] = s + 1;
          step[to_y][to_x][1] = _from[0];
          step[to_y][to_x][2] = _from[1];
          step[to_y][to_x][3] = buf[0];
          step[to_y][to_x][4] = buf[1];
          step[to_y][to_x][5] = buf[2];
        }
      }
    }
/* #ifdef DEBUG_ALGO1 */
/*     printf("from={%d, %d}, s=%d\n", _from[0], _from[1], s); */
/*     printStep(); */
/*     std::cout << std::endl; */
/* #endif */

    if(step[to[1]][to[0]][0] != 10){
#ifdef DEBUG_ALGO1
      std::cout << "serchShortestStep1: return step=" << step[to[1]][to[0]][0] << std::endl;
#endif
      int t[2] = {to[0], to[1]}, a, i = step[to[1]][to[0]][0] - 1;
      while(t[0] != from[0] || t[1] != from[1]){
        ret[i][0] = step[t[1]][t[0]][3];
        ret[i][1] = step[t[1]][t[0]][4];
        ret[i][2] = step[t[1]][t[0]][5];
#ifdef DEBUG_ALGO1
        printf("%d\t{%d, %d, %d}\n", i, ret[i][0], ret[i][1], ret[i][2]);
        /* printf("%d\t{%d, %d, %d}\n", i, step[t[1]][t[0]][3], step[t[1]][t[0]][4], step[t[1]][t[0]][5]); */
#endif
        a = step[t[1]][t[0]][2];
        t[0] = step[t[1]][t[0]][1];
        t[1] = a;
        i--;
      }
      return step[to[1]][to[0]][0];
    }

    do{
      from_x++;
      if(fsize <= from_x){
        from_x = 0;
        from_y++;
        if(fsize <= from_y){
          s++;
          from_y = 0;
          if(s >= 10) return -1;
        }
      }
    }while(step[from_y][from_x][0] != s);

    _from = f->get(from_x, from_y)->p;
  }
}

//とりあえず先読みはしない
void alg1(Field *f){
  int fsize = f->getSize();
  int tp[][2] = {
    {0, 0},     {fsize-1, 0},
    {0, fsize-1}, {fsize-1, fsize-1}
  };
  //それぞれの最短手数を求める
  //最短手数が少ないやつで進める
  int *pairP, pairToP[2];
  //終了時に削除
  int **min_step = new int*[8], **buf_step = new int*[8];
  int min_s = 99, buf_s, min_p, stepC = 0, endMode = (fsize & 2) ? 0 : 1;
  int endStep = fsize / 4 - endMode;
  int confirm_num[4] = {0,0,0,0}, next_stepF[4] = {0,0,0,0};//1は終了2は初期中断3は終期中断
  for(int i=0; i<8; i++){
    min_step[i] = new int[3];
    buf_step[i] = new int[3];
  }
#ifdef DEBUG_ALGO1
  auto algDebugPrint = [&](){
    printf("tp={{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}\n", tp[0][0], tp[0][1], tp[1][0], tp[1][1], tp[2][0], tp[2][1], tp[3][0], tp[3][1]);
    printf("next_stepF={%d, %d, %d, %d}\n", next_stepF[0], next_stepF[1], next_stepF[2], next_stepF[3]);
    f->print();
  };
#endif
  //とりあえず横にのみ揃える
  //mode: 0=縦, 1=横
  auto pairTo = [&](int i, int mode) {
    if(mode){
      if(i == 0){
        pairToP[0] = tp[i][0] + 1;
        pairToP[1] = tp[i][1];
      }else if(i == 1){
        pairToP[0] = tp[i][0];
        pairToP[1] = tp[i][1] + 1;
      }else if(i == 3){
        pairToP[0] = tp[i][0] - 1;
        pairToP[1] = tp[i][1];
      }else if(i == 2){
        pairToP[0] = tp[i][0];
        pairToP[1] = tp[i][1] - 1;
      }
    }else{
      if(i == 0){
        pairToP[0] = tp[i][0];
        pairToP[1] = tp[i][1] + 1;
      }else if(i == 1){
        pairToP[0] = tp[i][0] - 1;
        pairToP[1] = tp[i][1];
      }else if(i == 3){
        pairToP[0] = tp[i][0];
        pairToP[1] = tp[i][1] - 1;
      }else if(i == 2){
        pairToP[0] = tp[i][0] + 1;
        pairToP[1] = tp[i][1];
      }
    }
  };
  auto updateTp = [&](int i) {
    //tp[i]が確定していたら次へ
    //0が1の領域を侵す
    int endF = 0;
    do{
      /* printf("tp[%d]={%d, %d}, %d\n", i, tp[i][0], tp[i][1], (tp[i][1] & 1)); */
      if(i == 0){
        if(tp[i][1] & 1){
          tp[i][0] += 2;
          tp[i][1] -= 1;
          if(fsize <= tp[i][0]){
            tp[i][1] += 2;
            tp[i][0] = tp[i][1];
            next_stepF[i] = 1;
            if(next_stepF[1] == 2) next_stepF[1] = 0; 
          }else if(tp[i][0] == fsize - (stepC << 1) - 4){
            if(!f->isConfirm(tp[i][0]+3, tp[i][1])){
              tp[1][1] += 2;
              next_stepF[1] = 2;
            }else if(!f->isConfirm(tp[i][0]+2, tp[i][1])){
              next_stepF[i] = 3;
            }
          }
        }else{
          tp[i][1]++;
          if(next_stepF[2] == 3)  next_stepF[2] = 0;
        }
      }else if(i == 1){
        if(!(tp[i][0] & 1)){
          tp[i][1] += 2;
          tp[i][0] += 1;
          if(fsize <= tp[i][1]){
            tp[i][0] -= 2;
            tp[i][1] = fsize - tp[i][0] - 1;
            next_stepF[i] = 1;
            if(next_stepF[3] == 2)  next_stepF[3] = 0;
          }else if(tp[i][1] == fsize - (stepC << 1) - 4){
             if(!f->isConfirm(tp[i][0], tp[i][1]+3)){
              tp[3][0] -= 2;
              next_stepF[3] = 2;
            }else if(!f->isConfirm(tp[i][0], tp[i][1]+2)){
              next_stepF[i] = 3;
            }
          }
        }else{
          tp[i][0]--;
          if(next_stepF[0] == 3)  next_stepF[0] = 0;
        }
      }else if(i == 3){
        if(!(tp[i][1] & 1)){
          tp[i][0] -= 2;
          tp[i][1] += 1;
          if(tp[i][0] < 0){
            tp[i][1] -= 2;
            tp[i][0] = tp[i][1];
            next_stepF[i] = 1;
            if(next_stepF[2] == 2)  next_stepF[2] = 0;
          }else if(tp[i][0] == (stepC << 1)  + 3){
            if(!f->isConfirm(tp[i][0]-3, tp[i][1])){
              tp[2][1] -= 2;
              next_stepF[2] = 2;
            }else if(!f->isConfirm(tp[i][0]-2, tp[i][1])){
              next_stepF[i] = 3;
            }
          }
        }else{
          tp[i][1]--;
          if(next_stepF[1] == 3)  next_stepF[1] = 0;
        }
      }else if(i == 2){
        if(tp[i][0] & 1){
          tp[i][1] -= 2;
          tp[i][0] -= 1;
          if(tp[i][1] < 0){
            tp[i][0] += 2;
            tp[i][1] = fsize - tp[i][0] - 1;
            next_stepF[i] = 1;
            if(next_stepF[0] == 2)  next_stepF[0] = 0;
          }else if(tp[i][0] == (stepC << 1) + 3){
            if(!f->isConfirm(tp[i][0], tp[i][1]-3)){
              tp[0][1] += 2;
              next_stepF[0] = 2;
            }else if(!f->isConfirm(tp[i][0], tp[i][1]-2)){
              next_stepF[i] = 3;
            }
          }
        }else{
          tp[i][0]++;
          if(next_stepF[3] == 3)  next_stepF[3] = 0;
        }
      }else{
        std::cout << "ERROR: updateTp i=" << i << std::endl;
        break;
      }
    }while(f->isConfirm(tp[i]) && 0 <= tp[i][0] && tp[i][0] < fsize && 0 <= tp[i][1] && tp[i][1] <= fsize);
    if(next_stepF[0] == 1 && next_stepF[1] == 1 && next_stepF[2] == 1 && next_stepF[3] == 1){
      next_stepF[0] = 0;
      next_stepF[1] = 0;
      next_stepF[2] = 0;
      next_stepF[3] = 0;
      stepC++;
    }
  };
  auto copyStep = [&](){
    for(int i=0; i<8; i++){
      for(int j=0; j<4; j++){
        min_step[i][j] = buf_step[i][j];
      }
    }
  };


#ifdef DEBUG_ALGO1
  algDebugPrint();
#endif
  //2*2で終わる
  while(stepC != endStep){
    min_s = 10;
    for(int i=0;i<4;i++){
      if(next_stepF[i])  continue;
      //step数が同じ場合は最も少ない場所を優先する
      //中央4*4の時は0, 2のみ行う
      pairTo(i, 1);
      pairP = f->getPair(f->get(tp[i][0], tp[i][1]))->p; //異常あり？
      f->setConfirm(tp[i]);
      buf_s = serchShortestStep1(f, pairP, pairToP, buf_step);
      if(buf_s == -1){
        pairTo(i, 0);
        buf_s = serchShortestStep1(f, pairP, pairToP, buf_step);
        buf_step[buf_s][0] = (i <= 1) ? pairToP[0] : pairToP[0] - 1;
        buf_step[buf_s][1] = (i <= 1) ? tp[i][1] : tp[i][1] - 1;
        buf_step[buf_s][2] = 2;
        buf_s++;
      }
      f->unsetConfirm(tp[i]);
      if(buf_s < min_s || (buf_s == min_s && confirm_num[i] < confirm_num[min_p])){
        copyStep();
        min_s = buf_s;
        min_p = i;
      }
    }
#ifdef DEBUG_ALGO1
    printf("alg1: min_s=%d min_p=%d\n", min_s, min_p);
#endif
    for(int i = 0; i < min_s; i++){
#ifdef DEBUG_ALGO1
      printf("alg1/rotate: x=%d y=%d siz=%d\n", min_step[i][0], min_step[i][1], min_step[i][2]);
#endif
      f->rotate(min_step[i][0], min_step[i][1], min_step[i][2]);
    }

    confirm_num[min_p] += 1;
    pairTo(min_p, 1);
    f->setConfirm(tp[min_p]);
    f->setConfirm(pairToP);
    updateTp(min_p);
#ifdef DEBUG_ALGO1
    algDebugPrint();
#endif
  }

  if(endMode){
    for(int i = 0; i < 3; i+=2){
      do{
        pairTo(i, 1);
        pairP = f->getPair(f->get(tp[i][0], tp[i][1]))->p;
        f->setConfirm(tp[i]);
        min_s = serchShortestStep1(f, pairP, pairToP, min_step);
        if(min_s == -1){
          pairTo(i, 0);
          min_s = serchShortestStep1(f, pairP, pairToP, min_step);
          min_step[min_s][0] = (i <= 1) ? pairToP[0] : pairToP[0] - 1;
          min_step[min_s][1] = (i <= 1) ? tp[i][1] : tp[i][1] - 1;
          min_step[min_s][2] = 2;
          min_s++;
        }
        f->unsetConfirm(tp[i]);
        for(int j = 0; j < min_s; j++){
#ifdef DEBUG_ALGO1
          printf("alg1/rotate: x=%d y=%d siz=%d\n", min_step[j][0], min_step[j][1], min_step[j][2]);
#endif
          f->rotate(min_step[j][0], min_step[j][1], min_step[j][2]);
        }
        pairTo(i, 1);
        f->setConfirm(tp[i]);
        f->setConfirm(pairToP);
        updateTp(i);
#ifdef DEBUG_ALGO1
        algDebugPrint();
#endif
      }while(!next_stepF[i]);
    }
  }
  if(f->get(tp[0][0], tp[0][1]-1)->num != f->get(tp[0][0]+1, tp[0][1]-1)->num){
    f->rotate(tp[0][0], tp[0][1]-2, 2);
  }
  if(f->get(tp[0][0]-2, tp[0][1]-1)->num != f->get(tp[0][0]-1, tp[0][1]-1)->num){
    f->rotate(tp[0][0]-2, tp[0][1]-2, 2);
  }
  f->rotate(tp[0][0], tp[0][1]-1, 2);
  f->rotate(tp[0][0]-2, tp[0][1]-1, 3);
  f->setConfirm(tp[0]);
  f->setConfirm(tp[0][0]+1, tp[0][1]);
  f->setConfirm(tp[0][0], tp[0][1]+1);
  f->setConfirm(tp[0][0]+1, tp[0][1]+1);
}


