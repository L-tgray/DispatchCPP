#include "TestMalloc.h"

using namespace DispatchCPP;

double testQueueMallocWithSizesManual(unsigned int numEntries, unsigned int bufferSize) {
	// Declare our return value.
	double returnValue = 0.0f;

	// Allocate -------------------------------------------------------------------------------------

	// Declare our pointer to pointers.
	void ** ppEntries = ((void **) malloc(numEntries * sizeof(void *)));

	// Start our timer.
	auto beforeManual = chrono::high_resolution_clock::now();

	// Iterate over all entries we have to allocate.
	for (unsigned int index = 0; index < numEntries; ++index) {
		ppEntries[index] = ((void *) malloc(bufferSize * sizeof(char)));
	}

	// End our timer.
	auto afterManual = chrono::high_resolution_clock::now();

	// ----------------------------------------------------------------------------------------------

	// Calculate our time, now.
	returnValue = ((double) chrono::duration_cast<chrono::microseconds>(afterManual - beforeManual).count());

	// Deallocate: ----------------------------------------------------------------------------------

	// Iterate over all our entries and deallocate them, and then deallocate our pointer to pointers.
	for (unsigned int index = 0; index < numEntries; ++index) {
		free(ppEntries[index]);
	}
	free(ppEntries);

	// ----------------------------------------------------------------------------------------------

	// Return how long it took to do all our operations.
	return(returnValue);
}

double testQueueMallocWithSizes(unsigned int numThreads, unsigned int numEntries, unsigned int bufferSize) {
	// Declare our return value.
	double returnValue = 0.0f;

	// Allocate: ------------------------------------------------------------------------------------

	// Declare our pointer to pointers.
	void ** ppEntries = ((void **) malloc(numEntries * sizeof(void *)));

	// Grab the current time as our start time.
	auto beforeParallel = chrono::high_resolution_clock::now();

	// Declare our Queue.
	Queue<void, void **, unsigned int> * pMallocQueue = new Queue<void, void **, unsigned int>(
		new QueueFunction<void, void **, unsigned int>(
			[](void ** ppEntry, unsigned int sizeOfEntry) {
				*ppEntry = (void *) malloc(sizeOfEntry * sizeof(char));
			}
		),
		numThreads,
		true
	);

	// Iterate over all the work we have to dispatch.
	for (unsigned int index = 0; index < numEntries; ++index) {
		pMallocQueue->dispatchWork(&(ppEntries[index]), numEntries);
	}

	// Wait for all work to finish.
	pMallocQueue->hasWorkLeft(true);

	// Grab the current time as our end time.
	auto afterParallel = chrono::high_resolution_clock::now();

	// Calculate our return value.
	returnValue = ((double) chrono::duration_cast<chrono::microseconds>(afterParallel - beforeParallel).count());

	// Deallocate: ----------------------------------------------------------------------------------

	// Clean up after ourselves.
	delete(pMallocQueue);

	// Iterate over all entries, deallocating each of them, as well as our pointer to pointers.
	for (unsigned int index = 0; index < numEntries; ++index) {
		free(ppEntries[index]);
	}
	free(ppEntries);

	// ----------------------------------------------------------------------------------------------

	// Return how long it took to do all our operations.
	return(returnValue);
}

void testQueueMalloc(unsigned int maxNumThreads) {
	// The number of times to average each run.
	unsigned int numTimesToAverage = 3;

	// Declare the initial number of entries and buffer size.
	unsigned int initNumEntries = 50;
	unsigned int initBufferSize = 1024;

	// Declare how much we multiply the number of entries and buffer size each iteration.
	unsigned int multNumEntries = 10;
	unsigned int multBufferSize = 4;

	// Declare our max number of entries and max buffer size.
	unsigned int maxNumEntries = 50000;
	unsigned int maxBufferSize = 65536;

	// Iterate over all the buffer sizes we should use.
	for (unsigned int bufferSize = initBufferSize; bufferSize <= maxBufferSize; bufferSize *= multBufferSize) {
		printf("==========================================================================\n");
		printf("==========================================================================\n");
		// Iterate over all the number of entries we should use.
		for (unsigned int numEntries = initNumEntries; numEntries <= maxNumEntries; numEntries *= multNumEntries) {
			printf("[Manually]    Num Entries: %6u, Buffer Size: %6u...", numEntries, bufferSize);
			double manualRunTotal = 0.0f;
			for (unsigned int runIndex = 0; runIndex < numTimesToAverage; ++runIndex) {
				manualRunTotal += testQueueMallocWithSizesManual(numEntries, bufferSize);
			}
			double manualRunAvg = manualRunTotal / ((double) numTimesToAverage);
			printf("%9.3f mS\n", manualRunAvg / 1000.0f);

			// Iterate over all the number of threads we should run.
			for (unsigned int numThreads = 1; numThreads <= maxNumThreads; ++numThreads) {
				printf("[%2u Thread%s]  %sNum Entries: %6u, Buffer Size: %6u...",
					numThreads,
					(numThreads == 1) ? "" : "s",
					(numThreads == 1) ? " " : "",
					numEntries,
					bufferSize);

				double threadRunTotal = 0.0f;
				for (unsigned int runIndex = 0; runIndex < numTimesToAverage; ++runIndex) {
					threadRunTotal += testQueueMallocWithSizes(numThreads, numEntries, bufferSize);
				}
				double threadRunAvg = threadRunTotal / ((double) numTimesToAverage);

				printf("%9.3f mS\n", threadRunAvg / 1000.0f);
			}
			if (numEntries != maxNumEntries) {
				printf("--------------------------------------------------------------------------\n");
			}
		}
	}
}
