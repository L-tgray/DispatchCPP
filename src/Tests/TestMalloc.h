#ifndef __TEST_MALLOC_H__
#define __TEST_MALLOC_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>

#include "DispatchCPP/DispatchCPP.h"

double testQueueMallocWithSizesManual(unsigned int numEntries, unsigned int bufferSize);
double testQueueMallocWithSizes(unsigned int numThreads, unsigned int numEntries, unsigned int bufferSize);

void testQueueMalloc(unsigned int numThreads);

#endif // __TEST_MALLOC_H__
