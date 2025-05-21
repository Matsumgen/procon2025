#ifndef ALGO_LIB_1_1_HPP_
#define ALGO_LIB_1_1_HPP_

#include <Field.hpp>
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace algolib1_1{
  typedef std::array<int, 2> Point; // {x, y}
  typedef std::array<int, 3> Ope; // {x, y, n}

  template <size_t N>
  std::string arrToString(const std::array<int, N>& arr);

  struct OpeTree;
  class StepAnalysisTree;
  class StepAnalysisLeaf;
  typedef std::shared_ptr<OpeTree> OpeTree_ptr;
  typedef StepAnalysisTree SATree;
  typedef StepAnalysisLeaf SALeaf;
  typedef std::shared_ptr<SATree> SATree_ptr;
  typedef std::shared_ptr<SALeaf> SALeaf_ptr;

  template <typename... Args>
  OpeTree_ptr make_OpeTree_ptr(Args&&... args) {
    return std::make_shared<OpeTree>(std::forward<Args>(args)...);
  }
  template <typename... Args>
  SATree_ptr make_SATree_ptr(Args&&... args) {
    return std::make_shared<SATree>(std::forward<Args>(args)...);
  }
  template <typename... Args>
  SALeaf_ptr make_SALeaf_ptr(Args&&... args) {
    return std::make_shared<SALeaf>(std::forward<Args>(args)...);
  }

  /* 点を移動させる時の最短手の一覧を木構造で保存
   * root: from, leaf: to
   * ポインタの向き： 親1<-*子
   * 葉が削除されると、連動して葉の無い節も削除される
   * 循環参照に注意すること
   */
  struct OpeTree{
    Ope ope;
    Point to;
    OpeTree_ptr parent;

    OpeTree(int x, int y, int n, Point to, OpeTree_ptr parent);
    std::vector<Ope> get();
    int getCount();
    OpeTree_ptr getParent();
  };

  struct TP{
    Point target;
    Point to;
    std::vector<Ope> addOpe;
    std::vector<Point> setConfirmPoints;
    std::vector<Point> unsetConfirmPoints;

    TP();
    TP(Point target, Point to);
    TP(Point target, Point to, std::vector<Ope> o);
    TP(Point target, Point to, std::vector<Ope> o, std::vector<Point> sc, std::vector<Point> usc);
    void reflectCorrectionX(int x);
    void reflectCorrectionR(int r, int fsize);
    std::string toString() const;
  };


  struct TargetPoint{
    TP tp;
    std::vector<TargetPoint> next;

    TargetPoint(TP tp, std::vector<TargetPoint> nr);
    TargetPoint(TP tp);

    std::string toString() const;
  };

  /*@:揃える数, +:持ってくる場所, %:確定した場所
  rw1        |rw2
  @+  @*  **  ***  ***  ***  ***  ***  ***  ***  ***
  **  +*  @+  ***  ***  +**  @**  @+*  +@*  %%*  %%*
              @+*  +@*  @**  +**  ***  ***  +@*  @+*
  (0,0,4) 最後ら辺とかこっちの方が良くない？
  %%**  %%%%
  %%**  %%%%
  %%**  ****
  %%**  ****
  
  */
  // TPの一覧を保存する抽象クラス
  // 横方向の進みも扱う
  class BaseTargetPoints{
  public:
    BaseTargetPoints(int fsize, int rw, int mode);
    virtual ~BaseTargetPoints() = default;
    virtual void setup(const int mode) = 0;
    virtual std::vector<TP> get(int d) const = 0;
    virtual void update(int index) = 0;
    virtual int size() const = 0;
    virtual std::string toString() const = 0;
    virtual std::shared_ptr<BaseTargetPoints> clone() const = 0;
    virtual std::shared_ptr<BaseTargetPoints> clone(int fsize) const = 0;
    virtual int getFsize() const;
    bool isEnd() const;
    bool isLast() const;
  protected:
    int fsize;
    int rw;
    int x;
    int mode;
    bool endFlag;
    bool lastFlag;
  };

  //左上を起点として、size=rw+1で回転させた時に1~2行揃えることのできるTPの一覧
  class TPS : public BaseTargetPoints{
  public:
    TPS(int fsize, int mode);
    TPS(int fsize, int rw, int mode);
    virtual void setup(const int mode) override;
    virtual std::vector<TP> get(int d) const override;
    virtual void update(int index) override;
    virtual int size() const override;
    virtual int getFsize() const override;
    virtual std::string toString() const override;
    virtual std::shared_ptr<BaseTargetPoints> clone() const override;
    virtual std::shared_ptr<BaseTargetPoints> clone(int fsize) const override;
  private:
    int rw;
    std::vector<bool> enable;
    std::vector<TargetPoint> next;
    static std::vector<TargetPoint> tps;
  };

  // BaseTargetPointsの相対座標と向きを管理する
  class BaseTargetPointsManager{
  public:
    BaseTargetPointsManager() = default;
    virtual ~BaseTargetPointsManager() = default;
    virtual std::vector<TP> get() const = 0;
    virtual void update(int index) = 0;
    virtual bool isLast() const = 0;
    virtual bool isEnd() const = 0;
    virtual std::string toString() const = 0;
    virtual std::shared_ptr<BaseTargetPointsManager> clone() const = 0;
    virtual std::shared_ptr<BaseTargetPointsManager> clone(int fsize) const = 0;

  };

  // 4方向を管理するクラス
  class TPM4 : public BaseTargetPointsManager{
  public:
    TPM4(std::array<std::shared_ptr<BaseTargetPoints>, 4> btp);
    virtual std::vector<TP> get() const override;
    virtual void update(int index) override;
    virtual bool isLast() const override;
    virtual bool isEnd() const override;
    virtual std::string toString() const override;
    virtual std::shared_ptr<BaseTargetPointsManager> clone() const override;
    virtual std::shared_ptr<BaseTargetPointsManager> clone(int fsize) const override;
  protected:
    std::array<std::shared_ptr<BaseTargetPoints>, 4> btp;
  };

  // L字で2方向を管理するクラス
  /* class TPM2L : BaseTargetPointsManager{ */
  /* } */

  /*  ステップなどを解析した結果を木構造で保存
   *  ポインタの向き：  親<-子
   *  葉はvectorのsizeが0
   *  stepには現在までのステップ数を保持。
   *  rootはnullptr
   *  循環参照に注意すること
   *  StepAnalysisLeafのFieldで全て保存できてる？
   */
  class StepAnalysisTree{
  public:
    StepAnalysisTree(std::shared_ptr<OpeTree> opt, std::shared_ptr<StepAnalysisTree> parent);
    std::vector<Ope> getOperate();
    OpeTree_ptr getOpeTree() const;
    SATree_ptr getParent() const;
    int getStepNum() const;
  private:
    OpeTree_ptr opt;
    SATree_ptr parent;
    int step_num;
  };

  /*  StepAnalysisTreeの葉。木の増強などを行う
   *  Field fはFieldChildだったとしても親は考慮しない(Fieldとしてしか扱わない)
   *  ポインタの向き：  StepAnalysisTreeの葉<-StepAnalysisLeaf
   */
  class StepAnalysisLeaf{
  public:
    StepAnalysisLeaf(std::shared_ptr<Field> f, std::shared_ptr<BaseTargetPointsManager> tpm, SATree_ptr leaf);
    std::vector<SALeaf_ptr> analysis();
    bool isEnd() const;
    void setEnd();
    std::shared_ptr<Field> getField() const;
    SATree_ptr getTree() const;
    void print() const;
    std::vector<Ope> getOperate();

  private:
    std::shared_ptr<Field> f;
    std::shared_ptr<BaseTargetPointsManager> tpm;
    SATree_ptr leaf;
    bool endFlag;
  };

  std::vector<OpeTree_ptr> serchShortestStep2(std::shared_ptr<Field>& f, TP& tp);

  template<class T>
  std::shared_ptr<TPM4> createTPM4(int fsize, int rw){
    static_assert(std::is_base_of<BaseTargetPoints, T>::value, "T must derive from BaseTargetPoints");
    std::array<std::shared_ptr<BaseTargetPoints>, 4> btp = {
      std::static_pointer_cast<BaseTargetPoints>(std::make_shared<T>(fsize, rw, 1)),
      std::static_pointer_cast<BaseTargetPoints>(std::make_shared<T>(fsize, rw, 1)),
      std::static_pointer_cast<BaseTargetPoints>(std::make_shared<T>(fsize, rw, 1)),
      std::static_pointer_cast<BaseTargetPoints>(std::make_shared<T>(fsize, rw, 1))
    };
    return std::make_shared<TPM4>(btp);
  }

}

#endif
