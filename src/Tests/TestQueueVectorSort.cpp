#include "TestQueueVectorSort.h"

using namespace std;
using namespace std::string_literals;
using namespace DispatchCPP;

// Forward declarations of our functions.
void testVectorSortingManually(unsigned int numVectors, unsigned int vectorSize);
void testVectorSortingDispatchCPP(unsigned int numVectors, unsigned int vectorSize, unsigned int numThreads);

void testQueueVectorSort(unsigned int maxNumThreads) {
    // Iterate over the sizes of the vectors we'll be sorting.
    // for (unsigned int vectorSize = 4096; vectorSize <= 131072; vectorSize *= 2) {
    for (unsigned int vectorSize = 2048; vectorSize <= 131072; vectorSize *= 4) {
        // Iterate over the number of vectors we have to sort.
        bool isFirstRun = true;
        for (unsigned int numVectors = 50; numVectors <= 5000; numVectors *= 10) {
            // Make things pretty.
            if (!isFirstRun) {
                printf("---------------------------------------------------------------------------------------------------\n");
            } else {
                isFirstRun = false;
            }

            // Let the user know what we're doing.
            printf("%s      Manually: %6u vectors, %6u entries/vector => ", Colors::pColorGreen, numVectors, vectorSize);

            // The number of times we should average the manual run.
            unsigned int numManualRuns = 5;

            // The average number of microseconds the manual run took.
            double numMicrosecondsBase = 0.0;

            // Iterate over all the manual runs we should perform.
            for (unsigned int manualRunIndex = 0; manualRunIndex < numManualRuns; ++manualRunIndex) {
                // Manually run the sorting of vectors.
                auto beforeManual = chrono::high_resolution_clock::now();
                testVectorSortingManually(numVectors, vectorSize);
                auto afterManual = chrono::high_resolution_clock::now();

                // Calculate the time it took to perform this manual run.
                numMicrosecondsBase += chrono::duration_cast<chrono::microseconds>(afterManual - beforeManual).count();
            }

            // Average them, now.
            numMicrosecondsBase = (numMicrosecondsBase / ((double) numManualRuns));

            // Calculate the amount of time the manual run took.
            printf("%9.2fms (averaged over %1u runs)%s\n", numMicrosecondsBase / 1000.0, numManualRuns, Colors::pColorReset);

            // -------------------------------------------------------------------------------------------

            // Declare the number of threads we'll be using in our loop.
            unsigned int numTimesToRunEachThreadCount = 2;

            // Declare our variables for calculating our speedup averages.
            double       totalThreadSpeedup = 0.0;
            unsigned int totalThreadCount   = 0;

            // Iterate over all the variants of threads we want to test (two times apiece)
            for (unsigned int numThreadsLoop = numTimesToRunEachThreadCount; numThreadsLoop <= ((maxNumThreads * numTimesToRunEachThreadCount) + (numTimesToRunEachThreadCount - 1)); ++numThreadsLoop) {
                // Calculate the number of threads we should be using.
                unsigned int numThreads = (numThreadsLoop / numTimesToRunEachThreadCount);

                // Decloare our current color to default to being normal text, unless we're executing with only 1 thread.
                const char * pCurrentColor = Colors::pColorReset;
                if (numThreads == 1) {
                    pCurrentColor = Colors::pColorGreen;
                }

                // Output what we're attempting to do.
                printf("%s%s %2u %sThread%s: %6u vectors, %6u entries/vector => ",
                    pCurrentColor,
                    (numThreadsLoop % numTimesToRunEachThreadCount) ? "   " : "---",
                    numThreads,
                    (numThreads == 1) ? " " : "",
                    (numThreads == 1) ? "" : "s",
                    numVectors,
                    vectorSize);
                
                // Perform our calculations.
                auto beforeDispatch = chrono::high_resolution_clock::now();
                testVectorSortingDispatchCPP(numVectors, vectorSize, numThreads);
                auto afterDispatch = chrono::high_resolution_clock::now();

                // Output the result.
                double numMicroseconds = chrono::duration_cast<chrono::microseconds>(afterDispatch - beforeDispatch).count();
                double speedup = (numMicrosecondsBase / numMicroseconds);
                printf("%9.2fms ", numMicroseconds / 1000.0);

                // Should we output a negative/bad result?
                if (numMicroseconds < numMicrosecondsBase) {
                    printf("(%s-%7.1f%%, %4.2fx speedup%s)%s\n",
                        Colors::pColorGreen,
                        (1.0 - (numMicroseconds / numMicrosecondsBase)) * 100.0,
                        speedup,
                        pCurrentColor,
                        ((numThreadsLoop % numTimesToRunEachThreadCount) ? "" : " ---"));
                    totalThreadSpeedup += speedup;
                    totalThreadCount   += 1;

                // Should we output a positive/good result?
                } else {
                    printf("(%s+%7.1f%%, %4.2fx speedup%s)%s\n",
                        Colors::pColorRed,
                        ((numMicroseconds / numMicrosecondsBase) - 1.0) * 100,
                        speedup,
                        pCurrentColor,
                        ((numThreadsLoop % numTimesToRunEachThreadCount) ? "" : " ---"));
                    totalThreadSpeedup += speedup;
                    totalThreadCount   += 1;
                }

                // Is this our final iteration through the loop?
                if (numThreadsLoop == ((maxNumThreads * numTimesToRunEachThreadCount) + (numTimesToRunEachThreadCount - 1))) {
                    // Calculate our average speedup.
                    double averageSpeedup = (totalThreadSpeedup / ((double) totalThreadCount));
                    printf("%sAverage speedup: %6.3fx%s\n", Colors::pColorMagenta, averageSpeedup, Colors::pColorReset);
                }
            }
        }
        printf("===================================================================================================\n");
        printf("===================================================================================================\n");
    }
}

void testVectorSortingManually(unsigned int numVectors, unsigned int vectorSize) {
    // Declare our array of vectors.
    vector<unsigned int> ** ppAllVectors = (vector<unsigned int> **) malloc(numVectors * sizeof(vector<unsigned int> *));

    // Iterate over all of the vectors we have to initialize.
    for (unsigned int index = 0; index < numVectors; ++index) {
        // Create our vector.
        vector<unsigned int> * pCurrentVector = new vector<unsigned int>();

        // Did we fail to create a new vector?
        if (!pCurrentVector) {
            printf("FAILED TO ALLOCATE A NEW VECTOR\n");
            exit(EXIT_FAILURE);
        }

        // Reserve our space.
        pCurrentVector->reserve(vectorSize);

        // Initialize our randomness to something we can expect.
        srand(123456);

        // Iterate over the size of each vector.
        for (unsigned int subIndex = 0; subIndex < vectorSize; ++subIndex) {
            pCurrentVector->push_back(rand());
        }

        // Now that we've initialized the vector, save it to the vector of vectors.
        ppAllVectors[index] = pCurrentVector;
    }

    // Iterate over all of the vectors, sorting each of them.
    for (unsigned int index = 0; index < numVectors; ++index) {
        // Grab the current vector.
        vector<unsigned int> * pCurrentVector = ppAllVectors[index];

        // Sort it, now.
        sort(pCurrentVector->begin(), pCurrentVector->end());
    }

    // Clean up after ourselves.
    for (unsigned int index = 0; index < numVectors; ++index) {
        vector<unsigned int> * pCurrentVector = ppAllVectors[index];
        if (pCurrentVector) {
            delete(pCurrentVector);
        }
    }
    free(ppAllVectors);
}

vector<unsigned int> ** testVectorSortingDispatchCPP_Allocate(unsigned int numVectors, unsigned int vectorSize, unsigned int numThreads) {
    vector<unsigned int> ** ppAllVectors = ((vector<unsigned int> **) malloc(numVectors * sizeof(vector<unsigned int> *)));
    if (!ppAllVectors) {
        printf("FAILED TO ALLOCATE SPACE FOR ALL OUR VECTOR POINTERS\n");
        exit(EXIT_FAILURE);
    }

#ifdef ENABLE_PARALLEL_ALLOCATIONS
    // Create our Queue to allocate all the space for us.
    Queue<bool, unsigned int> * pAllocationQueue = new Queue<bool, unsigned int>(
        new QueueFunction<bool, unsigned int>(
            [ppAllVectors, vectorSize](unsigned int targetIndex) {
                vector<unsigned int> * pNewVector = new vector<unsigned int>();
                if (pNewVector) {
                    pNewVector->reserve(vectorSize);
                    // Initialize our randomness to something we can expect.
    	            srand(123456);
                    for (unsigned int index = 0; index < vectorSize; ++index) {
                        pNewVector->push_back(rand());
                    }
                    ppAllVectors[targetIndex] = pNewVector;
                    return(true);
                }
                return(false);
            }
        ),
        numThreads,
        true
    );
#endif // ENABLE_PARALLEL_ALLOCATIONS

    // Iterate over all of the vectors we have to initialize.
    for (unsigned int index = 0; index < numVectors; ++index) {
#ifndef ENABLE_PARALLEL_ALLOCATIONS

        // Create our vector.
        vector<unsigned int> * pCurrentVector = new vector<unsigned int>();

        // Did we fail to create a new vector?
        if (!pCurrentVector) {
            printf("FAILED TO ALLOCATE A NEW VECTOR\n");
            exit(EXIT_FAILURE);
        }

        // Reserve our space.
        pCurrentVector->reserve(vectorSize);

        // Initialize our randomness to something we can expect.
        srand(123456);

        // Iterate over the size of each vector.
        for (unsigned int subIndex = 0; subIndex < vectorSize; ++subIndex) {
            pCurrentVector->push_back(rand());
        }

        // Now that we've initialized the vector, save it to the vector of vectors.
        ppAllVectors[index] = pCurrentVector;

#else // ENABLE_PARALLEL_ALLOCATIONS

        // Dispatch our work.
        pAllocationQueue->dispatchWork(index);

#endif // ENABLE_PARALLEL_ALLOCATIONS
    }

#ifdef ENABLE_PARALLEL_ALLOCATIONS
    // Wait for everything to be allocated.
    pAllocationQueue->hasWorkLeft(true);

    // Delete the queue, now.
    delete(pAllocationQueue);
#endif // ENABLE_PARALLEL_ALLOCATIONS

    // Return our array of vectors!
    return(ppAllVectors);
}

void testVectorSortingDispatchCPP_Deallocate(vector<unsigned int> ** ppAllVectors, unsigned int numVectors, unsigned int numThreads) {
#ifdef ENABLE_PARALLEL_DEALLOCATIONS
    // Declare our deallocation queue.
    Queue<bool, vector<unsigned int> *> * pDeallocationQueue = new Queue<bool, vector<unsigned int> *>(
        new QueueFunction<bool, vector<unsigned int> *>(
            [](vector<unsigned int> * pCurrentVector) {
                if (pCurrentVector) {
                    delete(pCurrentVector);
                    return(true);
                }
                return(false);
            }
        ),
        numThreads,
        true
    );
#endif // ENABLE_PARALLEL_DEALLOCATIONS

    // Clean up after ourselves.
    for (unsigned int index = 0; index < numVectors; ++index) {
#ifndef ENABLE_PARALLEL_DEALLOCATIONS

        vector<unsigned int> * pCurrentVector = ppAllVectors[index];
        if (pCurrentVector) {
            delete(pCurrentVector);
        }

#else // ENABLE_PARALLEL_DEALLOCATIONS

        // Dispatch the deallocation work, now.
        pDeallocationQueue->dispatchWork(ppAllVectors[index]);

#endif // ENABLE_PARALLEL_DEALLOCATIONS
    }

#ifdef ENABLE_PARALLEL_DEALLOCATIONS
    // Wait for all the deallocations to happen.
    pDeallocationQueue->hasWorkLeft(true);

    // Delete our deallocation queue, now.
    delete(pDeallocationQueue);
#endif // ENABLE_PARALLEL_DEALLOCATIONS

    // Now that all the work has finished, delete our pointer to our pointers.
    free(ppAllVectors);
}

void testVectorSortingDispatchCPP(unsigned int numVectors, unsigned int vectorSize, unsigned int numThreads) {
    // Allocate our array of vectors.
    vector<unsigned int> ** ppAllVectors = testVectorSortingDispatchCPP_Allocate(numVectors, vectorSize, numThreads);

    // Declare our new Queue.
    Queue<void, vector<unsigned int> *> * pTestQueue = new Queue<void, vector<unsigned int> *>(
        new QueueFunction<void, vector<unsigned int> *>(
            // Our main func which returns nothing (void) and takes in a "vector<unsigned int> *"
            [](vector<unsigned int> * pTargetVector) {
                sort(pTargetVector->begin(), pTargetVector->end());
            },
            // These default to nullptr, so you could omit these last two arguments to the QueueFunction.
            nullptr, // No pre func
            nullptr  // No post func
            /* []() {
                pthread_t tid = pthread_self();
                printf("Thread started, TID = %llu\n", (unsigned long long int) tid);
            },
            []() {
                pthread_t tid = pthread_self();
                printf("Thread stopped, TID = %llu\n", (unsigned long long int) tid);
            } */
        ),
        numThreads,
        true
    );

    // Iterate over all of the vectors, sorting each of them.
    for (unsigned int index = 0; index < numVectors; ++index) {
        // Grab the current vector.
        vector<unsigned int> * pCurrentVector = ppAllVectors[index];

        // Dispatch the work, now.
        pTestQueue->dispatchWork(pCurrentVector);
    }

    // Wait for all the work to complete.
    pTestQueue->hasWorkLeft(true);

    // Delete our test queue, now.
    delete(pTestQueue);

    // Deallocate our array of vectors.
    testVectorSortingDispatchCPP_Deallocate(ppAllVectors, numVectors, numThreads);
}
