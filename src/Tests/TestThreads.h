#ifndef __TEST_THREADS_H__
#define __TEST_THREADS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>

#include "DispatchCPP/DispatchCPP.h"
#include "Colors.h"

#define SRAND_INIT_VALUE 1234567

double testQueueMathManual(unsigned int numArrays, unsigned int numEntries);
double testQueueMathThreads(unsigned int numThreads, unsigned int numArrays, unsigned int numEntries);
void   testQueueThreads(unsigned int numThreads);

#endif // __TEST_THREADS_H__
