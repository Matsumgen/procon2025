
/*
セグメント方式で行う
盤面を2分割(できれば4分割)し、片方の半分を固定とする
縦・横は偶数となるように
固定は全ての数字が分割されるまで
評価関数は正しいセグメントか>盤面を小さくできるか(縦か横が確定)>距離が近いか
そうすれば、どの数字がどの場所に来るかが決まる


*/

/*

盤面を分割
分割した盤面ごとにビームサーチ
分けれたらそこまでの履歴を一本に
履歴はResultTreeへ

ResultTree res;
MainField field = load();
field.setResultTreePearent(res);
field.run();
  res.operate = operate_hist;
  std::array<MainField, 2> childen = this->bunnkatu();
  childen[0].setResultTreePearent(res.child[0])
  childen[1].setResultTreePearent(res.child[1])

*/


#ifndef ALGO_1_4
#define ALGO_1_4

#include<BitField.hpp>
#include<array>
#include <vector>
#include <cstdint>
#include <memory>

// 長方形を扱うBitFieldを作ってないため、4分割
namespace algo1_4 {
  #define SEGMENT_NUM 4
  #define GC_LIMIT 4194304
  using namespace bf;


  struct ResultTree {
    std::vector<Ope> operate;
    std::array<std::shared_ptr<ResultTree>, SEGMENT_NUM> child;
  };

  struct SeparateResult {
    std::uint16_t rank;
    std::vector<Point> points;
  };



  // 縦横異なる長さでも対応させる
  class VHField : public RBField {
  public:
    static VHField loadCsv(const std::string& path);
    VHField() = default;
    VHField(const std::vector<std::uint16_t> f, const std::uint8_t x_size, const std::uint8_t y_size);
    VHField(const std::vector<std::uint16_t> f, const std::uint8_t size, const std::uint8_t size_y, std::vector<bf::PENT> pent, std::shared_ptr<OperateHist> operate);

    virtual void print() const override;
    virtual bool inField(const std::uint8_t x, const std::uint8_t y) const override;
    VHField copy();

    virtual bool isIndexAdjacent(const std::uint16_t i) const override;

    bool isProblem() const;

    virtual std::uint8_t getSizeY() const;
    SeparateResult isSeparate(const std::uint8_t width, const std::uint8_t height) const;
    std::vector<VHField> separate(const std::uint8_t width, const std::uint8_t height) const;

    /* 非対応の関数
    virtual bool isEnd() const override;
    virtual bool isNumAdjacent(const std::uint16_t num) const;

    */

  protected:
    //size_x は size で代用
    std::uint8_t size_y;
    std::vector<std::vector<std::uint16_t>> separated; //セクションごとの数字の一覧

  };

  struct GCRank {
    VHField f;
    std::uint16_t rank;
    GCRank() = default;
    GCRank(VHField& f, std::uint16_t rank);
    GCRank(VHField&& f, std::uint16_t rank);
  };


  inline std::array<GCRank, GC_LIMIT> GC;
  inline std::array<GCRank, GC_LIMIT> GC_buffer;
  inline std::array<GCRank, 5000> GC_buffer2;
  inline size_t gcb = 0; // GC_bufferの最大インデックス
  inline size_t gcm = 0; // GCの最大インデックス
  inline size_t gcb2 = 0; // GCの最大インデックス

  inline std::array<std::uint16_t, 576> separate_buf1, separate_buf2, separate_buf1_rev;

  VHField beam_search(VHField& f, std::uint8_t width, std::uint8_t height, std::uint8_t deep, size_t beam_width);
  void algo1_4(BitField& f);
}

#endif
