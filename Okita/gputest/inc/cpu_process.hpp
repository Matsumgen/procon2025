#ifndef CPU_PROCESS_HPP_
#define CPU_PROCESS_HPP_

#include <stdint.h>
#include <vector>

bool checkProblem(const uint32_t fsize, const uint16_t *f);
std::vector<uint16_t> makeShuffledPairs(const uint16_t fsize);
void printField(const uint32_t fsize, const uint16_t *field, const uint16_t *before_f);
void printField(const uint32_t fsize, const uint16_t *field);
uint32_t getTargetIndex(const uint16_t *vals, const uint32_t length, const uint16_t v);
void getParamsCpu(const uint32_t tid, const uint32_t fsize, uint32_t *X, uint32_t *Y, uint32_t *N);

void rotateField(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n);
void rotateField(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t rid);

bool isIndexAdjacent(std::vector<uint16_t>& field, const uint32_t fsize, const uint32_t i);
bool isEnd(std::vector<uint16_t>& field, const uint32_t fsize);

#endif
