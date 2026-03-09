#include <algo1.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#include <cstring>
#include <iostream>
#include <map>
#include <unordered_map>

using namespace algo1;
using namespace fdb;

// スタブ関数
MemObj1 init1() { 
  MemObj1 mem;
  mem.size = 0;

  /* bfs結果関連のメモリ確保(ファイル読み込み含む) */
  char *base_file_name1 = (char*)"../data/bfs_result8/bfs_result8_n%d_m5.bin";
  char *base_file_name2 = (char*)"../data/bfs_result10/bfs_result10_n%d_m5.bin";
  char *base_file_name3 = (char*)"../data/bfs_result11/bfs_result11_n%d_m5.bin";
  char *base_file_name4 = (char*)"../data/bfs_result12/bfs_result12_n%d_m5.bin";
  FILE *fp;
  char file_name[256];

  std::cout << "Start memory allocation (1)." << std::endl;
  mem.size += 11 * sizeof(uint16_t*);
  mem.bfs_result1 = new uint16_t*[11];
  for (size_t size = 4; size <= MAX_FIELD_SIZE; size += 2) {
    size_t len = (size * 2 - 2) * TYPE_CNT1 * ((size * size) * (size * size - 1) / 2) * RESULT1_ENUM_CNT;
    mem.size += len * sizeof(uint16_t);
    mem.bfs_result1[size / 2 - 2] = new uint16_t[len];
    sprintf(file_name, base_file_name2, size);
    fp = fopen(file_name, "rb");
    if (fp == NULL) {
      printf("Cannot open \"%s\".\n", file_name);
      std::cout << std::flush;
      exit(1);
    }
    size_t read_mem = fread(&mem.bfs_result1[size / 2 - 2][0], 1, sizeof(uint16_t) * len, fp);
    if (read_mem != len * sizeof(uint16_t)) {
      std::cout << "File read error" << std::endl;
      std::cout << read_mem << " " << len * sizeof(uint16_t) << std::endl;
      exit(1);
    }
    fclose(fp);
    printf("size: %ld, len: %ld, use_memory: %f [GB]\n", size, len, (double)(len * sizeof(uint16_t)) / (1 << 30));
    std::cout << std::flush;
  }
  std::cout << "Finish memory allocation (1)." << std::endl;

  int tmp_N;
  int T;
  int M = 5;
  mem.size += 11 * sizeof(uint16_t*);
  mem.bfs_result2 = new uint16_t*[11];
  for (int size = 4; size <= std::min(22, MAX_FIELD_SIZE); size += 2) {
    sprintf(file_name, base_file_name1, size);
    printf("Loading \"%s\".\n", file_name);
    
    fp = fopen(file_name, "rb");
    if (fp == NULL) {
      std::cout << "File Open Error\nCannot ope bfs_result file." << std::endl;
      exit(1);
    }

    size_t read_mem = fread(&tmp_N, sizeof(char), 1, fp);
    read_mem = fread(&T, sizeof(int), 1, fp);
    int64_t len = (int64_t)T * TYPE_CNT2 * size * size * size * size * M;
    std::cout << len << std::endl;
    mem.size += len * sizeof(uint16_t);
    mem.bfs_result2[size / 2 - 1] = new uint16_t[len];
    std::cout << fread(&mem.bfs_result2[size / 2 - 1][0], sizeof(uint16_t), len, fp) << std::endl;
    fclose(fp);
  }

  auto isOKType3 = [](int size, int num, int type, int x) {
    int progress, dir;
    if (num >= size) {
      progress = num - size;
      dir = 1;
    } else {
      progress = num;
      dir = 0;
    }

    if (progress == size - 1 - 2 * dir && (type == 1 || type == 3)) return false;
    int y = progress - x;
    if (type == 2 || type == 4) y--;

    int w = size - 2 * dir;
    int max_y = size - 1;
    if (type >= 1) max_y--;
    return x >= 0 && x < w - 1 && y >= 2 && y <= max_y;
  };

  size_t mem_sum3 = 0;
  mem.size += 11 * sizeof(uint16_t*);
  mem.bfs_result3 = new uint16_t*[11];
  mem.idx_memo3.resize(11);
  std::cout << "Start memory allocation (3)." << std::endl;
  for (size_t size = 4; size <= MAX_FIELD_SIZE; size += 2) {
      int cnt = 0;
      mem.idx_memo3[size / 2 - 2] = std::vector<std::vector<std::vector<int>>>(size * 2 - 2, std::vector<std::vector<int>>(TYPE_CNT3, std::vector<int>(size - 1, -1)));
      for (int i = 0; i < size * 2 - 2; i++) for (int j = 0; j < TYPE_CNT3; j++) for (int k = 0; k < size - 1; k++) {
        if (isOKType3(size, i, j, k)) mem.idx_memo3[size / 2 - 2][i][j][k] = cnt++;
      }
      // size_t len = (size * 2 - 2) * TYPE_CNT3 * (size - 1) * size * size * size * size * 5;
      size_t len = cnt * (size * size * (size * size - 1) / 2) * RESULT3_ENUM_CNT;
      mem.size += len * sizeof(uint16_t);
      mem.bfs_result3[size / 2 - 2] = new uint16_t[len];
      sprintf(file_name, base_file_name3, size);
      fp = fopen(file_name, "rb");
      if (fp == NULL) {
        printf("Cannot open \"%s\".\n", file_name);
        std::cout << std::flush;
        exit(1);
      }
      size_t read_mem = fread(&mem.bfs_result3[size / 2 - 2][0], 1, sizeof(uint16_t) * len, fp);
      if (read_mem != len * sizeof(uint16_t)) {
        std::cout << "File read error" << std::endl;
        std::cout << read_mem << " " << len * sizeof(uint16_t) << std::endl;
        exit(1);
      }
      fclose(fp);
      mem_sum3 += len * sizeof(uint16_t);
      printf("size: %ld, len: %ld, use_memory: %f [GB]\n", size, len, (double)(len * sizeof(uint16_t)) / (1 << 30));
      std::cout << std::flush;
  }
  printf("bfs_result3 memory_sum: %f [GB]\n", (double)mem_sum3 / (1 << 30));
  std::cout << "Finish memory allocation (3)." << std::endl;

  std::cout << "Start memory allocation (4)." << std::endl;
  mem.size += 11 * sizeof(uint16_t*);
  mem.bfs_result4 = new uint16_t*[11];
  size_t mem_sum4 = 0;
  for (size_t i = 8; i <= MAX_FIELD_SIZE; i += 4) {
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
      std::cout << read_mem << " " << len * sizeof(uint16_t) << std::endl;
      exit(1);
    }
    fclose(fp);
    mem_sum4 += len * sizeof(uint16_t);
    printf("size: %ld, len: %ld, use_memory: %f [GB]\n", i, len, (double)(len * sizeof(uint16_t)) / (1 << 30));
    std::cout << std::flush;
  }
  printf("bfs_result4 memory_sum: %f [GB]\n", (double)mem_sum4 / (1 << 30));
  std::cout << "Finish memory allocation (4)." << std::endl;

  mem.all_ope.reserve(23 * (23 + 1) * (2 * 23 + 1) / 6 - 1);
  for (int n = 2; n <= 24; n++) {
    for (int x = 0; x <= 24 - n; x++) {
      for (int y = 0; y <= 24 - n; y++) {
        mem.all_ope.push_back(Ope(x, y, n));
      }
    }
  }
  std::sort(mem.all_ope.begin(), mem.all_ope.end());


  /* 4x4のデータベース読み込み */
  // field4_init("../data/field4_db");
  field4_init("../data/field4_db");

  /* ビームサーチ関連のメモリ確保 */
  for (int i = 0; i < 2; i++) {
    size_t state_cnt = (std::max(BEAM_WIDTH1, BEAM_WIDTH2) + 1) * CPU_CNT;
    mem.size += state_cnt * sizeof(State);
    mem.tmp_state_mem[i] = new State[state_cnt]{};
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Ent);
    mem.tmp_ent_mem[i] = new Ent[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE]{};
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Pos);
    mem.tmp_pos_mem[i] = new Pos[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE]{};
    for (size_t j = 0; j < state_cnt; j++) {
      mem.tmp_state_mem[i][j].f.ent_mem = mem.tmp_ent_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
      mem.tmp_state_mem[i][j].f.pos_mem = mem.tmp_pos_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
    }
  }
  for (int i = 0; i < 2; i++) {
    size_t state_cnt = std::min(BEAM_WIDTH1, BEAM_WIDTH2);
    mem.com_state_mem[i] = new State[state_cnt];
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Ent);
    mem.com_ent_mem[i] = new Ent[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE];
    mem.size += state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE * sizeof(Pos);
    mem.com_pos_mem[i] = new Pos[state_cnt * MAX_FIELD_SIZE * MAX_FIELD_SIZE];
    for (size_t j = 0; j < state_cnt; j++) {
      mem.com_state_mem[i][j].f.ent_mem = mem.com_ent_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
      mem.com_state_mem[i][j].f.pos_mem = mem.com_pos_mem[i] + j * MAX_FIELD_SIZE * MAX_FIELD_SIZE;
    }
  }

  mem.size += 2 * std::max(BEAM_WIDTH1, BEAM_WIDTH2) * CPU_CNT * sizeof(BeamNode);
  mem.now_beam = new BeamNode[std::max(BEAM_WIDTH1, BEAM_WIDTH2) * CPU_CNT];
  mem.tmp_beam = new BeamNode[std::max(BEAM_WIDTH1, BEAM_WIDTH2) * CPU_CNT];
  return mem;
}

std::vector<Ope> algorithm1(RawField field, uint32_t fsize, MemObj1& mem1, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields, std::vector<std::pair<uint8_t, uint8_t>> &offsets) {
  State first = State(field, fsize);
  first.getClone(&mem1.com_state_mem[1][0]);

  int second_state_idx = 0;  

  Seg<BeamNode> seg[2][CPU_CNT];
  for (int i = 0; i < CPU_CNT; i++) {
    seg[0][i] = Seg<BeamNode>(BEAM_WIDTH1 + 1, BeamNode(NULL, -1), seg_ope);
    seg[1][i] = Seg<BeamNode>(BEAM_WIDTH2 + 1, BeamNode(NULL, -1), seg_ope);
  }
  Seg<BeamNode> seg2 = Seg<BeamNode>(std::min(BEAM_WIDTH1, BEAM_WIDTH2), BeamNode(NULL, -1), seg_ope);

  int beam_cnt = 1;
  int best_final_score = INT_MIN;
  std::vector<AnsLog> best_log;
  for (int I = 0; I < 12; I++) {
    if (beam_cnt == 0) break;
    int start_size = mem1.com_state_mem[(I + 1) & 1][0].f.size;
    clearSeg(&seg2);
    for (int J = 0; J < 2; J++) {
      if (J == 1 && start_size <= 12) continue;
      int beam_cnt2 = beam_cnt;
      int tmp_width = (J == 0) ? BEAM_WIDTH1 : BEAM_WIDTH2;
      for (int i = 0; i < beam_cnt; i++) {
        mem1.com_state_mem[(I + 1) & 1][i].getClone(&mem1.tmp_state_mem[1][i]);
        mem1.tmp_state_mem[1][i].solve_type = J;
        mem1.now_beam[i] = BeamNode(&mem1.tmp_state_mem[1][i], i);
      }

      for (int i = 0; i < 500; i++) {
        if (beam_cnt2 == 0) break;
        State *best_state = mem1.now_beam[0].p;
        int par_cpu = beam_cnt2 / CPU_CNT;

        #pragma omp parallel for
        for (int j = 0; j < CPU_CNT; j++) {
          clearSeg(&seg[J][j]);
          int beam_start_idx;
          int tmp_beam_cnt;
          if (j < beam_cnt2 % CPU_CNT) {
            tmp_beam_cnt = par_cpu + 1;
            beam_start_idx = (par_cpu + 1) * j;
          } else {
            tmp_beam_cnt = par_cpu;
            beam_start_idx = (par_cpu + 1) * (beam_cnt2 % CPU_CNT) + par_cpu * (j - (beam_cnt2 % CPU_CNT));
          }
          int mem_start_idx = j * (tmp_width + 1);
          int tmp_beam_start_idx = j * tmp_width;
          for (int k = 0; k < tmp_beam_cnt; k++) {
            BeamNode tmp = mem1.now_beam[beam_start_idx + k];
            int type_cnt = J == 0 ? (TYPE_CNT1 + TYPE_CNT2 + TYPE_CNT3 + 1) : TYPE_CNT4; 
            for (int type = 0; type < type_cnt; type++) {
              if (!tmp.p->isOKType(type)) continue;
              int ent_cnt;
              if (J == 0) {
                ent_cnt = (tmp.p->progress != 0 && (type <= 2 || (type >= TYPE_CNT1 + TYPE_CNT2 && type < TYPE_CNT1 + TYPE_CNT2 + TYPE_CNT3))) ? (tmp.p->f.size * tmp.p->f.size / 2) : 1;
              } else {
                ent_cnt = (tmp.p->edge_cnt == -1) ? 1 : (tmp.p->f.size * tmp.p->f.size / 2);
              }
              for (int l = 0; l < ent_cnt; l++) {
                int next_cnt = tmp.p->getNextCount(type, l, mem1);
                for (int m = 0; m < next_cnt; m++) {
                  BeamNode worst = seg[J][j].top();
                  State *next_state = mem1.tmp_state_mem[i % 2] + mem_start_idx + worst.idx;
                  next_state->setScore(tmp.p, type, l, m, mem1);
                  seg[J][j].set(worst.idx, BeamNode(next_state, worst.idx));
                }
              }
            }
            // debug++;
            // if (debug % 100 == 0) cout << "." << flush;
          }
          tmp_beam_cnt = 0;
          for (int k = 0; k < tmp_width + 1; k++) {
            if (seg[J][j].get(k).p == NULL || k == seg[J][j].top().idx) continue;
            mem1.tmp_beam[tmp_beam_start_idx + tmp_beam_cnt++] = seg[J][j].get(k);        
          }
          // cout << "cnt:" << tmp_beam_cnt << endl;
          for (int k = tmp_beam_cnt; k < tmp_width; k++) mem1.tmp_beam[tmp_beam_start_idx + k].p = NULL;
        }
        memcpy(mem1.now_beam, mem1.tmp_beam, sizeof(BeamNode) * tmp_width * CPU_CNT);
        std::sort(mem1.now_beam, mem1.now_beam + (tmp_width * CPU_CNT), [&](BeamNode x, BeamNode y) { 
          if (x.p == NULL) {
            return false;
          } else if (y.p == NULL) {
            return true;
          }
          return x.p->getScore() > y.p->getScore(); 
        });

        int idx = 0;
        beam_cnt2 = 0;
        while (idx < tmp_width * CPU_CNT && mem1.now_beam[idx].p != NULL && beam_cnt2 < tmp_width) {
          mem1.now_beam[idx].p->prev->getClone(mem1.now_beam[idx].p);
          mem1.now_beam[idx].p->moveNextState(mem1.now_beam[idx].p->last_ope.type, mem1.now_beam[idx].p->last_ope.ent, mem1.now_beam[idx].p->last_ope.idx, mem1);

          if (mem1.now_beam[idx].p->isEnd()) {
            if (mem1.now_beam[idx].p->getScore() > best_final_score) {
              best_final_score = mem1.now_beam[idx].p->getScore();
              best_log = mem1.now_beam[idx].p->log;
            }
            idx++;
            continue;
          } else if (mem1.now_beam[idx].p->f.size == start_size - 4) {
            BeamNode worst = seg2.top();
            if (worst.p == NULL || mem1.now_beam[idx].p->getScore() > worst.p->getScore()) {
              State *tmp_p = &mem1.com_state_mem[I % 2][worst.idx];
              mem1.now_beam[idx].p->getClone(tmp_p);
              seg2.set(worst.idx, BeamNode(tmp_p, worst.idx));
            }

            if ((mem1.now_beam[idx].p->f.size == 10 || mem1.now_beam[idx].p->f.size == 12) && second_state_idx < MAX_ENUM_COUNT) {
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

          if (beam_cnt2 == 0 || !(mem1.now_beam[beam_cnt2 - 1].p->f == mem1.now_beam[idx].p->f)) {
            mem1.now_beam[beam_cnt2++] = mem1.now_beam[idx];
          }
          idx++;
        }
        if (beam_cnt2 == 0) continue;
        best_state = mem1.now_beam[0].p;
        printf("(%d, %d, %d, %d, %d, %d)\n", i, beam_cnt2, idx, best_state->score, best_state->ope_sum, best_state->ok_pair);
        // rep (j, best_state->log.size()) {
        //     printf("(%d, %d, %d, %d)\n", j, best_state->log[j].type, best_state->log[j].ent, best_state->log[j].idx);
        // }
        // cout << endl;
        // best_state->f.printField();
        std::cout << std::flush;
      }
    }

    beam_cnt = 0;
    for (int i = 0; i < std::min(BEAM_WIDTH1, BEAM_WIDTH2); i++) {
      beam_cnt += seg2.get(i).p != NULL;
    }

    // cout << best_state->progress << " " << best_state->last_type << endl;
    // rep (j, best_state->log.size()) {
    //     printf("(%d, %d), ", best_state->log[j].first, best_state->log[j].second);
    // }
    // cout << endl;
    // best_state->f.printField();
  }

  std::vector<Ope> res;
  RawField tmp_field;
  std::pair<uint8_t, uint8_t> tmp_offset;
  first.getAnswer(best_log, tmp_field, res, tmp_offset, mem1);
  return res;
}

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), edge_cnt(-1), progress(0), log(std::vector<AnsLog>(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0), pile_dir(HORIZON) {
}

State::State(RawField &field, int fsize) : x_hosei(0), y_hosei(0), rotate_hosei(0), edge_cnt(-1), progress(0), log(std::vector<AnsLog>(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0), pile_dir(HORIZON){
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
  if (this->end_flag) return true;
  for (int i = 0; i < this->f.size * this->f.size / 2; i++) {
    if (manhattan(this->f.getEntPos(i, 0), this->f.getEntPos(i, 1)) != 1) return false;
  }
  return true;
}

int State::getScore() {
  return this->score;
}

void State::setScore(State *prev, int type, int ent, int idx, MemObj1 &mem1) {
  this->prev = prev;
  // this->last_ope = AnsLog(type, ent, idx, this->solve_type);
  this->last_ope = AnsLog(type, ent, idx, prev->solve_type);

  if (prev->solve_type == TYPE_A) {
    // 全体回転の場合
    if (prev->progress == 0) {
      this->score = prev->score;
      return;
    }
    
    std::vector<Ope> opes = prev->getOperation(type, ent, idx, mem1);
    int new_pair_cnt;
    if (type <= 7) {
      new_pair_cnt = type <= 2 ? 1 : 2;
    } else if (type <= 12) {
      new_pair_cnt = 1;
    } else if (type == 13) {
      new_pair_cnt = 0;
    } else if (type == 14) {
      new_pair_cnt = 2;
    }
    this->ok_pair = prev->ok_pair + new_pair_cnt;
    this->ope_sum = prev->ope_sum + (int)opes.size();
    this->score = -10000 * ope_sum / ok_pair;
  } else {
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
}

int State::getNextCount(int type, int ent, MemObj1 &mem1) {
  if (!isOKType(type)) return 0;
  if (this->solve_type == TYPE_A && type == 13) return 1;
  if ((this->solve_type == TYPE_A && this->progress == 0) || (this->solve_type == TYPE_B && this->edge_cnt == -1)) return 4 * (type == 0);
  // if (this->edge_cnt == -1) return 4 * (type == 0);

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
  if (this->solve_type == TYPE_A) {
    if (this->progress == 0) {
      std::vector<Ope> opes = this->getOperation(type, ent, idx, mem1);
      for (Ope &ope : opes) this->f.rotate(ope);
      this->rotate_hosei += opes.size();
      this->rotate_hosei &= 3;
      this->progress++;
      this->last_type = FLAT;
      this->pile_dir = HORIZON;
      this->last_pair_x = -1;
    } else {
      std::vector<Ope> opes = this->getOperation(type, ent, idx, mem1);
      for (Ope &ope : opes) this->f.rotate(ope);
      int new_pair_cnt;
      if (type <= 7) {
        new_pair_cnt = type <= 2 ? 1 : 2;
        this->progress += new_pair_cnt;
        this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);  // 要検討
        this->last_type = (type == 1 || type == 5 || type == 7) ? OUTSIDE : FLAT;
        this->pile_dir = HORIZON;
      } else if (type <= 12) {
        new_pair_cnt = 1;
        this->progress += new_pair_cnt;
        int tmp_type = type - 8;
        if (tmp_type == 1) {
          this->last_type = LEFT;
        } else if (tmp_type == 3) {
          this->last_type = RIGHT;
        } else {
          this->last_type = FLAT;
        }
        this->pile_dir = VERTICAL;
      } else if (type == 13) {
        new_pair_cnt = 0;
        this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);
        this->last_type = FLAT;
        this->pile_dir = HORIZON;
      } else if (type == 14) {
        new_pair_cnt = 2;
        this->progress += new_pair_cnt;
        this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);
        this->last_type = FLAT;
        this->pile_dir = HORIZON;
      }
      this->ok_pair += new_pair_cnt;
      this->ope_sum += (int)opes.size();
      this->score = -10000 * ope_sum / ok_pair;

      if (this->pile_dir == HORIZON && this->progress == this->f.size * 2 - 1) {
        if (this->f.size == 6) {
          this->end_flag = true;
          this->f.toSmall(0, 2, this->f.size - 2);
          this->x_hosei += 2 * (this->rotate_hosei == 1 || this->rotate_hosei == 2);
          this->y_hosei += 2 * (this->rotate_hosei <= 1);

          std::vector<Ope> final_ope = fdb::getField4(this->f);
          // cout << final_ope.size() << " " << flush;
          this->ope_sum += (int)final_ope.size();
          this->ok_pair += 8;
          this->score = -10000 * this->ope_sum / this->ok_pair;
        } else {
          this->f.toSmall(0, 2, this->f.size - 2);
          this->progress = 0;
          this->x_hosei += 2 * (this->rotate_hosei == 1 || this->rotate_hosei == 2);
          this->y_hosei += 2 * (this->rotate_hosei <= 1);
        }
      }
    }
  } else {
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
  }
  this->log.push_back(AnsLog(type, ent, idx, this->solve_type));
}

void State::getClone(State *out) {
  out->x_hosei = this->x_hosei;
  out->y_hosei = this->y_hosei;
  out->rotate_hosei = this->rotate_hosei;
  out->solve_type = this->solve_type;
  this->f.getClone(&out->f);
  out->edge_cnt = this->edge_cnt;
  out->progress = this->progress;
  out->score = this->getScore();
  out->end_flag = this->end_flag;
  out->log = this->log;
  out->last_type = this->last_type;
  out->ok_pair = this->ok_pair;
  out->ope_sum = this->ope_sum;
  out->pile_dir = this->pile_dir;
  out->last_pair_x = this->last_pair_x;
}

void State::getAnswer(std::vector<AnsLog> &ans_log, RawField &raw_field, std::vector<Ope> &out, std::pair<uint8_t, uint8_t> &offset, MemObj1 &mem1) {
  Ent tmp_ent_mem[this->f.size * this->f.size];
  Pos tmp_pos_mem[this->f.size * this->f.size];
  State tmp_s;
  tmp_s.f.ent_mem = tmp_ent_mem;
  tmp_s.f.pos_mem = tmp_pos_mem;

  int ope_cnt = 0;
  this->getClone(&tmp_s);

  // for (int i = 0; i < (int)ans_log.size(); i++) std::cout << i << " " << ans_log[i].type << " " << ans_log[i].ent << " " << ans_log[i].idx << " " << ans_log[i].solve_type << std::endl;
  for (int i = 0; i < (int)ans_log.size(); i++) {
    // std::cout << i << " " << ans_log[i].type << " " << ans_log[i].ent << " " << ans_log[i].idx << " " << ans_log[i].solve_type << std::endl;
    // tmp_s.f.printField();
    tmp_s.solve_type = ans_log[i].solve_type;
    if ((tmp_s.solve_type == TYPE_A && tmp_s.progress == 0) || (tmp_s.solve_type == TYPE_B && tmp_s.edge_cnt == -1)) {
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
    if (tmp_s.f.size == 4) {
      std::vector<Ope> final_ope = fdb::getField4(tmp_s.f);
      for (Ope &ope : final_ope) {
        Ope tmp = rotateOpe(ope, tmp_s.f.size, tmp_s.rotate_hosei);
        tmp.data[0] += tmp_s.x_hosei;
        tmp.data[1] += tmp_s.y_hosei;
        out.push_back(tmp);
      }
    }
  }
  for (int i = 0; i < (4 - tmp_s.rotate_hosei) % 4; i++) tmp_s.f.rotate(Ope(0, 0, tmp_s.f.size));
  raw_field.resize(tmp_s.f.size * tmp_s.f.size);
  for (int i = 0; i < tmp_s.f.size * tmp_s.f.size; i++) raw_field[i] = tmp_s.f.ent_mem[i].val;
  offset = std::make_pair(tmp_s.x_hosei, tmp_s.y_hosei);
}

std::vector<Ope> State::getOperation(int type, int ent, int idx, MemObj1 &mem1) {
  std::vector<Ope> res;
  if (this->solve_type == TYPE_A) {
    if (this->progress == 0) {
      for (int i = 0; i < idx; i++) res.push_back(Ope(0, 0, this->f.size));
    } else {
      if (type == 13) {
        return this->getToHorizonOpe();
      } else if (type == 14) {
        // return TT::getOperation(this, idx);
      } else {
        std::vector<Pos> base_pos = this->getBasePos(type);
        std::vector<Pos> target_pos(base_pos.size());
        for (int i = 0; i < (int)base_pos.size(); i++) target_pos[i] = this->f.getPairPos(base_pos[i]);

        std::vector<Ope> tmp = mem1.getBfsResultOperation(this, target_pos, type, ent, idx);
        for (Ope &ope : tmp) {
          res.push_back(ope);
        }
      }
    }
  } else {
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
  }
  return res;
}

bool State::isOKType(int type) { 
  if (this->solve_type == TYPE_A) {
    if (this->progress == this->f.size * 2 - 1 && type != 13) return false;

    int tmp_progress, dir;
    if (this->progress >= this->f.size + 1) {
      tmp_progress = this->progress - (this->f.size + 1);
      dir = 1;
    } else {
      tmp_progress = this->progress - 1;
      dir = 0;
    }

    if (type <= 2) {
      if (this->pile_dir == VERTICAL) return false;
      if (dir == 0) {
        if ((tmp_progress == this->f.size - 3 && type == 1) || (tmp_progress == this->f.size - 2 && type != 1) || (tmp_progress == this->f.size - 1 && type != 2)) return false;
      } else {
        if ((tmp_progress == this->f.size - 5 && type == 1) || (tmp_progress == this->f.size - 4 && type != 1) || (tmp_progress == this->f.size - 3 && type != 2)) return false;
      }

      // if (this->last_type == FLAT && type == 2 || this->last_type == OUTSIDE && type <= 1) return false;
      if (type == 0) {
        return this->last_type == FLAT;
      } else if (type == 1) {
        return this->last_type == FLAT;
      } else {
        return this->last_type == OUTSIDE;
      }
    } else if (type <= 7) {
      if (this->pile_dir == VERTICAL) return false;
      if (this->f.size == 24) return false;
      type -= 3;
      if (dir == 0) {
        if (tmp_progress == this->f.size - 1 || (tmp_progress == this->f.size - 2 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 3) || (tmp_progress == this->f.size - 4 && (type == 2 || type == 4))) return false;
      } else {
        if (tmp_progress == this->f.size - 3 || (tmp_progress == this->f.size - 4 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 5) || (tmp_progress == this->f.size - 6 && (type == 2 || type == 4))) return false;
      }

      if (type == 0) {
        return this->last_type == FLAT;
      } else if (type == 1) {
        return this->last_type == FLAT;
      } else if (type == 2) {
        return this->last_type == FLAT;
      } else if (type == 3) {
        return this->last_type == OUTSIDE;
      } else {
        return this->last_type == OUTSIDE;
      }
    } else if (type <= 12) {
      type -= 8;
      if (tmp_progress == this->f.size - 1 - 2 * dir && (type == 1 || type == 3)) return false;
      int x = this->last_pair_x - 1;
      int y = tmp_progress - x;
      if (type == 2 || type == 4) y--;
      
      int w = this->f.size - 2 * dir;
      int max_y = this->f.size - 1;
      if (type >= 1) max_y--;
      if (!(x >= 0 && x < w - 1 && y >= 2 && y <= max_y)) return false;

      if (type == 0) {
        return this->last_type == FLAT;
      } else if (type == 1) {
        return x != w - 2 && this->last_type == FLAT;
      } else if (type == 2) {
        return this->last_type == RIGHT; 
      } else if (type == 3) {
        return x != 0 && this->last_type == FLAT;
      } else {
        return this->last_type == LEFT;
      }
    } else if (type == 13) {
      return this->pile_dir == VERTICAL && this->last_type == FLAT;
    } else {
      return this->pile_dir == HORIZON && this->last_type == FLAT;
    }
    return true;    
  } else {
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
  if (this->solve_type == TYPE_A) {
    std::vector<Pos> res;
    int tmp_progress, dir;
    if (this->progress >= this->f.size + 1) {
      tmp_progress = this->progress - (this->f.size + 1);
      dir = 1;
    } else {
      tmp_progress = this->progress - 1;
      dir = 0;
    }

    if (type <= 2) {
      if (dir == 0) {
        if (type == 0 || type == 1) {
          return {Pos(tmp_progress, 0)};
        } else {
          return {Pos(tmp_progress - 1, 1)};
        }
      } else {
        if (type == 0 || type == 1) {
          return {Pos(this->f.size - 1, 2 + tmp_progress)};
        } else {
          return {Pos(this->f.size - 2, 2 + tmp_progress - 1)};
        }
      }
    } else if (type <= 7) {
      type -= 3;

      if (type == 0 || type == 2) {
        res = {Pos(tmp_progress, 0), Pos(tmp_progress + 1, 0)};
      } else if (type == 1) {
        res = {Pos(tmp_progress, 0), Pos(tmp_progress, 1)};
      } else if (type == 3 || type == 4) {
        res = {Pos(tmp_progress - 1, 1), Pos(tmp_progress + 1, 0)};
      }

      if (dir == 1) {
        for (int i = 0; i < res.size(); i++) {
          res[i] = Pos(this->f.size - 1 - res[i].y, res[i].x + 2);
        }
      }
    } else {
      type -= 8;
      // int x = type % this->f.size;
      // type /= this->f.size;
      int x = this->last_pair_x - 1;
      int y = tmp_progress - x;
      if (type == 2 || type == 4) y--;
      
      Pos tmp = Pos(x, y);
      if (type == 3 || type == 4) tmp.x++;
      if (dir) {
        tmp = getRotatePos(tmp, Ope(0, 0, this->f.size));
        tmp.y += 2;
      }
      return {tmp};
    }
    return res;
  } else {
    Pos tmp = this->getTmpBasePos(type);
    for (int i = 0; i < this->edge_cnt / 2; i++) {
      tmp = getRotatePos(tmp, Ope(0, 0, this->f.size));
    }
    return {tmp};
  }
}

std::vector<Ope> State::getToHorizonOpe() {
  int tmp_progress, dir;
  if (this->progress > this->f.size + 1) {  // =はいらない？
    tmp_progress = this->progress - (this->f.size + 1);
    dir = 1;
  } else {
    tmp_progress = this->progress - 1;
    dir = 0;
  }

  int size = tmp_progress - this->last_pair_x + 1;
  std::vector<Ope> res;
  if (dir) {
    if (this->f.getEnt(this->last_pair_x, this->f.size - 1).val == this->f.getEnt(this->last_pair_x + 1, this->f.size - 1).val) {
      res.push_back(Ope(this->f.size - 2, this->last_pair_x, 2));
    }
    res.push_back(Ope(this->f.size - size, this->last_pair_x + 1, size));
  } else {
    if (this->f.getEnt(0, this->last_pair_x - 2).val == this->f.getEnt(0, this->last_pair_x - 1).val) {
      res.push_back(Ope(this->last_pair_x - 2, 0, 2));
    }
    res.push_back(Ope(this->last_pair_x - 1, 0, size));
  }
  return res;
}

AnsLog::AnsLog() {
}

AnsLog::AnsLog(int type, int ent, int idx, int solve_type) : type(type), ent(ent), idx(idx), solve_type(solve_type) {
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
    // printf("<: (%p, %d), (%p, %d) ", this->p, this->idx, other.p, other.idx);
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
  if (s->solve_type == TYPE_A) {
    if (type < TYPE_CNT1) {
      std::vector<Ope> res;
      int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
      int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
      if (p2 < p1) std::swap(p1, p2);
      uint16_t parent;
      while ((parent = this->getParent1(s->f.size, s->progress - 1, type, p1, p2, idx)) != ROOT_NODE) {
        if (parent == BAD_NODE) {
          std::cout << "typeA: Bad parent" << std::endl;
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
    } else if (type < TYPE_CNT1 + TYPE_CNT2) {
      type -= TYPE_CNT1;
      std::vector<Ope> res;
      uint16_t parent;
      std::vector<Pos> now_target = target_pos;
      int now_idx = idx;
      while ((parent = this->getParent2(s->f.size, s->progress - 1, type, now_target[0].toInt(s->f.size), now_target[1].toInt(s->f.size), now_idx)) != 65535) {
        Ope tmp_ope = this->all_ope[parent & 0x1FFF];
        res.push_back(tmp_ope);
        for (int i = 0; i < 2; i++) {
          now_target[i] = getRotatePos(now_target[i], tmp_ope);
        }
        now_idx = parent >> 13;
      }
      return res;
    } else {
      type -= TYPE_CNT1 + TYPE_CNT2;
      std::vector<Ope> res;
      int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
      int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
      if (p2 < p1) std::swap(p1, p2);
      uint16_t parent;
      while ((parent = this->getParent3(s->f.size, s->progress - 1, type, s->last_pair_x - 1, p1, p2, idx)) != ROOT_NODE) {
        if (parent == BAD_NODE) {
          std::cout << "typeC: Bad parent" << std::endl;
          s->f.printField();
          printf("size: %d, progress: %d, type: %d, last_pair_x: %d, ent: %d, p1: (%d, %d), p2: (%d, %d), idx: %d\n", s->f.size, s->progress, type, s->last_pair_x, ent, p1 % s->f.size, p1 / s->f.size, p2 % s->f.size, p2 / s->f.size, idx);
          std::cout << std::flush;
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
  } else {
    std::vector<Ope> res;
    int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
    int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
    if (p2 < p1) std::swap(p1, p2);
    uint16_t parent;
    int pair_cnt = s->progress + (s->edge_cnt / 2) * (s->f.size - 2) + (s->edge_cnt & 1) * s->f.size / 2;
    while ((parent = this->getParent4(s->f.size, pair_cnt, type, p1, p2, idx)) != ROOT_NODE) {
      if (parent == BAD_NODE) {
        std::cout << "typeA: Bad parent" << std::endl;
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
}

int MemObj1::getBfsResultOperationCount(State *s, std::vector<Pos> &target_pos, int ent, int type) {
  if (s->solve_type == TYPE_A) {
    if (type < TYPE_CNT1) {
      int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
      int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
      if (p2 < p1) std::swap(p1, p2);
      size_t idx = this->getIndex1(s->f.size, s->progress - 1, type, p1, p2, 0);
      for (int i = 0; i < RESULT1_ENUM_CNT; i++) {
        if (this->bfs_result1[s->f.size / 2 - 2][idx++] == BAD_NODE) return i;
      }
      return RESULT1_ENUM_CNT;
    } else if (type < TYPE_CNT1 + TYPE_CNT2) {
      return 5;
    } else {
      type -= TYPE_CNT1 + TYPE_CNT2;
      int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
      int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
      if (p2 < p1) std::swap(p1, p2);
      size_t idx = this->getIndex3(s->f.size, s->progress - 1, type, s->last_pair_x - 1, p1, p2, 0);
      for (int i = 0; i < RESULT3_ENUM_CNT; i++) {
        if (this->bfs_result3[s->f.size / 2 - 2][idx++] == BAD_NODE) return i;
      }
      return RESULT3_ENUM_CNT;
    }
  } else {
    int p1 = s->f.getEntPos(ent, 0).toInt(s->f.size);
    int p2 = s->f.getEntPos(ent, 1).toInt(s->f.size);
    if (p2 < p1) std::swap(p1, p2);
    int pair_cnt = s->progress + (s->edge_cnt / 2) * (s->f.size - 2) + (s->edge_cnt & 1) * s->f.size / 2;
    size_t idx = this->getIndex4(s->f.size, pair_cnt, type, p1, p2, 0);
    if (this->bfs_result4[s->f.size / 2 - 2][idx] == NODATA_NODE) {
        std::cout << "Err" << std::endl;
        exit(1);
    }
    for (int i = 0; i < RESULT1_ENUM_CNT; i++) {
      if (this->bfs_result4[s->f.size / 2 - 2][idx++] == BAD_NODE) return i;
    }
    return RESULT1_ENUM_CNT;
  }
}

size_t MemObj1::getIndex1(int size, int t, int type, int p1, int p2, int idx) {
  const int idx_cnt = 4;
  int idxes[idx_cnt] = {idx, p2 * (p2 - 1) / 2 + p1, type, t};
  int max_cnt[idx_cnt] = {RESULT1_ENUM_CNT, (size * size) * (size * size - 1) / 2, TYPE_CNT1, (size * 2) - 2};
  size_t result_idx = 0;
  size_t hosei = 1;
  for (int i = 0; i < idx_cnt; i++) {
    result_idx += hosei * idxes[i];
    hosei *= max_cnt[i];
  }
  return result_idx;
}

size_t MemObj1::getIndex3(int size, int t, int type, int x, int p1, int p2, int idx) {
  if (this->idx_memo3[size / 2 - 2][t][type][x] == -1) {
    std::cout << "getIndex3: -1 error" << std::endl;
    std::cout << size << " " << t << " " << type << " " << x << std::endl;
    exit(1);
  }
  const int idx_cnt = 3;
  int idxes[idx_cnt] = {idx, p2 * (p2 - 1) / 2 + p1, this->idx_memo3[size / 2 - 2][t][type][x]};
  int max_cnt[idx_cnt] = {RESULT3_ENUM_CNT, (size * size * (size * size - 1)) / 2, 1};
  size_t result_idx = 0;
  size_t hosei = 1;
  for (int i = 0; i < idx_cnt; i++) {
    result_idx += hosei * idxes[i];
    hosei *= max_cnt[i];
  }
  return result_idx;
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

uint16_t MemObj1::getParent1(int size, int t, int type, int p1, int p2, int idx) {
    size_t result_idx = this->getIndex1(size, t, type, p1, p2, idx);
    return this->bfs_result1[size / 2 - 2][result_idx];
}

uint16_t MemObj1::getParent2(int size, int t, int type, int target1, int target2, int idx) {
  int idxes[5] = {idx, target2, target1, type, t};
  int max_cnt[5] = {5, size * size, size * size, TYPE_CNT2, 1};
  int64_t result_idx = 0;
  int64_t hosei = 1;
  for (int i = 0; i < 5; i++) {
    result_idx += hosei * idxes[i];
    hosei *= max_cnt[i];
  }
  return this->bfs_result2[size / 2 - 1][result_idx];
}

uint16_t MemObj1::getParent3(int size, int t, int type, int x, int p1, int p2, int idx) {
  size_t result_idx = this->getIndex3(size, t, type, x, p1, p2, idx);
  return this->bfs_result3[size / 2 - 2][result_idx];
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

int algo1::manhattan(Pos p1, Pos p2) {
  return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

void algo1::clearSeg(Seg<BeamNode> *seg) {
  for (int i = 0; i < seg->size; i++) seg->set(i, BeamNode(NULL, i));
}

BeamNode algo1::seg_ope(BeamNode x, BeamNode y) {
  return x < y ? x : y;
}

bool fdb::field4_init(const char *db_path){
  int ret = mdb_env_create(&field4_env);

  // 環境を初期化
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_create failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // データベースのパスに環境を設定
  ret = mdb_env_open(field4_env, db_path, MDB_RDONLY, 0664);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_env_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }
  

  // データベースをオープン
  ret = mdb_txn_begin(field4_env, nullptr, MDB_RDONLY, &field4_txn);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_txn_begin failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  ret = mdb_dbi_open(field4_txn, nullptr, 0, &field4_dbi);
  if (ret != MDB_SUCCESS) {
    std::cerr << "mdb_dbi_open failed: " << mdb_strerror(ret) << std::endl;
    return false;
  }

  // field4のdecodeOperateを事前に配列に保存
  f4decodeOpe[2] = {0, 0, 2};
  f4decodeOpe[3] = {0, 0, 3};
  f4decodeOpe[6] = {0, 1, 2};
  f4decodeOpe[7] = {0, 1, 3};
  f4decodeOpe[10] = {0, 2, 2};
  f4decodeOpe[18] = {1, 0, 2};
  f4decodeOpe[19] = {1, 0, 3};
  f4decodeOpe[22] = {1, 1, 2};
  f4decodeOpe[23] = {1, 1, 3};
  f4decodeOpe[26] = {1, 2, 2};
  f4decodeOpe[34] = {2, 0, 2};
  f4decodeOpe[38] = {2, 1, 2};
  f4decodeOpe[42] = {2, 2, 2};

  return true;
}

void fdb::field4_deinit(){
  if (field4_txn) { mdb_txn_abort(field4_txn); }
  if (field4_dbi) { mdb_dbi_close(field4_env, field4_dbi); }
  if (field4_env) { mdb_env_close(field4_env); }
}

std::vector<Ope> fdb::getField4(Field &f){
  Ent tmp_ent_mem[f.size * f.size];
  Pos tmp_pos_mem[f.size * f.size];
  Field tmp_f = Field(f.size, tmp_ent_mem, tmp_pos_mem);
  f.getClone(&tmp_f);

  std::vector<Ope> ret;
  ret.reserve(8);

  std::vector<std::uint8_t> key;
  MDB_val mdb_key, mdb_data;
  int r;
  
  while(true){
    key = encodeField4(tmp_f);
    mdb_key.mv_size = key.size();
    mdb_key.mv_data = (void*)key.data();

    // データを取得
    r = mdb_get(field4_txn, field4_dbi, &mdb_key, &mdb_data);
    if (r != MDB_SUCCESS) {
      if (r == MDB_NOTFOUND){
        std::cout << "Key not found in the database." << std::endl;
      }else{
        std::cerr << "mdb_get failed: " << mdb_strerror(r) << std::endl;
      }
      ret.clear();
      exit(1);
      return ret;
    }

    if (mdb_data.mv_size == 1) {
      std::uint8_t value = *((std::uint8_t*)mdb_data.mv_data);
      if(value == 0)  break;
      Ope ope = decodeOperate(value);
      tmp_f.rotate(ope);
      ret.push_back(ope);
    }else{
      std::cerr << "Invalid data size: Expected 1 byte." << std::endl;
      ret.clear();
      exit(1);
      return ret;
    }
  }
  return ret;
}

std::vector<std::uint8_t> fdb::encodeField4(Field& f){
  if(f.size != 4){
    std::cerr << "Field size is not 4: " << f.size << std::endl;
    return std::vector<std::uint8_t>();
  }
//   std::unordered_map<int, int> dic = Field::reallocation_map(f);
  f.reallocation();

  auto g = [&](int x, int y){
    //return dic[f.get(x, y)->num];
    return f.getEnt(y, x).val;
  };

  // 最初の12要素のカウント
  std::map<int, int> d;
  std::unordered_map<int, int> d_index;
  for(int i=0; i<8; ++i)  d[i] = 0;
  for(int y = 0; y < 3; ++y) for(int x = 0; x < 4; ++x){ d[g(x,y)] += 1; }

  auto update_d_index = [&](){
    // 出現回数が1回の要素だけを残す
    for (auto it = d.begin(); it != d.end(); ) {
      if (it->second > 1) {
        it = d.erase(it);
      } else {
        ++it;
      }
    }

    // d_index の作成（ソートなし）
    int index = 0;
    d_index.clear();
    for (const auto& kv : d) {
      d_index[kv.first] = index++;
    }
  };

  // バイト列を格納するベクター
  std::vector<std::uint8_t> ret;

  // 最初の8ビットを取得
  std::uint8_t bi = (g(1,0) << 7) | (g(2,0) << 5) | (g(3,0) << 3) | g(0,1);
  ret.push_back(bi);

// 次の8ビットを取得
  bi = (g(1,1) << 5) | (g(2,1) << 2) | (g(3,1) >> 1);
  ret.push_back(bi);

  // 次の8ビットを取得
  bi = ((g(3,1) & 0b001) << 7) | (g(0,2) << 4) | (g(1,2) << 1) | (g(2,2) >> 2);
  ret.push_back(bi);

  // 次の8ビットを取得
  bi = ((g(2,2) & 0b011) << 6) | (g(3,2) << 3);
  update_d_index();
  d[g(0,3)] += 1;
  bi += (d_index[g(0,3)] << 1);
  update_d_index();
  d[g(1,3)] += 1;
  bi += (d_index[g(1,3)] >> 1);
  ret.push_back(bi);

  // 最後のビット
  bi = (d_index[g(1,3)] & 0b01) << 7;
  update_d_index();
  d[g(2,3)] += 1;
  bi += (d_index[g(2,3)] << 6);
  ret.push_back(bi);

#ifdef DEBUG_FIELD_DB
  std::cout << "encodeField4: ";
  for(std::uint8_t a : ret)  std::cout << std::bitset<8>(a) << " ";
  std::cout << std::endl;

  f.print();
#endif

  return ret;
  
}

Ope fdb::decodeOperate(const std::uint8_t& ope){
  Ope ret = Ope(static_cast<short>((ope >> 4) & 3), static_cast<short>((ope >> 2) & 3), static_cast<short>(ope & 3));
#ifdef DEBUG_FIELD_DB
  std::cout << "decodeOperate: " << std::bitset<8>(ope) << " -> ";
  for(auto& r : ret)  std::cout << r << ", ";
  std::cout << std::endl;
#endif
  return ret;
}
