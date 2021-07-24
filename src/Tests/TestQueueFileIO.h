#ifndef __TEST_QUEUE_FILE_IO_H__
#define __TEST_QUEUE_FILE_IO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <chrono>

#include "../DispatchCPP/DispatchCPP.h"

void testQueueFileIO(unsigned int maxNumThreads = 4);

void copyFileManually(string srcFile, string dstFile, unsigned int numCopies);

#endif // __TEST_QUEUE_FILE_IO_H__