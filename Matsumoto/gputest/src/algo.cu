#include <algo.cuh>
#include <matrix.cuh>
#include <matrixField.cuh>
#include <iostream>
#include <fstream>
#include <string>
#include <cuda_runtime.h>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <unordered_set>

/*
Device name: NVIDIA GeForce RTX 4070 Laptop GPU
Optimization Strategy:
1. GPU: On-the-fly calculation.
2. CPU: Histogram-based Filtering.
*/

const uint32_t tidPerField_list[] = { 0, 0, 0, 0,13, 0, 54, 0, 139, 0, 284, 0, 505, 0, 818, 0, 1239, 0, 1784, 0, 2469, 0, 3310, 0, 4323 };

# define MAX_DEPTH 1024 
# define MAX_FIELD_LEN 576 
# define MAX_SCORE_RANGE 4096 

struct CandidateResult {
    uint32_t score;
    uint32_t parent_idx;
    uint32_t move_id;
    uint64_t hash;
};

// ファイル読み込みヘルパー
static bool loadFieldFromFile(const std::string& filepath, uint32_t& fsize, std::vector<uint16_t>& field) {
    std::ifstream ifs(filepath);
    if (!ifs) {
        // デバッグ用: 失敗したパスを表示
        std::cerr << "Error: Could not open file [" << filepath << "]" << std::endl;
        return false;
    }
    ifs >> fsize;
    if (fsize == 0 || fsize > 24) return false;
    field.resize(fsize * fsize);
    for (size_t i = 0; i < field.size(); ++i) {
        ifs >> field[i];
    }
    return true;
}

// ハッシュ計算
__device__ __host__
uint64_t calculateHashDevice(const uint16_t* field, uint32_t size) {
    uint64_t hash = 14695981039346656037ULL;
    for(uint32_t i=0; i < size; ++i){
        hash ^= field[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

__device__ __host__
void getParams(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N) {
  uint32_t acc = 0;
  uint32_t w = fsize - 1;
  while(true) {
    uint32_t size = w * w;
    if(tid < acc + size){
      break;
    }
    acc += size;
    --w;
  }
  acc = tid - acc;
  *X = acc % w;
  *Y = acc / w;
  *N = fsize - w + 1;
}

__device__ 
uint32_t evaluation1_device(const uint16_t *f, uint32_t fsize) {
  uint32_t val = 0, x, y, i;
  val += (uint32_t)(f[0] == f[1]) + (uint32_t)(f[0] == f[fsize])
        +(uint32_t)(f[fsize - 1] == f[fsize - 2]) + (uint32_t)(f[fsize - 1] == f[2 * fsize - 1])
        +(uint32_t)(f[(fsize - 1) * fsize] == f[(fsize - 2) * fsize]) + (uint32_t)(f[(fsize - 1) * fsize] == f[(fsize - 1) * fsize + 1])
        +(uint32_t)(f[fsize * fsize - 1] == f[fsize * fsize - 2]) + (uint32_t)(f[fsize * fsize - 1] == f[(fsize - 1) * fsize - 1]);
  for(x = 1; x < fsize - 1; ++x) {
    val += (uint32_t)(f[x] == f[x - 1]) + (uint32_t)(f[x] == f[x + 1]) +(uint32_t)(f[x] == f[fsize + x])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 1) * fsize + x - 1])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 1) * fsize + x + 1])
          +(uint32_t)(f[(fsize - 1) * fsize + x] == f[(fsize - 2) * fsize + x]);
  }
  for(y = 1; y < fsize - 1; ++y) {
    val += (uint32_t)(f[y * fsize] == f[(y - 1) * fsize]) + (uint32_t)(f[y * fsize] == f[(y + 1) * fsize]) +(uint32_t)(f[y * fsize] == f[y * fsize] + 1)
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[y * fsize - 1])
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[(y + 2) * fsize - 1])
          +(uint32_t)(f[(y + 1) * fsize - 1] == f[(y + 1) * fsize - 2]);
  }
  for(y = 1; y < fsize - 1; ++y) {
    for(x = 1; x < fsize - 1; ++x) {
      i = y * fsize + x;
      val += (uint32_t)(f[i] == f[i-fsize]) + (uint32_t)(f[i] == f[i+fsize])
            +(uint32_t)(f[i] == f[i-1])     + (uint32_t)(f[i] == f[i+1]);
    }
  }
  return val;
}

__global__
void evaluate_all_kernel(
    const uint32_t fsize,
    const uint32_t tidPerField,
    const uint32_t slotPerField,
    const uint32_t field_size,
    const uint32_t total_tasks,
    const uint16_t *parent_fields,
    CandidateResult *results
) {
    uint32_t global_idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (global_idx >= total_tasks) return;

    uint32_t threads_per_parent = slotPerField * blockDim.x;
    uint32_t parent_idx = global_idx / threads_per_parent;
    uint32_t move_id = global_idx % threads_per_parent;
    
    if (move_id >= tidPerField) {
        results[global_idx].score = 0;
        return;
    }

    const uint16_t *src_field = parent_fields + parent_idx * field_size;
    uint16_t my_field[MAX_FIELD_LEN]; 
    
    uint32_t data[1152];
    for (uint32_t i = 0; i < fsize * fsize * 2; i += 2) {
        data[i] = 0x01000001;
        data[i+1] = 0;
    }

    uint32_t X, Y, N;
    getParams(move_id, fsize, &X, &Y, &N);
    uint32_t rot[2];
    createMatrixArrayL(X, Y, N, rot);

    for(uint32_t y = Y; y < Y + N; ++y) {
        for(uint32_t x = X; x < X + N; ++x) {
            uint32_t i = y * fsize + x;
            multDp4a(data + i * 2, rot);
        }
    }

    uint8_t p[2];
    for(uint32_t y = 0, i; y < fsize; ++y) {
        for(uint32_t x = 0; x < fsize; ++x) {
            p[0] = x;
            p[1] = y;
            i = y * fsize + x;
            culcDp4a(data + i * 2, p);
            uint32_t j = p[1] * fsize + p[0];
            my_field[i] = src_field[j];
        }
    }

    results[global_idx].score = evaluation1_device(my_field, fsize) + 1;
    results[global_idx].hash = calculateHashDevice(my_field, field_size);
    results[global_idx].parent_idx = parent_idx;
    results[global_idx].move_id = move_id;
}

__global__
void gather_next_generation_kernel(
    const uint32_t fsize,
    const uint32_t field_size,
    const uint32_t num_winners,
    const CandidateResult *winners,
    const uint16_t *parent_fields,
    uint16_t *next_fields
) {
    uint32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_winners) return;

    CandidateResult win = winners[idx];
    const uint16_t *src_field = parent_fields + win.parent_idx * field_size;
    uint16_t *dest_field = next_fields + idx * field_size;
    
    uint32_t data[1152];
    for (uint32_t i = 0; i < fsize * fsize * 2; i += 2) {
        data[i] = 0x01000001;
        data[i+1] = 0;
    }

    uint32_t X, Y, N;
    getParams(win.move_id, fsize, &X, &Y, &N);
    uint32_t rot[2];
    createMatrixArrayL(X, Y, N, rot);

    for(uint32_t y = Y; y < Y + N; ++y) {
        for(uint32_t x = X; x < X + N; ++x) {
            uint32_t i = y * fsize + x;
            multDp4a(data + i * 2, rot);
        }
    }

    uint8_t p[2];
    for(uint32_t y = 0, i; y < fsize; ++y) {
        for(uint32_t x = 0; x < fsize; ++x) {
            p[0] = x;
            p[1] = y;
            i = y * fsize + x;
            culcDp4a(data + i * 2, p);
            uint32_t j = p[1] * fsize + p[0];
            dest_field[i] = src_field[j];
        }
    }
}

// ===============================================
//  メインソルバー関数
// ===============================================
SolveResult solve_from_file(const std::string& filepath) {
    SolveResult result;
    // ファイル名だけ取得（表示用）
    result.filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    result.time_seconds = 0.0;
    result.moves = 0;
    result.solved = false;
    result.fsize = 0;

    // --- ファイル読み込み ---
    uint32_t fsize = 0;
    std::vector<uint16_t> start_field;
    
    // パスをそのまま渡す（test.cuから絶対パスが来ているはず）
    if (!loadFieldFromFile(filepath, fsize, start_field)) {
        return result;
    }

    const uint32_t beam_width = 10000; 
    const uint32_t field_size = fsize * fsize;
    const uint32_t threadsPerBlock = 256;

    result.fsize = fsize;
    
    // パラメータチェック
    if (fsize >= sizeof(tidPerField_list)/sizeof(uint32_t)) {
        return result;
    }
    const uint32_t tidPerField = tidPerField_list[fsize];
    const uint32_t slotPerField = (tidPerField + threadsPerBlock - 1) / threadsPerBlock;
    
    // GPUメモリ確保
    uint16_t *d_parent_fields, *d_next_fields;
    cudaMalloc(&d_parent_fields, beam_width * field_size * sizeof(uint16_t));
    cudaMalloc(&d_next_fields, beam_width * field_size * sizeof(uint16_t));
    
    cudaMemcpy(d_parent_fields, start_field.data(), field_size * sizeof(uint16_t), cudaMemcpyHostToDevice);

    size_t max_candidates = (size_t)beam_width * slotPerField * threadsPerBlock;
    CandidateResult *d_candidates;
    cudaMalloc(&d_candidates, max_candidates * sizeof(CandidateResult));
    
    CandidateResult *d_winners;
    cudaMalloc(&d_winners, beam_width * sizeof(CandidateResult));
    
    std::vector<CandidateResult> h_candidates;
    h_candidates.reserve(max_candidates);
    
    std::vector<CandidateResult> shortlist;
    shortlist.reserve(max_candidates); 

    std::vector<uint32_t> operations(beam_width * MAX_DEPTH, 0);
    std::vector<uint32_t> next_operations(beam_width * MAX_DEPTH, 0);

    std::vector<int> score_counts(MAX_SCORE_RANGE);

    uint32_t parent_count = 1;
    uint32_t ope_num = 1;
    auto start_time = std::chrono::high_resolution_clock::now();
    uint32_t best_score = 0;

    auto comp = [](const CandidateResult& a, const CandidateResult& b){
        if (a.score != b.score) return a.score > b.score;
        return a.hash < b.hash;
    };

    // --- 探索ループ ---
    while(best_score < fsize * fsize + 1 && ope_num < MAX_DEPTH) {
        uint32_t total_threads = parent_count * slotPerField * threadsPerBlock;
        uint32_t num_blocks = (total_threads + threadsPerBlock - 1) / threadsPerBlock;

        evaluate_all_kernel<<<num_blocks, threadsPerBlock>>>(
            fsize, tidPerField, slotPerField, field_size, 
            total_threads, d_parent_fields, d_candidates
        );
        
        h_candidates.resize(total_threads);
        cudaMemcpy(h_candidates.data(), d_candidates, total_threads * sizeof(CandidateResult), cudaMemcpyDeviceToHost);
        
        // 1. ヒストグラム法で足切りラインを決定
        std::fill(score_counts.begin(), score_counts.end(), 0);
        int max_s = 0;
        for(const auto& c : h_candidates) {
            uint32_t s = c.score;
            if(s == 0) continue;
            if(s >= MAX_SCORE_RANGE) s = MAX_SCORE_RANGE - 1;
            score_counts[s]++;
            if((int)s > max_s) max_s = (int)s;
        }

        int count_sum = 0;
        int threshold = 0;
        for(int s = max_s; s >= 0; --s) {
            count_sum += score_counts[s];
            if(count_sum >= (int)beam_width * 2) { 
                threshold = s;
                break;
            }
        }
        
        // 2. 候補抽出
        shortlist.clear();
        for(const auto& c : h_candidates) {
            if((int)c.score >= threshold) {
                shortlist.push_back(c);
            }
        }
        
        // 3. ソート (スコア+ハッシュ)
        std::sort(shortlist.begin(), shortlist.end(), comp);

        std::vector<CandidateResult> winners;
        winners.reserve(beam_width);
        std::unordered_set<uint64_t> seen_hashes;
        
        uint64_t prev_hash = 0;
        bool first = true;

        // 4. 重複排除して採用
        for(const auto& cand : shortlist) {
            if (!first && cand.hash == prev_hash) continue;
            
            winners.push_back(cand);
            prev_hash = cand.hash;
            first = false;

            size_t win_idx = winners.size() - 1;
            uint32_t* src_ptr = &operations[cand.parent_idx * MAX_DEPTH];
            uint32_t* dst_ptr = &next_operations[win_idx * MAX_DEPTH];
            std::memcpy(dst_ptr, src_ptr, MAX_DEPTH * sizeof(uint32_t));
            
            dst_ptr[0] = cand.score; 
            dst_ptr[ope_num] = cand.move_id;

            if(winners.size() >= beam_width) break;
        }
        
        if(winners.empty()) break;

        best_score = winners[0].score;
        
        cudaMemcpy(d_winners, winners.data(), winners.size() * sizeof(CandidateResult), cudaMemcpyHostToDevice);
        
        uint32_t gather_blocks = (winners.size() + threadsPerBlock - 1) / threadsPerBlock;
        gather_next_generation_kernel<<<gather_blocks, threadsPerBlock>>>(
            fsize, field_size, winners.size(), d_winners, d_parent_fields, d_next_fields
        );
        
        std::swap(d_parent_fields, d_next_fields);
        std::swap(operations, next_operations);
        parent_count = winners.size();
        
        ope_num++;
    }
    
    // --- 終了処理 ---
    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() / 1000.0;
    result.moves = ope_num - 1;
    result.solved = (best_score >= fsize * fsize + 1);

    cudaFree(d_parent_fields);
    cudaFree(d_next_fields);
    cudaFree(d_candidates);
    cudaFree(d_winners);

    return result;
}

// 互換性のため残す
void test_beam_search() {}
void test_rotate() {}