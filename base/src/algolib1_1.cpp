//デバッグ情報表示したい場合コメントを外す
/* #define DEBUG_ALGO_LIB1_1 */
/* #define DEBUG_ALGO_LIB1_1_VVV */

#include<algolib1_1.hpp>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>

using namespace algolib1_1;


template <size_t N>
std::string algolib1_1::arrToString(const std::array<int, N>& arr){
  std::ostringstream oss;
  oss << "{";
  for (size_t i = 0; i < N; ++i) {
    oss << arr[i];
    if (i != N - 1) oss << ", ";
  }
  oss << "}";
  return oss.str();
}

OpeTree::OpeTree(int x, int y, int n, Point to, OpeTree_ptr parent)
: ope({x, y, n}), to(to), parent(parent) {}

std::vector<Ope> OpeTree::get(){
  if(!this->parent) return (this->ope[2] == 0) ? std::vector<Ope>() :  std::vector<Ope>(1, this->ope);
  std::vector<Ope> ret = this->parent->get();
  ret.push_back(this->ope);
  return ret;
}
int OpeTree::getCount(){  return (this->parent) ? this->parent->getCount() + 1 : ((this->ope[2] == 0) ? 0 : 1); }
OpeTree_ptr OpeTree::getParent(){ return this->parent; }

TP::TP(Point target, Point to, std::vector<Ope> o, std::vector<Point> sc, std::vector<Point> usc)
  : target(target), to(to), addOpe(o), setConfirmPoints(sc), unsetConfirmPoints(usc) {}

TP::TP(Point target, Point to, std::vector<Ope> o) : target(target), to(to), addOpe(o) {
  std::vector<Point> sc = { to, target };
  for(auto& e : o){
    sc[0] = { e[0]+e[2] - (sc[0][1]-e[1]) - 1, e[1] + (sc[0][0]-e[0]) };
    sc[1] = { e[0]+e[2] - (sc[1][1]-e[1]) - 1, e[1] + (sc[1][0]-e[0]) };
  }
  this->setConfirmPoints = std::move(sc);
}
TP::TP(Point target, Point to)
: TP(target, to, std::vector<Ope>(), { target, to }, std::vector<Point>()) {}
TP::TP() : TP({0, 0}, {0, 0}) {}


std::string TP::toString() const{
  std::ostringstream oss;
  oss << "{target: " << arrToString(this->target);
  oss << ", to: " << arrToString(this->to);
  oss << ", addOpe: {";
  for (size_t i = 0; i < this->addOpe.size(); ++i) {
    oss << arrToString(this->addOpe[i]);
    if (i != addOpe.size() - 1) oss << ", ";
  }
  oss << "}, scp: {";
  for (size_t i = 0; i < this->setConfirmPoints.size(); ++i) {
    oss << arrToString(this->setConfirmPoints[i]);
    if (i != setConfirmPoints.size() - 1) oss << ", ";
  }
  oss << "}, uscp: {";
  for (size_t i = 0; i < this->unsetConfirmPoints.size(); ++i) {
    oss << arrToString(this->unsetConfirmPoints[i]);
    if (i != unsetConfirmPoints.size() - 1) oss << ", ";
  }

  oss << "}}";
  return oss.str();
}

void TP::reflectCorrectionX(int x){
  this->target[0] += x;
  this->to[0] += x;
  for(auto& ao : this->addOpe) ao[0] += x;
  for(auto& sc : this->setConfirmPoints)  sc[0] += x;
  for(auto& uc : this->unsetConfirmPoints)  uc[0] += x;
}

void TP::reflectCorrectionR(int r, int fsize){
  auto rotate = [&](int& x, int& y, int z = 1) {
    int tx = x, ty = y;
    if(r == 1){
      x = fsize - ty - z;
      y = tx;
    }else if(r == 2){
      x = fsize - tx - z;
      y = fsize - ty - z;
    }else if(r == 3){
      x = ty;
      y = fsize - tx - z;
    }
  };
  auto rotateVec3 = [&](std::vector<Ope>& vec) {
    for (auto& ao : vec) {
      rotate(ao[0], ao[1], ao[2]);
    }
  };
  auto rotateVec2 = [&](std::vector<Point>& vec) {
    for (auto& ao : vec) {
      rotate(ao[0], ao[1]);
    }
  };
  rotate(this->target[0], this->target[1]);
  rotate(this->to[0], this->to[1]);
  rotateVec3(this->addOpe);
  rotateVec2(this->setConfirmPoints);
  rotateVec2(this->unsetConfirmPoints);
}

TargetPoint::TargetPoint(TP tp, std::vector<TargetPoint> nr) : tp(tp), next(nr) {}
TargetPoint::TargetPoint(TP tp) : tp(tp), next(std::vector<TargetPoint>()) {}

std::string TargetPoint::toString() const {
  std::ostringstream oss;
  oss << "{tp: " << this->tp.toString() << "\n\tnext: {";
  for (size_t i = 0; i < this->next.size(); ++i) {
    oss << this->next[i].toString();
    if (i != next.size() - 1) oss << ",\n";
  }
  oss << "}\n}";
  return oss.str();
}

BaseTargetPoints::BaseTargetPoints(int fsize, int rw, int mode)
: fsize(fsize), rw(rw), mode(mode), endFlag(false), lastFlag(false) {}

bool BaseTargetPoints::isEnd() const { return this->endFlag; }
bool BaseTargetPoints::isLast() const { return this->lastFlag; }
int BaseTargetPoints::getFsize() const { return this->fsize; }


std::vector<TargetPoint> TPS::tps = {};

TPS::TPS(int fsize, int mode=0) : TPS(fsize, 1, mode) {}
TPS::TPS(int fsize, int rw, int mode=0) : BaseTargetPoints(fsize, rw, mode) {
  this->rw = rw;
  this->setup(mode);
  if(TPS::tps.empty()){
    TargetPoint tmp = TargetPoint(TP({0, 1}, {0, 2}, {{0, 1, 2}}));
    TPS::tps = {
      TargetPoint(TP({0, 0}, {0, 1}, {{0, 0, 2}}), {tmp}),    //最後の処理
      TargetPoint(TP({0, 0}, {0, 1})),                        //縦1
      TargetPoint(TP({0, 0}, {1, 0}), {TargetPoint(TP({0, 1}, {1, 1})), tmp}), //横
      TargetPoint(TP({0, 1}, {1, 1}, {{0, 0, 2}}))            //縦1
    };
  }
  int nowRw = 1 + (TPS::tps.size() - 4)/8;
  if(nowRw < rw){
    std::vector<Point> sc = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    for(int i = nowRw+1, j; i <= rw; i++){ // i = size - 1
      std::vector<Point> usc = {{0, i}, {0, i-1}, {1, i}, {1, i-1}};
      j = 4 + (i - 2) * 8;
      std::vector<TargetPoint> tmp = {
          TargetPoint(TP({1,i}, {1,i-1}, {{0, 0, i+1}}, sc, usc)),
          TargetPoint(TP({1,i-1}, {1,i}, {{0, 0, i+1}}, sc, usc)),
          TargetPoint(TP({0, i}, {1, i}, {{0, 0, i+1}}, sc, usc)),
          TargetPoint(TP({1, i}, {0, i}, {{0, 0, i+1}}, sc, usc))
        };
      // 横
      TPS::tps.push_back(TargetPoint(TP({0,i-1}, {0,i}, {{0, 0, i+1}}), TPS::tps[2].next));
      TPS::tps.push_back(TargetPoint(TP({0,i}, {0,i-1}, {{0, 0, i+1}}), TPS::tps[2].next));
      // 横2*2
      TPS::tps.push_back(TargetPoint(TP({0,i-1}, {0,i}), {tmp[0], tmp[1]}));
      TPS::tps.push_back(TargetPoint(TP({0,i}, {0,i-1}), {tmp[0], tmp[1]}));

      // 縦1
      TPS::tps.push_back(TargetPoint(TP({0, i}, {1, i}, {{0, 0, i+1}})));
      TPS::tps.push_back(TargetPoint(TP({1, i}, {0, i}, {{0, 0, i+1}})));
      // 縦2*2
      TPS::tps.push_back(TargetPoint(TP({0, i-1}, {1, i-1}), {tmp[2], tmp[3]}));
      TPS::tps.push_back(TargetPoint(TP({1, i-1}, {0, i-1}), {tmp[2], tmp[3]}));
    }
#ifdef DEBUG_ALGO_LIB1_1
    std::cout << "TPS constructor rw=" << rw << std::endl;
    for(auto& t : TPS::tps){
      std::cout << t.toString() << std::endl;
    }
#endif
  }
}

// 開始時のenable生成
// fsize: 揃える長さ
// mode:  mode == 1で初めに横1列になるのを防ぐ
void TPS::setup(const int mode=0) {
  this->mode = mode;
  int fs = this->getFsize();
  if(fs <= 2){
    this->enable = std::vector<bool>(4+(this->rw-1)*8, false);
    this->enable[0] = true;
  }
  this->enable = std::vector<bool>(4+(this->rw-1)*8, true);
  if(mode == 1){
    this->enable[0] = false;
    this->enable[2] = false;
    for(int i=4; i < this->enable.size(); i+=8){ // x=1もこうでは？
      this->enable[i] = false;
      this->enable[i+1] = false;
      this->enable[i+2] = false;
      this->enable[i+3] = false;
      this->enable[i+6] = false;
      this->enable[i+7] = false;
    }
  }
  if(fs < this->rw+1){
    for(int i=4+(fs-2)*8; i < this->enable.size(); ++i){
      this->enable[i] = false;
    }
  }
}

std::vector<TP> TPS::get(int d=0) const {
  std::vector<TP> ret;
  if(!this->next.empty()){
    ret.resize(this->next.size());
    for(int i = 0; i < this->next.size(); ++i){
      ret[i] = this->next[i].tp;
      ret[i].reflectCorrectionX(this->x);
      ret[i].reflectCorrectionR(d, this->fsize);
    }
  }else{
    for(int i = 0; i < enable.size(); i++){
      if(enable[i]){
        TP tp = TPS::tps[i].tp;
        tp.reflectCorrectionX(this->x);
        tp.reflectCorrectionR(d, this->fsize);
        ret.push_back(tp);
      }
    }
  }
  return ret;
}


// enableを更新する
// fsize: 揃える長さ
void TPS::update(int index) {
  if(this->next.empty()){
    // 実際のindex取得
    int i=0, j=0;
    for(i = 0, j = 0; i < this->enable.size(); ++i) if(this->enable[i] && j++ == index) break;

    if(!TPS::tps[i].next.empty()){
      this->next = TPS::tps[i].next;
      if(this->enable[0]) this->lastFlag = true;
      return;
    }
    this->x += 1;
  }else{
    if(!this->next[index].next.empty()){
      auto tmp = this->next[index].next;
      this->next = std::move(tmp);
      return;
    }
    this->x += 2;
    this->next.clear();
  }
  int fs = this->getFsize();
  std::fill(enable.begin(), this->enable.end(), false);
  if(fs - 2 <= this->x){
    if(fs - 2 == this->x)      this->enable[0] = true;
    else if(fs - 1 == this->x) throw std::logic_error("TargetPoints.x = fsize - 1");
    else                              this->endFlag = true;
    return;
  }else if(fs - 3 == this->x){
    //縦のみ
    this->enable[1] = true;
    this->enable[3] = true;
    if(this->enable.size() > 4){
      this->enable[8] = true;
      this->enable[9] = true;
    }
  }else{
  int j = 4 + (fs - 2) * 8;
  for(int i=1; i < j && i < enable.size(); ++i) enable[i] = true;
  }
}

int TPS::size() const{
  if(!this->next.empty())  return this->next.size();
  return std::count(this->enable.begin(), this->enable.end(), true);
}

int TPS::getFsize() const{
  return this->mode == 0 ? this->fsize : this->fsize - 2;
}

std::string TPS::toString() const{
  std::ostringstream oss;
  oss << "TPS: x=" << this->x;
  oss << ", fsize=" << this->fsize;
  oss << ", rw=" << this->rw;
  oss << ", mode=" << this->mode;
  oss << ", lastFlag=" << this->lastFlag;
  oss << ", endFlag=" << this->endFlag << std::endl;
  oss << "\tnext={";
  for (size_t i = 0; i < this->next.size(); ++i) {
    oss << this->next[i].toString();
    if (i != next.size() - 1) oss << ", ";
  }
  oss << "}" << std::endl;
  oss << "\tenable = {";
  for(int i=0; i<this->enable.size(); i++){
    oss << (this->enable[i] ? "true" : "false");
    if (i != enable.size() - 1) oss << ", ";
  }
  oss << "}" << std::endl;
  return oss.str();
}

std::shared_ptr<BaseTargetPoints> TPS::clone() const {
  return std::make_shared<TPS>(*this);
}
std::shared_ptr<BaseTargetPoints> TPS::clone(int fsize) const {
  return std::make_shared<TPS>(fsize, this->rw, this->mode);
}



TPM4::TPM4(std::array<std::shared_ptr<BaseTargetPoints>, 4> btp) : btp(btp){ }

std::vector<TP> TPM4::get() const {
  std::vector<TP> ret;
  int ret_size = 0;
  for(auto& t : this->btp)  ret_size += t->size();
  ret.reserve(ret_size);
  for(int i=0; i<4; ++i){
    std::vector<TP> tp = this->btp[i]->get(i);
    std::move(tp.begin(), tp.end(), std::back_inserter(ret));
  }
  return ret;
}

void TPM4::update(int index) {
  int d = 0;
  for(int i=0; i < 4; ++i){
    if(this->btp[i]->size() <= index){
      ++d;
      index -= this->btp[i]->size();
    }else break; 
  }
  this->btp[d]->update(index);
}

bool TPM4::isLast() const{
  if(this->btp[0]->isLast() && this->btp[1]->isEnd() && this->btp[2]->isEnd() && this->btp[3]->isEnd()) return true;
  else if(this->btp[1]->isLast() && this->btp[0]->isEnd() && this->btp[2]->isEnd() && this->btp[3]->isEnd()) return true;
  else if(this->btp[2]->isLast() && this->btp[0]->isEnd() && this->btp[1]->isEnd() && this->btp[3]->isEnd()) return true;
  else if(this->btp[3]->isLast() && this->btp[0]->isEnd() && this->btp[1]->isEnd() && this->btp[2]->isEnd()) return true;
  return false;
}
bool TPM4::isEnd() const{
  return this->btp[0]->isEnd() && this->btp[1]->isEnd() && this->btp[2]->isEnd() && this->btp[3]->isEnd();
}

std::string TPM4::toString() const{
  std::ostringstream oss;
  oss << "TPM4: end=" << this->isEnd() << ", last=" << this->isLast() << std::endl;
  for(int i=0; i<4; ++i){
    oss << "[" << i << "] = " << this->btp[i]->toString();
  }
  return oss.str();
}

std::shared_ptr<BaseTargetPointsManager> TPM4::clone() const {
  std::array<std::shared_ptr<BaseTargetPoints>, 4> cloned_btp;
  for (size_t i = 0; i < 4; ++i) {
    if (btp[i])  cloned_btp[i] = btp[i]->clone();
  }
  return std::make_shared<TPM4>(cloned_btp);
}
std::shared_ptr<BaseTargetPointsManager> TPM4::clone(int fsize) const {
  std::array<std::shared_ptr<BaseTargetPoints>, 4> cloned_btp;
  for (size_t i = 0; i < 4; ++i) {
    if (btp[i]) cloned_btp[i] = btp[i]->clone(fsize);
  }
  return std::make_shared<TPM4>(cloned_btp);
}

StepAnalysisTree::StepAnalysisTree(OpeTree_ptr opt, SATree_ptr parent)
: opt(opt), parent(parent){
  this->step_num = opt->getCount();
  if(parent){
    this->step_num += parent->getStepNum();
  }
}

// 根から葉までの全てのOpeをvectorで入手
std::vector<Ope> StepAnalysisTree::getOperate(){
  if(this->parent){
    std::vector<Ope> ret = this->parent->getOperate();
    std::vector<Ope> o = this->opt->get();
    ret.insert(ret.end(), std::make_move_iterator(o.begin()), std::make_move_iterator(o.end()));
    return ret;
  }else{
    return this->opt->get();
  }
}

int StepAnalysisTree::getStepNum() { return this->step_num; }
SATree_ptr StepAnalysisTree::getParent() { return this->parent; }
OpeTree_ptr StepAnalysisTree::getOpeTree() { return this->opt; }

StepAnalysisLeaf::StepAnalysisLeaf(std::shared_ptr<Field> f, std::shared_ptr<BaseTargetPointsManager> tpm, SATree_ptr leaf)
: f(f), tpm(tpm), leaf(leaf), endFlag(false) {}

// 次の葉を作る
std::vector<SALeaf_ptr> StepAnalysisLeaf::analysis(){
  std::vector<SALeaf_ptr> ret;
  if(this->endFlag) return ret;
  else if(this->f->getSize() <= 6){
    //最終版の処理。
    this->endFlag = true;
    if(this->f->getSize() == 6){
    }else if(this->f->getSize() == 4){
    }else{
      std::cout << "f->size = " << f->getSize() << std::endl;
      throw std::logic_error("f->size < 4");
    }

    ret.push_back(std::shared_ptr<SALeaf>(this));
    return ret;
  }

  auto processOperation = [&](std::shared_ptr<Field> fc, TP& tp, OpeTree_ptr& otp){
    /* fc->print(); */
    for (const auto& ope : otp->get()) {
#ifdef DEBUG_ALGO_LIB1_1
      printf("analysis: rotate(%d, %d, %d)\n", ope[0], ope[1], ope[2]);
#endif
      fc->rotate(ope[0], ope[1], ope[2]);
    }

    if (!tp.addOpe.empty()) {
      Point newTarget = tp.target;
      Point newTo = tp.to;
      for (const auto& ope : tp.addOpe) {
#ifdef DEBUG_ALGO_LIB1_1
        printf("analysis: rotate(%d, %d, %d)\n", ope[0], ope[1], ope[2]);
#endif
        //optに追加
        fc->rotate(ope[0], ope[1], ope[2]);
        newTo = {ope[0]+ope[2] - (newTo[1]-ope[1]) - 1, ope[1] + (newTo[0]-ope[0])};
        newTarget = {ope[0]+ope[2] - (newTarget[1]-ope[1]) - 1, ope[1] + (newTarget[0]-ope[0])};
        otp = make_OpeTree_ptr(ope[0], ope[1], ope[2], newTo, otp);
      }
      if(fc->get(newTo[0], newTo[1])->num != fc->get(newTarget[0], newTarget[1])->num){
        std::cout << "tp: " << tp.toString() << std::endl;
        for (const auto& ope : otp->get()) 
          printf("analysis: rotate(%d, %d, %d)\n", ope[0], ope[1], ope[2]); 
        for (const auto& ope : tp.addOpe)
          printf("analysis: rotate(%d, %d, %d) addOpe\n", ope[0], ope[1], ope[2]); 
        fc->print();
        throw std::runtime_error("Numbers not aligned.");
      }
    }else if(fc->get(tp.target[0], tp.target[1])->num != fc->get(tp.to[0], tp.to[1])->num){
      std::cout << "tp: " << tp.toString() << std::endl;
      for (const auto& ope : otp->get())
        printf("analysis: rotate(%d, %d, %d)\n", ope[0], ope[1], ope[2]); 
      fc->print();
      throw std::runtime_error("Numbers not aligned.");
    }
#ifdef DEBUG_ALGO_LIB1_1
    fc->print();
#endif
  };

  //confirm情報も入れる？
  std::vector<TP> tps = this->tpm->get();
  for(int tpi = 0; tpi < tps.size(); ++tpi){
    std::vector<OpeTree_ptr> opts = serchShortestStep2(this->f, tps[tpi]);
    /* this->f->setConfirm(tps[tpi].target.data()); */
    /* this->f->setConfirm(tps[tpi].to.data()); */
    for(auto& o : opts){
#ifdef DEBUG_ALGO_LIB1_1
      std::cout << "analysis tps[tpi]: " << tps[tpi].toString() << std::endl;
#endif
      std::shared_ptr<Field> fc = this->f->clone();
      for(auto& p : tps[tpi].unsetConfirmPoints){
        fc->unsetConfirm(p.data());
      }
      for(auto& p : tps[tpi].setConfirmPoints){
        fc->setConfirm(p.data());
      }
      processOperation(fc, tps[tpi], o);
      if(this->tpm->isLast()){
        //Fieldを一回り小さくする
        int ns = this->f->getSize() - 4;
        std::shared_ptr<BaseTargetPointsManager> tpmc = this->tpm->clone(ns);
        std::shared_ptr<FieldChild> _fc = std::make_shared<FieldChild>(fc, 2, 2, ns);
        ret.push_back(make_SALeaf_ptr(_fc, tpmc, make_SATree_ptr(o, this->leaf)));
        //一時的に追加・ここで最終版の処理を行う？
        if(ns <= 6){
          ret.back()->setEnd();
        }
      }else{
        std::shared_ptr<BaseTargetPointsManager> tpmc = this->tpm->clone();
        tpmc->update(tpi);
        ret.push_back(make_SALeaf_ptr(std::move(fc), tpmc, make_SATree_ptr(o, this->leaf)));
      }
    }
    /* this->f->unsetConfirm(tps[tpi].target.data()); */
    /* this->f->unsetConfirm(tps[tpi].to.data()); */
    if(this->tpm->isLast()){
      int ns = this->f->getSize() - 4;
      if(ns <= 6){
        /* std::cout << "Not implemented" << std::endl; */
        if(ns == 4){
        }else{
        }
        this->endFlag = true;
      }
    }
  }
  return ret;
}

void StepAnalysisLeaf::print(){
  std::cout << "StepAnalysisLeaf: endFlag=" << endFlag << std::endl;
  std::cout << "Field: " << std::endl;
  this->f->print();
  std::cout << this->tpm->toString() << std::endl;

}

std::vector<Ope> StepAnalysisLeaf::getOperate(){
  return this->f->getOperate();
}

bool StepAnalysisLeaf::isEnd(){ return this->endFlag; }
void StepAnalysisLeaf::setEnd(){ this->endFlag = true; }
SATree_ptr StepAnalysisLeaf::getTree(){ return this->leaf; }

/* x, y: 移動先の座標
 * sn: step数, ep: 目的地, o:  親
 * 戻り値: 目的地へ到達したらtrue
 */
bool sss2(const int x, const int y, const int sn, const Point ep
        , OpeTree_ptr parent
        , std::shared_ptr<Field>& f
        , std::vector<std::vector<int>>& steps
        , std::vector<OpeTree_ptr>& leaf)
{
  if(x < 0 || f->getSize() <= x || y < 0 || f->getSize() <= y || steps[y][x] <= sn) return false;
  int buf[3];
  Point to = {x, y};
#ifdef DEBUG_ALGO_LIB1_1_VVV
  /* std::cout << "sss2: sn=" << sn << ", " << arrToString(parent->to) << "->" << arrToString(to) << std::endl; */
  /* for(int y=0; y<steps.size();++y) { */
  /*   for(int x=0; x<steps[y].size();++x) std::cout << steps[y][x] << " " ; */
  /*   std::cout << std::endl; */
  /* } */
#endif
  if(f->toPointCheck(parent->to.data(), to.data(), buf)){
    steps[y][x] = sn;
    OpeTree_ptr o = make_OpeTree_ptr(buf[0], buf[1], buf[2], to, parent);
    leaf.push_back(o);
    if(ep[0] == x && ep[1] == y){
      return true;
    }
    return  sss2(x - 1, y, sn, ep, parent, f, steps, leaf)
          | sss2(x + 1, y, sn, ep, parent, f, steps, leaf)
          | sss2(x, y - 1, sn, ep, parent, f, steps, leaf)
          | sss2(x, y + 1, sn, ep, parent, f, steps, leaf);
  }
  return false;
}

/* tp: 対象のPointとか
 * 戻り値: tpを揃えるときのstepを木構造で保存したときの全ての葉
 */
std::vector<OpeTree_ptr> algolib1_1::serchShortestStep2(std::shared_ptr<Field>& f, TP& tp){
  if(f->isConfirm(tp.target.data()) || f->isConfirm(tp.to.data())) return std::vector<OpeTree_ptr>();
  int *_from = f->getPair(f->get(tp.target[0], tp.target[1]))->p;
  Point from = {_from[0], _from[1]};
  std::vector<OpeTree_ptr> leaf(1), leaf_buf;
  std::vector<std::vector<int>> step(f->getSize(), std::vector<int>(f->getSize(), 8));

  leaf[0] = make_OpeTree_ptr(0, 0, 0, from, nullptr);
  f->setConfirm(tp.target[0], tp.target[1]);
  for(int y = 0; y < f->getSize(); y++)
    for(int x = 0; x < f->getSize(); x++)
      if(f->isConfirm(x, y))
        step[y][x] = 0;
  
  step[from[1]][from[0]] = 0;
  bool endFlag = false;
  int sn = 1;
  while(!endFlag && !leaf.empty()){
    for(auto& l : leaf){
      endFlag = endFlag
              | sss2(l->to[0]-1, l->to[1], sn, tp.to, l, f, step, leaf_buf)
              | sss2(l->to[0]+1, l->to[1], sn, tp.to, l, f, step, leaf_buf)
              | sss2(l->to[0], l->to[1]-1, sn, tp.to, l, f, step, leaf_buf)
              | sss2(l->to[0], l->to[1]+1, sn, tp.to, l, f, step, leaf_buf);
    }
    leaf = std::move(leaf_buf);
    ++sn;
  }
  f->unsetConfirm(tp.target[0], tp.target[1]);

  //toがtp.toでない葉を切り落とす
  for(auto& l : leaf){
    if(l->to[0] == tp.to[0] && l->to[1] == tp.to[1]){
      leaf_buf.push_back(l);
    }
  }
#ifdef DEBUG_ALGO_LIB1_1
  printf("serchShortestStep2: target: {%d, %d}, from: {%d, %d}, to: {%d, %d}\n", tp.target[0], tp.target[1], from[0], from[1], tp.to[0], tp.to[1]);
#endif
#ifdef DEBUG_ALGO_LIB1_1_VVV
  for(int y=0; y<step.size();++y) {
    for(int x=0; x<step[y].size();++x) std::cout << step[y][x] << " " ;
    std::cout << std::endl;
  }
#endif
  return leaf_buf;
}


