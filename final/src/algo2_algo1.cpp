#include<algo2_algo1.hpp>

namespace algo2_algo1 {

ControlThread::ControlThread() : field_len(0), fsize(0) {
  this->result = (uint16_t*)malloc(MemObj21::GPU_PROCESS_NUM * (MemObj21::ROT_DEPTH + 1) * sizeof(uint16))
  this->result_buf = (uint16_t*)malloc(MemObj21::GPU_PROCESS_NUM * (MemObj21::ROT_DEPTH + 1) * sizeof(uint16))

  resultOperations.resize(MemObj21::BEAM_WIDTH);
  bresultOperations.resize(MemObj21::BEAM_WIDTH);
  for(auto& v: resultOperaitons)  { v.reserve(350); }
  for(auto& v: bresultOperaitons) { v.reserve(350); }
}

void ControlThread::afterTask() {
  while(true){
    this->waitAfterTask();

    // ompつかう？
    for(size_t i = 0; i < MemObj21::GPU_PROCESS_NUM * (MemObj21::ROT_DEPTH + 1); i += MemObj21::ROT_DEPTH + 1){
      if(this->result[i]){
        size_t t = i, j;
        uint16_t type = this->result[t++];
        std::array<uint16_t, MemObj21::ROT_DEPTH+2> d{};
        d[0] = i % this->field_len;
        for(j = 1; j < MemObj21::ROT_DEPTH + 1 && this->result[t] < 0xffff; ++j) d[j] = this->result[t++];

        uint32_t nowfsize = (type >> 12) & 0b1100;

        if((nowfsize >> 1) <= (type & 0b1111)) {
          uint8_t a = (type >> 12) & 0b11;
          d[j++] = (0b11 << 14) | a; // ridは13bitしか使わない

          type &= 0b1111111111110000;
          uint8_t r = (4 + (a<<1));
          uint16_t b = (type >> r) & 0b11;
          if(b == 0) type |= (1 << r);
          if(b == 1) type |= (0b11 << r);
          if(type & 0b0000111111110000 == 0b0000111111110000){
            type += (1 << 14);
            if(type >= (0b11 << 14)) {
              // 盤面一つ終了
              std::vector<Ope> buf = resultOperations[d[0]];
              for(size_t k = 0; k < j; ++k) {
                uint16_t rid = d[k+1];
                Ope ope;
                if(rid >= 0b11 << 14) {
                  rid &= 0b1111;
                  uint8_t a = (type >> 13) & 0b110;
                  if(rid == 0){
                    ope.data[0] = fsize >> 1;
                    ope.data[1] = a;
                  }else if(rid == 1) {
                    ope.data[0] = fsize >> 1;
                    ope.data[1] = fsize >> 1;
                  }else if(rid == 2) {
                    ope.data[0] = a;
                    ope.data[1] = fsize >> 1;
                  }else {
                    ope.data[0] = a;
                    ope.data[1] = a;
                  }
                  ope.data[2] = (fsize >> 1) - a;
                }else{
                  ope = getParamsCpu(rid, fsize, type);
                }
                buf.push_back(ope);
              }
              resultopes.push_back(buf);
            }
          }
        }

        for(; j < MemObj21::ROT_DEPTH + 2; ++j) d[j] = 0x3fff;

        this->tq_add(fid << 32 | rid1 << 16 | rid2, next_type, resultOperations[fid].size() + openum);
      }
    }
  }
}

}
