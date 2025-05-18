//デバッグ情報表示したい場合コメントを外す
#define DEBUG_ALGO1_1

#include <algo.hpp>
#include <Field.hpp>
#include <algolib1_1.hpp>

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

using namespace algolib1_1;

#ifdef DEBUG_ALGO1_1
#include<map>
#include <set>
// ノードの比較用：アドレス値（ポインタの生値）で比較
struct PtrLess {
  template <typename T>
  bool operator()(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) const {
    return a.get() < b.get();
  }
};

void buildTree(const std::vector<SATree_ptr>& leaves) {
  std::map<SATree_ptr, std::set<SATree_ptr, PtrLess>, PtrLess> children;
  std::set<SATree_ptr, PtrLess> all_nodes;

  // 子から親へ構築
  for (auto& leaf : leaves) {
    SATree_ptr current = leaf;
    while (current) {
      all_nodes.insert(current);
      SATree_ptr parent = current->getParent();
      if (parent) children[parent].insert(current);
      current = parent;
    }
  }

  // ルートノードを見つける（親が存在しないノード）
  std::vector<SATree_ptr> roots;
  for (auto& node : all_nodes)  if (!node->getParent()) roots.push_back(node);


  // 再帰的に木構造を表示
  std::function<void(SATree_ptr, std::string, bool)> printTree;
  printTree = [&](SATree_ptr node, std::string prefix, bool isLast) {
    std::string currentPrefix = prefix + (isLast ? "└── " : "├── ");
    std::cout << currentPrefix;

    // ノード情報を文字列に変換する関数
    auto printNodeInfo = [](SATree_ptr n) -> std::string {
      std::ostringstream oss;
      oss << "[";
      for (auto& o : n->getOpeTree()->get()) oss << arrToString(o) << "->";
      oss << "]" << n->getStepNum();
      return oss.str();
    };

    // 1本鎖をたどる
    std::vector<SATree_ptr> chain;
    SATree_ptr current = node;
    while (true) {
      chain.push_back(current);
      auto& kids = children[current];
      if (kids.size() != 1) break;
      current = *kids.begin();
    }

    // 表示処理
    if (chain.size() >= 6) {
      // 省略表示：最初の2つ＋...＋最後の1つ
      for (size_t i = 0; i < 2; ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << printNodeInfo(chain[i]);
      }
      std::cout << " -> ... -> " << printNodeInfo(chain.back()) << std::endl;
    } else {
      // 通常の連結表示
      for (size_t i = 0; i < chain.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << printNodeInfo(chain[i]);
      }
      std::cout << std::endl;
    }

    // 最後のノードの子供たちを再帰的に表示
    auto& finalChildren = children[chain.back()];
    size_t i = 0;
    for (auto& child : finalChildren) {
      printTree(child, prefix + (isLast ? "    " : "│   "), (++i == finalChildren.size()));
    }
  };

  for (size_t i = 0; i < roots.size(); ++i)  printTree(roots[i], "", i == roots.size() - 1); 
}

void printSALeaf(std::vector<SALeaf_ptr> leaves){
  std::vector<SATree_ptr> lvs;
  lvs.reserve(leaves.size());
  for(auto& l : leaves){
    lvs.push_back(l->getTree());
  }
  std::cout << leaves.size() << std::endl;
  buildTree(lvs);
}
#endif

// rootの次を保持しておく
//leavesの中でstep数の少ない上位いくつかに対してanalysis()を行い、それ以外は破棄
void alg1_1(Field& f, int rw=1, int deep=1, int leaves_limit=100){
  std::vector<SALeaf_ptr> leaves = {make_SALeaf_ptr(std::make_shared<Field>(f), createTPM4<TPS>(f.getSize(), rw), nullptr)};
  std::vector<SALeaf_ptr> leaves_buf;

/* #ifdef DEBUG_ALGO1_1 */
/*   printSALeaf(leaves); */
/* #endif */

  bool endFlag = false;
  do{
    endFlag = true;
    //deep回先読み
    for(int i = 0; i < deep; i++){
      for(auto& l : leaves){
        /* l->print(); */
        if(l->isEnd()){
          leaves_buf.push_back(std::move(l));
        }else{
          std::vector<SALeaf_ptr> lret = l->analysis();
          std::move(lret.begin(), lret.end(), std::back_inserter(leaves_buf));
          endFlag = false;
        }
      }
      if(leaves_buf.empty()){
        for(auto& l : leaves) l->print();
        throw std::logic_error("leaves is empty.");
      }
      leaves = std::move(leaves_buf);
    }

    // leaves_limit個のみを残す
    if(leaves.size() > leaves_limit){
      std::sort(leaves.begin(), leaves.end(), [](SALeaf_ptr& a, SALeaf_ptr& b){ return a->getTree()->getStepNum() < b->getTree()->getStepNum(); });
      leaves.erase(leaves.begin() + leaves_limit, leaves.end());
    }
#ifdef DEBUG_ALGO1_1
  printSALeaf(leaves);
  /* for(auto& l : leaves) l->print(); */
#endif
  }while(!endFlag);

#ifdef DEBUG_ALGO1_1
  printSALeaf(leaves);
  leaves[0]->print();
  std::cout << "step num: " << leaves[0]->getOperate().size() << std::endl;
#endif
  /* std::vector<Ope> opes = leaves[0]->getTree()->getOperate(); //OpeTree内はFieldChildの偏差が反映されない*/
  std::vector<Ope> opes = leaves[0]->getOperate();
  for(auto& ope : opes){
    /* std::cout << arrToString(ope) << std::endl; */
    f.rotate(ope[0], ope[1], ope[2]);
  }
}
