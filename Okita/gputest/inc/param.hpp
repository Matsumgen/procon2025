#ifndef PARAM_HPP_
#define PARAM_HPP_

// どれも2の倍数

#define BLOCKS_PER_GRID 512
#define THREADS_PER_BLOCK 256

// QUEUE_SIZE >= blocksPerGrid * threadsPerBlocks
#define QUEUE_SIZE (1<<20)

#define BEAM_WIDTH (1<<18)
#define FIELDS_PER_THREAD 32
#define CPU_THREAD_NUM 3
#define MAX_DEPTH 512
#define SLEEP_TIME 10

// fsizeの最大値
#define MAX_FSIZE 24

#endif
