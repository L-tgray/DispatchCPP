#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Args.h"

#include "DispatchCPP/DispatchCPP.h"

#include "Tests/TestQueueVectorSort.h"
#include "Tests/TestQueueDownloads.h"
#include "Tests/TestQueueFileIO.h"
#include "Tests/TestMalloc.h"
#include "Tests/TestThreads.h"

// Forward declaration of our application's entry point.
int main(int numArgs, char ** ppArgs);

#endif // __MAIN_H__
