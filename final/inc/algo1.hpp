#ifndef ALGO1_HPP_
#define ALGO1_HPP_

#include <util.hpp>
#include <array>
#include <vector>
#include <stdint.h>

struct MemObj1;

namespace algo1 {
  #define MAX_ENUM_COUNT 1024
  #define MAX_FIELD_SIZE 24

  #define ROOT_NODE 0xFFFE
  #define BAD_NODE 0xFFFD
  #define NODATA_NODE 0xFFFC

  #define RESULT1_ENUM_CNT 5
  #define RESULT2_ENUM_CNT 5
  #define RESULT3_ENUM_CNT 5

  #define TYPE_CNT4 5
  #define CPU_CNT 20

  const static size_t BEAM_WIDTH = 5000;

  #define getBit(x, y) (((x)>>(y))&1)
  #define setBit(x, y) ((x)|((unsigned long long int)1<<(y)))
  #define resetBit(x, y) ((x)&(~((unsigned long long int)1<<(y))))
  #define reverseBit(x, y) ((x)^((unsigned long long int)1<<(y)))

  enum PairType {
    HORIZON = 0,
    LEFT_0,  // 左のみ
    LEFT_1,  // すでに右が揃っている
    RIGHT_0,  // 左のみ
    RIGHT_1,  // すでに右が揃っている
  };

  enum LastType {
    FLAT = 0,
    OUTSIDE,
    LEFT,
    RIGHT
  };

  template<typename T>
  struct Seg{
    T* array;
    int capacity;
    int start_idx;
    int height;
    T reset_data;
    int size;
    T (*callback_func)(T l, T r);

    Seg();
    Seg(int size, T reset_data, T (*callback)(T, T));
    void set(int idx, T value);
    void remove(int idx);
    T get(int idx);
    T get2(int real_idx);
    T get3(int y, int x);
    T top();
    void check(int check_idx);
  };

  template<typename T>
  Seg<T>::Seg() {
  }

  template<typename T>
  Seg<T>::Seg(int size, T reset_data, T (*callback)(T, T)) {
    int c = 1;
    int height = 0;
    while (c < size){
        c *= 2;
        height++;
    }
    c *= 2;
    height++;
    T* array_ptr = (T*)malloc(c * sizeof(T));
    T* p = array_ptr;
    for (int i = 0; i < c; i++){
        *p = reset_data;
        p++;
    }
    this->array = array_ptr;
    this->capacity = c;
    this->start_idx = c / 2 - 1;
    this->height = height;
    this->reset_data = reset_data;
    this->size = size;
    this->callback_func = callback;
  }
    
  template<typename T>
  void Seg<T>::set(int idx, T value){
    array[start_idx+idx]=value;
    check(start_idx+idx);
  }

  template<typename T>
  void Seg<T>::remove(int idx){
    array[start_idx+idx]=reset_data;
    check(start_idx+idx);
  }

  template<typename T>
  T Seg<T>::get(int idx){
    return array[start_idx+idx];
  }

  template<typename T>
  T Seg<T>::get2(int real_idx){
    return array[real_idx];
  }

  template<typename T>
  T Seg<T>::get3(int y, int x){
    return get2((1<<y)-1+x);
  }

  template<typename T>
  T Seg<T>::top(){
    return array[0];
  }

  template<typename T>
  void Seg<T>::check(int check_idx){
    if (check_idx==0){
        return;
    }
    if (check_idx%2==1){
        array[(check_idx-1)/2]=callback_func(array[check_idx], array[check_idx+1]);
    }else{
        array[(check_idx-1)/2]=callback_func(array[check_idx-1], array[check_idx]);
    }
    check_idx=(check_idx-1)/2;
    check(check_idx);
  }

  class Pos {
    public:
      short x, y;
      Pos();
      Pos(short x, short y);
      Pos operator + (const Pos &other) const;
      bool operator == (const Pos &other) const;
      int toInt(int N);
  };

  class Ent {
    public:
      short val;
      short num;
      Ent();
      Ent(short val, short num);
  };

  class Field {
    public:
      short size;
      Ent *ent_mem;
      Pos *pos_mem;

      bool operator ==(Field &other);
      Field();
      Field(short size);
      Field(short size, Ent *ent_mem, Pos *pos_mem);
      Ent &getEnt(int y, int x);
      Pos &getEntPos(int val, int num);
      Pos getPairPos(const Pos &pos);
      Pos getPairPos(const Ent &ent);
      void getClone(Field *out);
      void rotate(Ope ope);
      void toSmall(int x, int y, int next_size);
      void reallocation();
      void printField();
      void printEntPos();
  };

  class AnsLog {
    public:
      int type;
      int ent;
      int idx;

      AnsLog();
      AnsLog(int type, int ent, int idx);
  };

  class State {
    public:
      int x_hosei, y_hosei;
      int rotate_hosei;
      Field f;
      int edge_cnt;
      int progress;
      int score;
      bool end_flag;
      int last_type;
      int ok_pair;
      int ope_sum;
      std::vector<AnsLog> log;
      State *prev;
      AnsLog last_ope;

      State();
      State(RawField &field, int fsize);
      bool isEnd();
      int getScore();
      void setScore(State *prev, int type, int ent, int idx, MemObj1 &mem1);
      bool isOKType(int type);
      int getNextCount(int type, int ent, MemObj1 &mem1);
      void moveNextState(int type, int ent, int idx, MemObj1 &mem1);
      void getClone(State *out);
      std::vector<Pos> getBasePos(int type);
      void getAnswer(std::vector<AnsLog> &ans_log, RawField &raw_field, std::vector<Ope> &out, std::pair<uint8_t, uint8_t> &offset, MemObj1 &mem1);

    private:
      std::vector<Ope> getOperation(int type, int ent, int idx, MemObj1 &mem1);
      Pos getTmpBasePos(int type);
  };

  class BeamNode {
    public:
      BeamNode();
      BeamNode(State *p, int idx);

      State *p;
      int idx;
      bool operator < (const BeamNode &other) const;
  };

  Ope rotateOpe(Ope ope, int fsize, int r);
  Pos getRotatePos(Pos p, Ope ope);
  Pos intToPos(int p, int size);

  void clearSeg(Seg<BeamNode> *seg);
  BeamNode seg_ope(BeamNode x, BeamNode y);
};

struct MemObj1 {
  size_t size;

  /* bfs結果ファイルで使用  */
  uint16_t **bfs_result4;
  std::vector<Ope> all_ope;  

  /* ビームサーチで使用 */
  algo1::State *state_mem[2];
  algo1::Pos *pos_mem[2];
  algo1::Ent *ent_mem[2];
  algo1::BeamNode *now_beam;
  algo1::BeamNode *tmp_beam;

  /* bfs結果取得関連の関数 */
  std::vector<Ope> getBfsResultOperation(algo1::State *s, std::vector<algo1::Pos> &target_pos, int type, int ent, int idx);
  int getBfsResultOperationCount(algo1::State *s, std::vector<algo1::Pos> &target_pos, int type, int ent);
  size_t getIndex4(int size, int t, int type, int p1, int p2, int idx);
  uint16_t getParent4(int size, int t, int type, int p1, int p2, int idx);
};

MemObj1 init1();
void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields, std::vector<std::pair<uint8_t, uint8_t>> &offsets);
#endif
