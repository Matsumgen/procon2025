

void alg1(Field *f){
  int x, y, siz, max_siz, pt*, pa*, b1, b2;
  ENT *target, *pair;
  pt = target->p;
  pa = pair->p;

  target = f->get(x, y);
  pair = f->getPair(target);
  /* pos = {target->p[0] + 1, target->p[1]}; */
  if(pt[0] < pa[0]){
    // targetより右

  }else if(pt[0] == pa[0]){
    // target真下
    siz = pa[1] - pt[1];
    max_siz = f->size - pt[0];
    b1 = siz / max_siz;
    b2 = siz % max_siz;
    if(b1 >= 3){
      if(siz <= pt[0] + 1){
        rotate(pt[0] - siz + 1, pt[1] + 1, siz);
        rotate(pt[0] - siz + 1, pt[1] + 1, siz);
        rotate(pt[0] - siz + 1, pt[1] + 1, siz);
      }else{
        //左下領域へ移動後そのように
      }
    }else{
      switch(b1){
        case 2:
          rotate(pt[0], pa[1] - max_siz + 1, max_siz);
        case 1:
          rotate(pt[0], pa[1] - max_siz + 1, max_siz);
        case 0:
          rotate(pt[0], pt[1] + 1, b2 + 1);
      }
    }
  }

}


