#include <algo1.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#include <cstring>
#include <iostream>

using namespace algo1;

// スタブ関数
MemObj1 init1() { 
  MemObj1 mem;
  mem.size = 0;

  /* bfs結果関連のメモリ確保(ファイル読み込み含む) */
  char *base_file_name4 = (char*)"../data/bfs_result12/bfs_result12_n%d_m5.bin";
  FILE *fp;
  char file_name[256];

  std::cout << "Start memory allocation (1)." << std::endl;
  mem.size += 11 * sizeof(uint16_t*);
  mem.bfs_result4 = new uint16_t*[11];
  size_t mem_sum4 = 0;
  for (size_t i = 8; i <= MAX_FIELD_SIZE; i += 2) {
    size_t len = (i * 4 - 8) * TYPE_CNT4 * ((i * i) * (i * i - 1) / 2) * RESULT1_ENUM_CNT;
    mem.size += len * sizeof(uint16_t);
    mem.bfs_result4[i / 2 - 2] = new uint16_t[len];
    sprintf(file_name, base_file_name4, i);
    fp = fopen(file_name, "rb");
    if (fp == NULL) {
      printf("Cannot open \"%s\".\n", file_name);
      std::cout << std::flush;
      exit(1);
    }
    size_t read_mem = fread(&mem.bfs_result4[i / 2 - 2][0], 1, sizeof(uint16_t) * len, fp);
    if (read_mem != len * sizeof(uint16_t)) {
      std::cout << "File read error" << std::endl;
      std::cout << read_mem << " " << sizeof(uint16_t) * len << std::endl;
      exit(1);
    }
    fclose(fp);
    mem_sum4 += len * sizeof(uint16_t);
    printf("size: %ld, len: %ld, use_memory: %f [GB]\n", i, len, (double)(len * sizeof(uint16_t)) / (1 << 30));
    std::cout << std::flush;
  }
  printf("bfs_result4 memory_sum: %f [GB]\n", (double)mem_sum4 / (1 << 30));
  std::cout << "Finish memory allocation (1)." << std::endl;

  mem.all_ope.reserve(23 * (23 + 1) * (2 * 23 + 1) / 6 - 1);
  for (int n = 2; n <= 24; n++) {
    for (int x = 0; x <= 24 - n; x++) {
      for (int y = 0; y <= 24 - n; y++) {
        mem.all_ope.push_back(Ope(x, y, n));
      }
    }
  }
  std::sort(mem.all_ope.begin(), mem.all_ope.end());


  /* ビームサーチ関連のメモリ確保 */
  for (int i = 0; i < 2; i++) {
    size_t state_cnt = (BEAM_WIDTH + 1) * CPU_CNT;
    mem.size += state_cnt * sizeof(State);
    mem.state_mem[i] = new State[state_cnt]{};
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Ent);
    mem.ent_mem[i] = new Ent[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE]{};
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Pos);
    mem.pos_mem[i] = new Pos[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE]{};
    for (size_t j = 0; j < state_cnt; j++) {
      mem.state_mem[i][j].f.ent_mem = mem.ent_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
      mem.state_mem[i][j].f.pos_mem = mem.pos_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
    }
  }

  mem.size += 2 * BEAM_WIDTH * CPU_CNT * sizeof(BeamNode);
  mem.now_beam = new BeamNode[BEAM_WIDTH * CPU_CNT];
  mem.tmp_beam = new BeamNode[BEAM_WIDTH * CPU_CNT];
  std::cout << "Finish memory alloc" << std::endl;
  return mem;
}

void algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields, std::vector<std::pair<uint8_t, uint8_t>> &offsets){
  State first = State(field, fsize);
  first.getClone(&mem1.state_mem[1][0]);

  Seg<BeamNode> seg[CPU_CNT];
  for (int i = 0; i < CPU_CNT; i++) seg[i] = Seg<BeamNode>(BEAM_WIDTH + 1, BeamNode(NULL, -1), seg_ope);

  int beam_cnt = 1;
  int best_final_score = INT_MIN;
  int second_state_idx = 0;
  mem1.now_beam[0] = BeamNode(&mem1.state_mem[1][0], 0);

  for (int i = 0; i < 500; i++) {
    if (beam_cnt == 0) break;
    State *best_state = mem1.now_beam[0].p;
    int par_cpu = beam_cnt / CPU_CNT;

    #pragma omp parallel for
    for (int j = 0; j < CPU_CNT; j++) {
      clearSeg(&seg[j]);
      int beam_start_idx;
      int tmp_beam_cnt;
      if (j < beam_cnt % CPU_CNT) {
        tmp_beam_cnt = par_cpu + 1;
        beam_start_idx = (par_cpu + 1) * j;
      } else {
        tmp_beam_cnt = par_cpu;
        beam_start_idx = (par_cpu + 1) * (beam_cnt % CPU_CNT) + par_cpu * (j - (beam_cnt % CPU_CNT));
      }
      int mem_start_idx = j * (BEAM_WIDTH + 1);
      int tmp_beam_start_idx = j * BEAM_WIDTH;
      for (int k = 0; k < tmp_beam_cnt; k++) {
        BeamNode tmp = mem1.now_beam[beam_start_idx + k];
        for (int type = 0; type < TYPE_CNT4; type++) {
          if (!tmp.p->isOKType(type)) continue;
          int ent_cnt = (tmp.p->edge_cnt == -1) ? 1 : (tmp.p->f.size * tmp.p->f.size / 2);
          for (int l = 0; l < ent_cnt; l++) {
            int next_cnt = tmp.p->getNextCount(type, l, mem1);
            for (int m = 0; m < next_cnt; m++) {
              BeamNode worst = seg[j].top();
              State *next_state = mem1.state_mem[i % 2] + mem_start_idx + worst.idx;
              next_state->setScore(tmp.p, type, l, m, mem1);
              seg[j].set(worst.idx, BeamNode(next_state, worst.idx));
            }
          }
        }
      }
      tmp_beam_cnt = 0;
      for (int k = 0; k < BEAM_WIDTH + 1; k++) {
        if (seg[j].get(k).p == NULL || k == seg[j].top().idx) continue;
        mem1.tmp_beam[tmp_beam_start_idx + tmp_beam_cnt++] = seg[j].get(k);        
      }
      for (int k = tmp_beam_cnt; k < BEAM_WIDTH; k++) mem1.tmp_beam[tmp_beam_start_idx + k].p = NULL;
    }
    memcpy(mem1.now_beam, mem1.tmp_beam, sizeof(BeamNode) * BEAM_WIDTH * CPU_CNT);
    std::sort(mem1.now_beam, mem1.now_beam + (BEAM_WIDTH * CPU_CNT), [&](BeamNode x, BeamNode y) { 
      if (x.p == NULL) {
        return false;
      } else if (y.p == NULL) {
        return true;
      }
      return x.p->getScore() > y.p->getScore(); 
    });
    
    int idx = 0;
    beam_cnt = 0;
    while (idx < BEAM_WIDTH * CPU_CNT && mem1.now_beam[idx].p != NULL && beam_cnt < BEAM_WIDTH) {
      mem1.now_beam[idx].p->prev->getClone(mem1.now_beam[idx].p);
      mem1.now_beam[idx].p->moveNextState(mem1.now_beam[idx].p->last_ope.type, mem1.now_beam[idx].p->last_ope.ent, mem1.now_beam[idx].p->last_ope.idx, mem1);

      if (mem1.now_beam[idx].p->isEnd()) {
        if (second_state_idx < MAX_ENUM_COUNT) {
          std::vector<Ope> tmp_ope;
          RawField tmp_field;
          std::pair<uint8_t, uint8_t> tmp_offset;
          first.getAnswer(mem1.now_beam[idx].p->log, tmp_field, tmp_ope, tmp_offset, mem1);
          opes.push_back(tmp_ope);
          fields.push_back(tmp_field);
          offsets.push_back(tmp_offset);
          second_state_idx++;
        }
        idx++;
        continue;
      }

      if (beam_cnt == 0 || !(mem1.now_beam[beam_cnt - 1].p->f == mem1.now_beam[idx].p->f)) {
        mem1.now_beam[beam_cnt++] = mem1.now_beam[idx];
      }
      idx++;
    }

    printf("(%d, %d, %d, %d, %d, %d)\n", i, beam_cnt, idx, best_state->score, best_state->ope_sum, best_state->ok_pair);
    std::cout << std::flush;
  }
  return;
}

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), edge_cnt(-1), progress(0), log(std::vector<AnsLog>(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0) {
}

State::State(RawField &field, int fsize) : x_hosei(0), y_hosei(0), rotate_hosei(0), edge_cnt(-1), progress(0), log(std::vector<AnsLog>(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0){
  this->f.size = fsize;
  this->f.ent_mem = new Ent[fsize * fsize];
  this->f.pos_mem = new Pos[fsize * fsize];

  int cnt[fsize * fsize / 2];
  for (int i = 0; i < fsize * fsize / 2; i++) cnt[i] = 0;
  for (int i = 0; i < fsize; i++) for (int j = 0; j < fsize; j++) {
    int val = field[i * fsize + j];
    this->f.getEnt(i, j).val = val;
    this->f.getEnt(i, j).num = cnt[val];
    this->f.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};
    cnt[val]++;
  }
}

bool State::isEnd() {
  return this->f.size <= 12;
}

int State::getScore() {
  return this->score;
}

void State::setScore(State *prev, int type, int ent, int idx, MemObj1 &mem1) {
  this->prev = prev;
  this->last_ope = AnsLog(type, ent, idx);

  // 全体回転の場合
  if (prev->edge_cnt == -1) {
    this->score = prev->score;
    return;
  }
  std::vector<Ope> opes = prev->getOperation(type, ent, idx, mem1);
  int new_pair_cnt;
  new_pair_cnt = 1;
  this->ok_pair = prev->ok_pair + new_pair_cnt;
  this->ope_sum = prev->ope_sum + (int)opes.size();
  this->score = -10000 * ope_sum / ok_pair;
}

int State::getNextCount(int type, int ent, MemObj1 &mem1) {
  if (!isOKType(type)) return 0;
  if (this->edge_cnt == -1) return 4 * (type == 0);

  std::vector<Pos> base_pos = this->getBasePos(type);
  std::vector<Pos> target_pos(base_pos.size());
  for (int i = 0; i < (int)base_pos.size(); i++) {
    target_pos[i] = this->f.getPairPos(base_pos[i]);
    for (int j = 0; j < (int)base_pos.size(); j++) {
      if (target_pos[i] == base_pos[j]) {
        return 0;
      }
    }
  }
  return mem1.getBfsResultOperationCount(this, target_pos, ent, type);
}

void State::moveNextState(int type, int ent, int idx, MemObj1 &mem1) {
  if (this->edge_cnt == -1) {
    // 全体回転
    std::vector<Ope> opes = this->getOperation(type, ent, idx, mem1);
    for (Ope &ope : opes) this->f.rotate(ope);
    this->rotate_hosei += opes.size();
    this->rotate_hosei &= 3;
    this->edge_cnt = 0;
    this->progress = 0;
    this->last_type = FLAT;
  } else {
    std::vector<Ope> opes = this->getOperation(type, ent, idx, mem1);
    for (Ope &ope : opes) this->f.rotate(ope);
    int new_pair_cnt;
    new_pair_cnt = 1;
    this->progress += new_pair_cnt;
    if (type == LEFT_0) {
      this->last_type = LEFT;
    } else if (type == RIGHT_0) {
      this->last_type = RIGHT;
    } else {
      this->last_type = FLAT;
    }
    this->ok_pair += new_pair_cnt;
    this->ope_sum += (int)opes.size();
    this->score = -10000 * ope_sum / ok_pair;
    if (this->progress == this->f.size / 2 - (this->edge_cnt & 1) * 2) {
      this->edge_cnt++;
      this->progress = 0;
    }

    if (this->edge_cnt == 8) {
      this->f.toSmall(2, 2, this->f.size - 4);
      this->progress = 0;
      this->edge_cnt = -1;
      this->x_hosei += 2;
      this->y_hosei += 2;
    }
  }
  this->log.push_back(AnsLog(type, ent, idx));
}

void State::getClone(State *out) {
  out->x_hosei = this->x_hosei;
  out->y_hosei = this->y_hosei;
  out->rotate_hosei = this->rotate_hosei;
  this->f.getClone(&out->f);
  out->edge_cnt = this->edge_cnt;
  out->progress = this->progress;
  out->score = this->getScore();
  out->end_flag = this->end_flag;
  out->log = this->log;
  out->last_type = this->last_type;
  out->ok_pair = this->ok_pair;
  out->ope_sum = this->ope_sum;
}

void State::getAnswer(std::vector<AnsLog> &ans_log, RawField &raw_field, std::vector<Ope> &out, std::pair<uint8_t, uint8_t> &offset, MemObj1 &mem1) {
  Ent tmp_ent_mem[this->f.size * this->f.size];
  Pos tmp_pos_mem[this->f.size * this->f.size];
  State tmp_s;
  tmp_s.f.ent_mem = tmp_ent_mem;
  tmp_s.f.pos_mem = tmp_pos_mem;

  out.resize(0);
  this->getClone(&tmp_s);
  for (int i = 0; i < (int)ans_log.size(); i++) {
    if (tmp_s.edge_cnt == -1) {
      // cout << ans_log[i].second << " " << tmp_s.rotate_hosei << " " << tmp_s.x_hosei << " " << tmp_s.y_hosei << endl;
    } else {
      std::vector<Ope> ope_list = tmp_s.getOperation(ans_log[i].type, ans_log[i].ent, ans_log[i].idx, mem1);
      for (Ope &ope : ope_list) {
        Ope tmp = rotateOpe(ope, tmp_s.f.size, tmp_s.rotate_hosei);
        tmp.data[0] += tmp_s.x_hosei;
        tmp.data[1] += tmp_s.y_hosei;
        out.push_back(tmp);
      }    
    }
    tmp_s.moveNextState(ans_log[i].type, ans_log[i].ent, ans_log[i].idx, mem1);
  }
  for (int i = 0; i < (4 - tmp_s.rotate_hosei) % 4; i++) tmp_s.f.rotate(Ope(0, 0, tmp_s.f.size));
  raw_field.resize(tmp_s.f.size * tmp_s.f.size);
  for (int i = 0; i < tmp_s.f.size * tmp_s.f.size; i++) raw_field[i] = tmp_s.f.ent_mem[i].val;
  offset = std::make_pair(tmp_s.x_hosei, tmp_s.y_hosei);
}

std::vector<Ope> State::getOperation(int type, int ent, int idx, MemObj1 &mem1) {
    std::vector<Ope> res;
    if (this->edge_cnt == -1) {
      // 全体回転
      for (int i = 0; i < idx; i++) res.push_back(Ope(0, 0, this->f.size));
    } else {
      std::vector<Pos> base_pos = this->getBasePos(type);
      std::vector<Pos> target_pos(base_pos.size());
      for (int i = 0; i < (int)base_pos.size(); i++) target_pos[i] = this->f.getPairPos(base_pos[i]);

      std::vector<Ope> tmp = mem1.getBfsResultOperation(this, target_pos, type, ent, idx);
      for (Ope &ope : tmp) {
        res.push_back(ope);
      }

      // 端の最後の場合は回転させる
      if (this->progress == this->f.size / 2 - (this->edge_cnt & 1) * 2 - 1) {
        if (this->edge_cnt / 2 == 0) {
          res.push_back(Ope(0, 0, this->f.size / 2));
        } else if (this->edge_cnt / 2 == 1) {
          res.push_back(Ope(this->f.size / 2, 0, this->f.size / 2));
        } else if (this->edge_cnt / 2 == 2) {
          res.push_back(Ope(this->f.size / 2, this->f.size / 2, this->f.size / 2));
        } else {
          res.push_back(Ope(0, this->f.size / 2, this->f.size / 2));
        }
      }
    }
    return res;
}

bool State::isOKType(int type) { 
  if (this->edge_cnt == -1) {
    return type == 0;
  }

  int max_progress = this->f.size / 2 - (this->edge_cnt & 1) * 2;
  switch (type) {
  case HORIZON:
    return this->last_type == FLAT;
  case LEFT_0:
    return this->last_type == FLAT && this->progress <= max_progress - 2;
  case LEFT_1:
    return this->last_type == RIGHT && this->progress >= 1;
  case RIGHT_0:
    return this->last_type == FLAT && this->progress <= max_progress - 2 && !((this->edge_cnt & 1) == 0 && this->progress == 0);
  case RIGHT_1:
    return this->last_type == LEFT && this->progress >= 1;
  default:
    return false;
  }
}

// 回転する前の一時的な基準位置
Pos State::getTmpBasePos(int type) {
  Pos tmp = Pos(this->progress + (this->edge_cnt & 1) * 2, this->f.size / 2 - 2);
  if (type == LEFT_1 || type == RIGHT_1) {
    tmp.x--;
  }
  if (type == LEFT_0 || type == LEFT_1) {
    tmp.y++;
  }
  return tmp;
}

std::vector<Pos> State::getBasePos(int type) {
  Pos tmp = this->getTmpBasePos(type);
  for (int i = 0; i < this->edge_cnt / 2; i++) {
    tmp = getRotatePos(tmp, Ope(0, 0, this->f.size));
  }
  return {tmp};
}

AnsLog::AnsLog() {
}

AnsLog::AnsLog(int type, int ent, int idx) : type(type), ent(ent), idx(idx) {
}

Field::Field() {
}

Field::Field(short size) : size(size) {
  this->ent_mem = new Ent[this->size * this->size];
  this->pos_mem = new Pos[this->size * this->size];
}

Field::Field(short size, Ent *ent_mem, Pos *pos_mem) : size(size), ent_mem(ent_mem), pos_mem(pos_mem) {
}

bool Field::operator ==(Field &other) {
  if (this->size != other.size) return false;
  for (int i = 0; i < this->size; i++) for (int j = 0; j < this->size; j++) {
    if (this->getEnt(i, j).val != other.getEnt(i, j).val) return false;
  }
  return true;
}

Ent& Field::getEnt(int y, int x) {
  return this->ent_mem[y * this->size + x];
}

Pos& Field::getEntPos(int val, int num) {
  return this->pos_mem[val * 2 + num];
}

Pos Field::getPairPos(const Pos &pos) {
  return getPairPos(this->getEnt(pos.y, pos.x));
}

Pos Field::getPairPos(const Ent &ent) {
  return this->getEntPos(ent.val, (ent.num + 1) & 1);
}

void Field::getClone(Field *out) {
  out->size = this->size;
  memcpy(out->ent_mem, this->ent_mem, this->size * this->size * sizeof(Ent));
  memcpy(out->pos_mem, this->pos_mem, this->size * this->size * sizeof(Pos));
}

void Field::rotate(Ope ope) {
  int a = ope.data[2] >> 1;
  int b = ope.data[2] & 1;
  Ent buf;
  int h1, w1, h2, w2;
  //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
  for(h1 = 0; h1 < a; h1++){
    h2 = ope.data[2] - h1 - 1;
    for(w1 = 0; w1 < a; w1++){
      w2 = ope.data[2] - w1 - 1;
      buf = this->getEnt(ope.data[1] + h1, ope.data[0] + w1);

      int dy[4] = {h1, w2, h2, w1};
      int dx[4] = {w1, h1, w2, h2};
      for (int i = 0; i < 4; i++) {
        Pos setting = (Pos){static_cast<uint8_t>(ope.data[0] + dx[i]), static_cast<uint8_t>(ope.data[1] + dy[i])};
        this->getEnt(setting.y, setting.x) = i == 3 ? buf : this->getEnt(ope.data[1] + dy[i + 1], ope.data[0] + dx[i + 1]);
        this->getEntPos(this->getEnt(setting.y, setting.x).val, this->getEnt(setting.y, setting.x).num) = setting;
      }
    }
  }
  //奇数の時の真ん中を動かす
  if(b == 1){
    int mw = ope.data[0] + a, mh = ope.data[1] + a;
    for(int i = 0; i < a; i++) {
      w1 = ope.data[0] + i;
      w2 = ope.data[0] + ope.data[2] - i - 1;
      h1 = ope.data[1] + i;
      h2 = ope.data[1] + ope.data[2] - i - 1;
      buf = this->getEnt(h1, mw);

      int dy[4] = {h1, mh, h2, mh};
      int dx[4] = {mw, w1, mw, w2};
      for (int j = 0; j < 4; j++) {
        Pos setting = (Pos){static_cast<uint8_t>(dx[j]), static_cast<uint8_t>(dy[j])};
        this->getEnt(setting.y, setting.x) = j == 3 ? buf : this->getEnt(dy[j + 1], dx[j + 1]);
        this->getEntPos(this->getEnt(setting.y, setting.x).val, this->getEnt(setting.y, setting.x).num) = setting;
      }
    }
  }
}

void Field::toSmall(int x, int y, int next_size) {
  if ((next_size & 1) || x + next_size > this->size || y + next_size > this->size) {
    std::cout << "Logic Error\nnext_size is invalid" << std::endl;
    exit(1);
  }
  int p_size = this->size;
  Ent *tmp_ptr = this->ent_mem + y * p_size + x;
  this->size = next_size;
  for (int i = 0; i < next_size; i++) {
    memmove(this->ent_mem + next_size * i, tmp_ptr + p_size * i, next_size * sizeof(Ent));
  }
  this->reallocation();
}

void Field::reallocation() {
  int cnt = 0;
  int max_cnt = this->size * this->size / 2;
  int new_val[288];
  for (int i = 0; i < 288; i++) new_val[i] = -1;
  for (int i = 0; i < this->size; i++) for (int j = 0; j < this->size; j++) {
    Ent &tmp = this->getEnt(i, j);
    if (new_val[tmp.val] == -1) {
      if (cnt == max_cnt) {
        std::cout << "Logic Error\ncnt is too big." << std::endl;
        exit(1);
      }
      new_val[tmp.val] = cnt;
      cnt++;
    }
    tmp.val = new_val[tmp.val];
    this->getEntPos(tmp.val, tmp.num) = Pos(j, i);
  }
}

void Field::printField() {
  for (int i = 0; i < this->size; i++) for (int j = 0; j < this->size; j++) std::cout << this->getEnt(i, j).val << " \n"[j == this->size - 1];
  std::cout << std::endl;
}

void Field::printEntPos() {
  for (int i = 0; i < this->size * this->size / 2; i++) printf("%d: (%d, %d), (%d, %d)\n", i, this->getEntPos(i, 0).y, this->getEntPos(i, 0).x, this->getEntPos(i, 1).y, this->getEntPos(i, 1).x);
  std::cout << std::endl;
}

Pos::Pos() {
}

Pos::Pos(short x, short y) : x(x), y(y) {
}

Pos Pos::operator + (const Pos &other) const {
  return Pos(this->x + other.x, this->y + other.y);
}

bool Pos::operator == (const Pos &other) const {
  return this->x == other.x && this->y == other.y;
}

int Pos::toInt(int size) {
  return this->x + this->y * size;
}

Ent::Ent() {
}

Ent::Ent(short val, short num) : val(val), num(num) {
}

BeamNode::BeamNode() {
}

BeamNode::BeamNode(State *p, int idx) : p(p), idx(idx) {
}

// 小さいほうが悪い(セグ木で優先的に消える)ものとする (スコアは大きいほうが良い)
bool BeamNode::operator < (const BeamNode &other) const {
    if (this->idx == -1) {
        return false;
    } else if (other.idx == -1) {
        return true;
    }

    if (this->p == NULL) {
        return true;
    } else if (other.p == NULL) {
        return false;
    }
    return this->p->getScore() < other.p->getScore();
}

std::vector<Ope> MemObj1::getBfsResultOperation(State *s, std::vector<Pos> &target_pos, int type, int ent, int idx) {
  std::vector<Ope> res;
  int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
  int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
  if (p2 < p1) std::swap(p1, p2);
  uint16_t parent;
  int pair_cnt = s->progress + (s->edge_cnt / 2) * (s->f.size - 2) + (s->edge_cnt & 1) * s->f.size / 2;
  while ((parent = this->getParent4(s->f.size, pair_cnt, type, p1, p2, idx)) != ROOT_NODE) {
    if (parent == BAD_NODE) {
      std::cout << "typeD: Bad parent" << std::endl;
      exit(1);
    }
    Ope tmp_ope = this->all_ope[parent & 0x1FFF];
    res.push_back(tmp_ope);
    p1 = getRotatePos(intToPos(p1, s->f.size), tmp_ope).toInt(s->f.size);
    p2 = getRotatePos(intToPos(p2, s->f.size), tmp_ope).toInt(s->f.size);
    if (p2 < p1) std::swap(p1, p2);
    idx = parent >> 13;
    if (res.size() >= 500) break;
  }
  return res;
}

int MemObj1::getBfsResultOperationCount(State *s, std::vector<Pos> &target_pos, int ent, int type) {
  int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
  int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
  if (p2 < p1) std::swap(p1, p2);
  int pair_cnt = s->progress + (s->edge_cnt / 2) * (s->f.size - 2) + (s->edge_cnt & 1) * s->f.size / 2;
  size_t idx = this->getIndex4(s->f.size, pair_cnt, type, p1, p2, 0);
  if (this->bfs_result4[s->f.size / 2 - 2][idx] == NODATA_NODE) {
    std::cout << "Error: NODATA_NODE" << std::endl;
    exit(1);
  }
  for (int i = 0; i < RESULT1_ENUM_CNT; i++) {
    if (this->bfs_result4[s->f.size / 2 - 2][idx++] == BAD_NODE) return i;
  }
  return RESULT1_ENUM_CNT;
}

size_t MemObj1::getIndex4(int size, int t, int type, int p1, int p2, int idx) {
  const int idx_cnt = 4;
  int idxes[idx_cnt] = {idx, p2 * (p2 - 1) / 2 + p1, type, t};
  int max_cnt[idx_cnt] = {RESULT1_ENUM_CNT, (size * size) * (size * size - 1) / 2, TYPE_CNT4, (size * 2) - 2};
  size_t result_idx = 0;
  size_t hosei = 1;
  for (int i = 0; i < idx_cnt; i++) {
    result_idx += hosei * idxes[i];
    hosei *= max_cnt[i];
  }
  return result_idx;
}

uint16_t MemObj1::getParent4(int size, int t, int type, int p1, int p2, int idx) {
  size_t result_idx = this->getIndex4(size, t, type, p1, p2, idx);
  return this->bfs_result4[size / 2 - 2][result_idx];
}

Ope algo1::rotateOpe(Ope ope, int fsize, int r) {
  if (r == 1){
    int px = ope.data[0];
    int py = ope.data[1];
    ope.data[0] = py;
    ope.data[1] = fsize - px - ope.data[2];
  } else if (r == 2) {
    ope.data[0] = fsize - ope.data[0] - ope.data[2];
    ope.data[1] = fsize - ope.data[1] - ope.data[2];
  } else if (r == 3) {
    int px = ope.data[0];
    int py = ope.data[1];
    ope.data[0] = fsize - py - ope.data[2];
    ope.data[1] = px;
  }
  return ope;
}

Pos algo1::getRotatePos(Pos p, Ope ope) {
  if (!(p.x >= ope.data[0] && p.x < ope.data[0] + ope.data[2] && p.y >= ope.data[1] && p.y < ope.data[1] + ope.data[2])) return p;
  Pos tmp = Pos(p.x - ope.data[0], p.y - ope.data[1]);
  Pos tmp_cp = tmp;
  tmp.x = ope.data[2] - tmp_cp.y - 1;
  tmp.y = tmp_cp.x;
  p.x = ope.data[0] + tmp.x;
  p.y = ope.data[1] + tmp.y;
  return p;
}

Pos algo1::intToPos(int p, int size) {
  return Pos(p % size, p / size);
}

void algo1::clearSeg(Seg<BeamNode> *seg) {
  for (int i = 0; i < seg->size; i++) seg->set(i, BeamNode(NULL, i));
}

BeamNode algo1::seg_ope(BeamNode x, BeamNode y) {
  return x < y ? x : y;
}