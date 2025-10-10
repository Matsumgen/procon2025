#ifndef ALGO_HPP_
#define ALGO_HPP_

#include <Field.hpp>

int serchShortestStep1(Field& f, const int *from, const int *to, int **ret);

void alg1(Field& f);
void alg1_1(Field& f, int rw, int deep, unsigned int leaves_limit);

#endif
