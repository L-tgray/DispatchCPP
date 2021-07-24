#ifndef __TEST_QUEUE_VECTOR_SORT_H__
#define __TEST_QUEUE_VECTOR_SORT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>

#include "../DispatchCPP/DispatchCPP.h"

#include "Colors.h"

// This define enables allocating the vectors in parallel.
#define ENABLE_PARALLEL_ALLOCATIONS

// This define enables deallocating the vectors in parallel.
#define ENABLE_PARALLEL_DEALLOCATIONS

void testQueueVectorSort(unsigned int maxNumThreads = 4);

#endif // __TEST_QUEUE_VECTOR_SORT_H__