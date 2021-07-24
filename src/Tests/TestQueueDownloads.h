#ifndef __TEST_QUEUE_DOWNLOADS_H__
#define __TEST_QUEUE_DOWNLOADS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <chrono>

#include "../DispatchCPP/DispatchCPP.h"

void testQueueDownloads(unsigned int maxNumThreads = 4);

#endif // __TEST_QUEUE_DOWNLOADS_H__