#include <algo2.cuh>
#include <algo2lib.cuh>
#include <algo2lib.hpp>
#include <algo2_algo1.hpp>
#include <vector>
#include <iostream>
#include <thread>
#include <queue>
#include <algorithm>

// getParamsでpaddingを考える

namespace algo2_algo1 {
  using namespace algo2lib;

MemObj21::MemObj21()
: fids(std::vector<uint16_t>(MemObj21::BEAM_WIDTH, 0)),
  opes_cpu(std::vector<uint8_t>(MemObj21::BEAM_WIDTH * (MemObj21::ROT_DEPTH + 1) * 3)),
  types(std::vector<uint16_t>(MemObj21::BEAM_WIDTH))
{
  const size_t field_size = 24 * 24;

  cudaError_t err = cudaMalloc(&fields_gpu, MemObj2::BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }
  err = cudaMalloc(&fields_gpu_buf, MemObj2::BEAM_WIDTH * field_size * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }
  err = cudaMalloc(&types_gpu, MemObj2::BEAM_WIDTH * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }

  err = cudaMalloc(&result_gpu, MemObj2::GPU_PROCESS_NUM * (MemObj2::ROT_DEPTH + 1) * sizeof(uint16_t));
  if (err != cudaSuccess) {
    std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
    return;
  }



}




__device__
void getParam1(uint16_t rid, uint32_t fsize, uint16_t type, uint32_t *X, uint32_t *Y, uint32_t *N) {
  uint32_t nowfsize = fsize - ((type >> 12) & 0b1100);
}
__device__
void getParam2(uint16_t rid, uint32_t fsize, uint16_t type, uint32_t *X, uint32_t *Y, uint32_t *N) {
}
__device__
uint32_t getRidsPerField1(const uint16_t type){
};
__device__
uint32_t getRidsPerField2(const uint16_t type){
};

Ope getParamsCpu(rid, fsize, type) {

}

// result: GPU_PROCESS_NUM * (ROT_DEPTH + 1): [type, rot1, rot2, ...]
void beam_search_kernel_depth2(uint16_t *fields, uint16_t *types const uint32_t field_len, uint32_t fsize, *uint16_t result, const uint32_t start_fid, const uint32_t start_rid ) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;

  uint32_t fid = (start_fid + gpu_id) % field_len;
  uint32_t rid = start_rid + (start_fid + gpu_id) / field_len;
  uint16_t type = fields[fid]


  const uint32_t field_size = fsize * fsize;
  uint16_t *field = fields * fid * field_size;

  const uint32_t ridsPerField1 = getRidsPerField1(type);
  const uint32_t ridsPerField2 = getRidsPerField2(type);

  if(rid >= ridsPerField1 * (ridsPerField2 + 1)) {
    result[gpu_id * (MemObj21::ROT_DEPTH + 1)] = 0; // type=0は最初しかない
    return;
  }

  const uint32_t rid1 = rid % ridsPerfield1;
  const uint32_t rid2 = rid / ridsPerfield1;

  const uint32_t fields4 = field_size >> 2;
  const bool isdepth2 = rid2 != ridsPerField2;

  uint16_t next_field[24 * 24];
  uint32_t X1, Y1, N1, X2, Y2, N2;
  uint32_t rot1[2], rot2[2], rot12[2], ps1[2], ps2[2], p[2];
  bool kasanari = false;
  getParams1(rid1, fsize, type, &X1, &Y1, &N1);
  createMatrixArrayL(X1, Y1, N1, rot1);
  
  if(isdepth2) {
    getParams2(rid2, fsize, type, &X2, &Y2, &N2);
    createMatrixArrayL(X2, Y2, N2, rot2);
    multDp4a(rot1, rot2, rot12);

    ps1[0] = X1 > X2 ? X1 : X2;                             // left
    ps1[1] = Y1 + N1-1 < Y2 + N2-1 ? Y1 + N1-1 : Y2 + N2-1; // dowN
    ps2[0] = X1 + N1-1 < X2 + N2-1 ? X1 + N1-1 : X2 + N2-1; // right
    ps2[1] = Y1 > Y2 ? Y1 : Y2;                             // up
    bool kasanari = ps1[0] <= ps2[0] && ps2[1] <= ps1[1];
    toInverse(rot2, p);
    culcDp4a(p, ps1);
    culcDp4a(p, ps2);
  }


  ushort4 *nf = (ushort4*)next_field;
  ushort4 *f = (ushort4*)field;
  for(uint32_t i = 0; i < fields4; ++i) { nf[i] = f[i]; }

  for(uint32_t y = Y1, i, j; y < Y1+N1; ++y) {
    i = y * fsize + X1;
    for(uint32_t x = X1; x < X1+N1; ++x) {
      culcDp4a(rot1, x, y, p);
      j = p[1] * fsize + p[0];
      next_field[i++] = field[j];
    }
  }

  if(isdepth2) {
    for(uint32_t y = Y2, i, j; y < Y2+N2; ++y) {
      i = y * fsize + X2;
      for(uint32_t x = X2; x < X2+N2; ++x) {
        culcDp4a(rot2, x, y, p);
        j = p[1] * fsize + p[0];
        next_field[i++] = field[j];
      }
    }
    if(kasanari){
      for(uint32_t y = ps1[1], i, j; y <= ps2[1]; ++y) {
        i = y * fsize + ps1[0];
        for(uint32_t x = ps1[0]; x <= ps2[0]; ++x) {
          culcDp4a(rot12, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = field[j];
        }
      }
    }
  }


  getCheckPoint(p, type);
  uint32_t i = p[1] * fsize + p[0];
  bool a = next_field[i] == next_field[i+1];
  bool b = next_field[i+1] == next_field[i+fsize+1];
  bool c = next_field[i+fsize] == next_field[i+fsize+1];
  bool d = next_field[i+fsize] == next_field[i];
  uint32_t t = type >> 12;
  i = gpu_id * (MemObj21::ROT_DEPTH + 1);
  if(a && c || b && d) {
    result[i++] = type + 2;
  }else if(t == 0 && a || t == 1 && b || t == 2 && c || t == 3 && d){
    result[i++] = type + 1;
  }else{
    result[i++] = 0;
  }
  result[i++] = rot1;
  result[i++] = isdepth2 ? rot2 : 0xffff;
}

void beam_search_kernel_after(uint16_t *fields, uint16_t *fields_buf, uint16_t *fies_gpu, uint8_t *opes, uint32_t field_len, uint32_t fsize) {
  const uint32_t gpu_id = blockIdx.x * blockDim.x + threadIdx.x;
  if(gpu_id < field_len) {
    const uint32_t field_size = fsize * fsize;
    uint16_t *field = fields + fids[gpu_id] * field_size;
    uint16_t *next_field = fields_buf + gpu_id * field_size;

    const uint32_t fields4 = field_size >> 2;
    ushort4 *nf = (ushort4*)next_field;
    ushort4 *f = (ushort4*)field;
    for(uint32_t i = 0; i < fields4; ++i) { nf[i] = f[i]; }

    uint32_t rot[2], p[2];
    uint8_t *ope = opes + gpu_id * (MemObj21::ROT_DEPTH + 1) * 3;

    if(ope[2] == 0) break;

    createMatrixArrayL(ope[0], ope[1], ope[2], rot);
    for(uint32_t y = ope[1], i, j; y < ope[1]+ope[2]; ++y) {
      i = y * fsize + ope[0];
      for(uint32_t x = ope[0]; x < ope[0]+ope[2]; ++x) {
        culcDp4a(rot1, x, y, p);
        j = p[1] * fsize + p[0];
        next_field[i++] = field[j];
      }
    }

    ope += 3;
    uint16_t fbuf[24 * 24];
    f = (ushort4*)fbuf;

    for(size_t t = 1; t < MemObj21::ROT_DEPTH + 1 && ope[2] != 0; ++t) {
      for(uint32_t i = 0; i < fields4; ++i) { f[i] = nf[i]; }
      createMatrixArrayL(ope[0], ope[1], ope[2], rot);
      for(uint32_t y = ope[1], i, j; y < ope[1]+ope[2]; ++y) {
        i = y * fsize + ope[0];
        for(uint32_t x = ope[0]; x < ope[0]+ope[2]; ++x) {
          culcDp4a(rot1, x, y, p);
          j = p[1] * fsize + p[0];
          next_field[i++] = fbuf[j];
        }
      }
      ope += 3;
    }
  }
}

template<typename Func>
void run_gpu(
    Func gpu_process,
    uint16_t *field_gpu,
    uint16_t *types_gpu,
    uint16_t *result_gpu,
    ControlThread& ct
) {

  uint32_t start_fid, start_rid;
  while(true){
    ct.wait_gpu();
    uint32_t field_len = ct.field_len
    uint32_t fsize = ct.fsize;
    gpu_process<<<MemObj21::BLOCKS_PER_GRID, MemObj21::THREADS_PER_BLOCK>>>(field_gpu, types_gpu, field_len, fsize, result_gpu, start_fid, start_rid);
    cudaDeviceSynchronize();
    for(size_t idx = MemObj21::GPU_PROCESSS_NUM; idx < 4323; rid += MemObj21::GPU_PROCESS_NUM) {
      start_fid = idx % field_len;
      start_rid = idx / field_len;

      cudaMemcpy(ct.result_buf, result_gpu, sizeof(uint16_t) * MemObj21::GPU_PROCESS_NUM, cudaMemcpyDeviceToHost);
      gpu_process<<<MemObj21::BLOCKS_PER_GRID, MemObj21::THREADS_PER_BLOCK>>>(field_gpu, types_gpu, field_len, fsize, result_gpu, start_fid, start_rid);

      ct.setAfterTask();

      cudaDeviceSynchronize();
    }
    cudaMemcpy(ct.result_buf, result_gpu, sizeof(uint16_t) * MemObj21::GPU_PROCESS_NUM, cudaMemcpyDeviceToHost);
    ct.setAfterTask();
  }
}


void algo2_algo1(RawField field, uint32_t fsize, MemObj21& mem21, std::vector<std::vector<Ope>>& opes, std::vector<RawField>& fields, std::vector<std::pair<uint8_t, uint8_t>> &offsets);
  cudaFree(0);
  std::cout << "start algo2_algo1" << std::endl;

  ControlThread& ct = mem21.ct;
  ct.field_len = 1;
  ct.fsize = fsize;
  ct.resultopes = opes;

  auto gpu_process = beam_search_kernel_depth2;

  std::vector<uint16_t>& fids = mem21.fids;
  std::vector<uint8_t>& opes_cpu = mem21.opes_cpu;
  std::vector<uint16_t>& types mem21.types;

  std::thread gthread([&](){run_gpu(gpu_process, mem21.fields_gpu, mem21.types_gpu, mem21.result_gpu, ct)});
  std::thread ctthread(ct.afterTask);

  while(true) {
    ct.join_gpu();
    ct.join_afterTask();

    if(opes.size() << 1024) break;

    size_t siz = ct.tq.size();

    // ompつかう？
    for(size_t i = 0; i < siz; ++i) {
      std::array<uint16_t>& arr = ct.tq.data[i];
      uint16_t fid = arr[0];
      fids[i] = fid;
      ct.bresultOperations[i] = ct.resultOperations[fid];
      uint16_t type = types[fid];
      for(size_t j = 0; j < MemObj21::ROT_DEPTH + 1; ++j){
        uint16_t rid = arr[j+1];
        if(rid == 0x3fff) {
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3] = 0;
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3+1] = 0;
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3+2] = 0;
        }else{
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
          ct.bresultOperations[i].push_back(ope);
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3] = ope.x();
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3+1] = ope.y();
          opes_cpu[i*3*(MemObj21::ROT_DEPTH+1)+j*3+2] = ope.n();
        }
      }
    }


    cudaMemcpy(fids.data(), mem21.fids_gpu, sizeof(uint16_t) * siz, cudaMemcpyHostToDevice);
    cudaMemcpy(opes_cpu.data(), mem21.opes_gpu, sizeof(uint8_t) * siz * 3 * (MemObj21::ROT_DEPTH+1), cudaMemcpyHostToDevice);

    
    uint32_t bp = (siz / MemObj2::THREADS_PER_BLOCK) + 1;
    beam_search_kernel_after<<<bp, MemObj21::THREADS_PER_BLOCK>>>(mem21.fields_gpu, mem21.fields_gpu_buf, mem21.fies_gpu, mem21.opes_gpu, field_len, fsize);

    cudaMemcpy(ct.tq.types.data(), mem21.types_gpu, sizeof(uint16_t) * siz, cudaMemcpyHostToDevice);
    std::swap(mem21.fields_gpu, mem21.fields_gpu_buf);
    std::swap(ct.resultOperations, ct.bresultOperations);
    std::swap(types, ct.tq.types);
    cudaDeviceSynchronize();

  }

}



}
