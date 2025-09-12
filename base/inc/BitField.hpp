#ifndef BIT_FIELD_HPP_
#define BIT_FIELD_HPP_

#include <cstdint>
#include <array>
#include <vector>

namespace bf {
  #define PENT_NULL 1024
  #define FIELD_SIZE_MAX 255
  #define FIELD_NUM_SIZE_MAX 1024
  typedef std::array<std::uint8_t, 3> Ope; // {x, y, n}

  struct Point {
    std::uint8_t x;
    std::uint8_t y;

    Point(const std::uint8_t x, const std::uint8_t y);
    Point(const std::uint16_t i, const std::uint8_t siz);

    std::array<std::uint8_t, 2> toArr() const;
    std::uint16_t toi(const std::uint8_t siz) const;

  };

  // 同じ数字のインデックス
  struct PENT {
    std::uint16_t p1;
    std::uint16_t p2;
    PENT();
    PENT(const std::uint16_t p1, const std::uint16_t p2);
  };

  // operateへのindexを用いたアクセスは想定されていない
  struct OperateHist {
    std::vector<Ope> operate;
    std::shared_ptr<OperateHist> before;

    bool operator <(const OperateHist& oh) const;
    bool operator >(const OperateHist& oh) const;

    std::uint16_t size() const;
    void push_back(Ope ope);
    void shrink_to_fit();
    std::vector<Ope> getOperate() const;
    std::shared_ptr<OperateHist> clone() const;
  };

  // 盤面のみを扱うFieldクラス
  // 0 ~ 287
  class BitField{
  public:
    static BitField loadCsv(const std::string& path);
    static BitField randomField(const std::uint8_t size);
    BitField(const std::vector<std::uint16_t> f, const std::uint8_t size);
    BitField() = default;
    virtual ~BitField() = default;

    virtual void print() const;
    virtual void print(const char sep) const;

    std::uint8_t getSize() const;
    std::uint16_t get(const std::uint16_t i) const;
    std::uint16_t get(const std::uint8_t x, const std::uint8_t y) const;
    std::vector<std::uint16_t> getField() const;

    bool inField(const std::uint8_t x, const std::uint8_t y) const;
    virtual bool isEnd() const;
    virtual bool isIndexAdjacent(const std::uint16_t i) const;
    virtual bool isIndexAdjacent(const std::uint8_t x, const std::uint8_t y) const;
    virtual bool toPointCheck(const Point from, const Point to) const;
    virtual bool toPointCheck(const Point from, const Point to, Ope& buf) const;

    virtual bool toPoint(const Point from, const Point to);
    virtual void rotate(const std::uint8_t x, const std::uint8_t y, const std::uint8_t siz);
    virtual void rotate(const Ope ope);

    std::vector<std::uint16_t> reallocationMap() const;
    void reallocation();
    std::vector<std::uint16_t> reallocationD();


  protected:
    std::vector<std::uint16_t> field;
    std::uint8_t size;
  };


  // 履歴保持を可能にした子クラス
  // コピー代入演算子やコピーコンストラクタは削除。代わりにcopy関数を使う
  class RBField : public BitField {
  public:
    static RBField loadCsv(const std::string& path);
    RBField(const std::vector<std::uint16_t> f, const std::uint8_t size);
    RBField(const std::vector<std::uint16_t> f, const std::uint8_t size, std::vector<PENT> pent, std::shared_ptr<OperateHist> operate);
    RBField(BitField& f);
    RBField(BitField&& f);
    RBField(const RBField& f) = default;
    RBField() = default;
    RBField& operator =(const RBField& f) = default;
    bool operator <(const RBField& f) const;
    bool operator >(const RBField& f) const;

    RBField copy();
    virtual void print() const override;
    virtual void print(const char sep) const override;
    virtual void print(const bool show_pair) const;
    virtual void print(const char sep, const bool show_pair) const;

    virtual std::vector<std::array<std::uint8_t, 3>> getOperate() const;
    virtual std::vector<std::string> getAnswer() const;

    virtual bool isEnd() const override;
    virtual bool isNumAdjacent(const std::uint16_t num) const;
    /* virtual bool canRotate(const std::uint8_t x, const std::uint8_t y, const std::uint8_t siz) const; */

    PENT getPent(const std::uint16_t num) const;
    Point getPair(const Point p) const;
    Point getPair(const std::uint8_t x, const std::uint8_t y) const;
    std::uint16_t getPairIndex(const std::uint16_t index) const;

    virtual void rotate(const std::uint8_t x, const std::uint8_t y, const std::uint8_t siz) override;
    virtual void rotate(const Ope ope) override;
     

  protected:
    std::vector<PENT> pent;
    std::shared_ptr<OperateHist> operate;


  };

  // 履歴保持と確定箇所の設定を追加した子クラス
  // 手戻りは考慮されない。
  class BField : public RBField {
  public:
    using RBField::print;
    static BField loadCsv(const std::string& path);
    BField(const std::vector<std::uint16_t> f, const std::uint8_t size);
    BField(BitField& f);
    BField(BitField&& f);
    BField(BField& f);
    BField& operator =(BField& f);

    virtual void print(const char sep, const bool show_pair) const override;

    void setConfirm(const std::uint16_t i);
    void setConfirm(const std::uint8_t x, const std::uint8_t y);
    void setConfirm(const Point p);
    void unsetConfirm(const std::uint16_t i);
    void unsetConfirm(const std::uint8_t x, const std::uint8_t y);
    void unsetConfirm(const Point p);
    bool isConfirm(const std::uint16_t i) const;
    bool isConfirm(const std::uint8_t x, const std::uint8_t y) const;
    bool isConfirm(const Point p) const;

  protected:
    std::vector<bool> confirm;

  };

  std::uint16_t rotatePointIndex(const std::uint16_t i, const uint8_t size, const Ope ope);
  std::uint16_t rotatePointIndex(const std::uint16_t i, const uint8_t size, const std::uint8_t x, const std::uint8_t y, const std::uint8_t n);
  Point rotatePoint(const Point p, const Ope ope);
  Point rotatePoint(const Point p, const std::uint8_t x, const std::uint8_t y, const std::uint8_t n);
}

#endif
