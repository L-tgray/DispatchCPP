#include "TestThreads.h"

using namespace DispatchCPP;

double testQueueMathManual(unsigned int numArrays, unsigned int numEntries) {
	// Declare our return value.
	double returnValue = 0.0f;

	// Allocate -------------------------------------------------------------------------------------

	// Initialize our seed to something the same, always.
	srand(SRAND_INIT_VALUE);

	double ** ppData = ((double **) malloc(sizeof(double *) * numArrays));
	for (unsigned int index = 0; index < numArrays; ++index) {
		ppData[index] = ((double *) malloc(sizeof(double) * numEntries));
		for (unsigned int subIndex = 0; subIndex < numEntries; ++subIndex) {
			ppData[index][subIndex] = (((double) rand()) / ((double) rand()));
		}
	}

	// Start our timer.
	auto beforeManual = chrono::high_resolution_clock::now();

	// Iterate over all arrays.
	double sumTotal = 0.0f;
	for (unsigned int index = 0; index < numArrays; ++index) {
		for (unsigned int subIndex = 0; subIndex < numEntries; ++subIndex) {
			for (unsigned int subSubIndex = 0; subSubIndex < subIndex; ++subSubIndex) {
				double currentValue = ppData[index][subSubIndex];
				sumTotal = ((subSubIndex % 2) ? (sumTotal + currentValue) : (sumTotal - currentValue));
			}
			
		}
	}

	// End our timer.
	auto afterManual = chrono::high_resolution_clock::now();

	// ----------------------------------------------------------------------------------------------

	// Calculate our time, now.
	returnValue = ((double) chrono::duration_cast<chrono::microseconds>(afterManual - beforeManual).count());

	// Deallocate: ----------------------------------------------------------------------------------

	// Iterate over all our arrays and deallocate them, and then deallocate our pointer to pointers.
	for (unsigned int index = 0; index < numArrays; ++index) {
		free(ppData[index]);
	}
	free(ppData);

	// ----------------------------------------------------------------------------------------------

	// Return how long it took to do all our operations.
	return(returnValue + (sumTotal * 0.0000000001f));
}

double testQueueMathThreads(unsigned int numThreads, unsigned int numArrays, unsigned int numEntries) {
	// Declare our return value.
	double returnValue = 0.0f;

	// Allocate: ------------------------------------------------------------------------------------

	// Initialize our seed to something the same, always.
	srand(SRAND_INIT_VALUE);

	double ** ppData = ((double **) malloc(sizeof(double *) * numArrays));
	for (unsigned int index = 0; index < numArrays; ++index) {
		ppData[index] = ((double *) malloc(sizeof(double) * numEntries));
		for (unsigned int subIndex = 0; subIndex < numEntries; ++subIndex) {
			ppData[index][subIndex] = (((double) rand()) / ((double) rand()));
		}
	}

	// Grab the current time as our start time.
	auto beforeParallel = chrono::high_resolution_clock::now();

	// Declare our Queue.
	Queue<double, unsigned int> * pMallocQueue = new Queue<double, unsigned int>(
		new QueueFunction<double, unsigned int>(
			[ppData, numEntries](unsigned int arrayIndex) {
				double sumTotal = 0.0f;
				for (unsigned int index = 0; index < numEntries; ++index) {
					for (unsigned int subIndex = 0; subIndex < index; ++subIndex) {
						double currentValue = ppData[arrayIndex][subIndex];
						sumTotal = ((subIndex % 2) ? (sumTotal + currentValue) : (sumTotal - currentValue));
					}
				}
				return(sumTotal);
			}
		),
		numThreads,
		true
	);

	// Iterate over all the work we have to dispatch.
	for (unsigned int index = 0; index < numArrays; ++index) {
		pMallocQueue->dispatchWork(index);
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
	for (unsigned int index = 0; index < numArrays; ++index) {
		free(ppData[index]);
	}
	free(ppData);

	// ----------------------------------------------------------------------------------------------

	// Return how long it took to do all our operations.
	return(returnValue);
}

void testQueueThreads(unsigned int maxNumThreads) {
	// The number of times to average each run.
	unsigned int numTimesToAverage = 3;

	// Declare the initial number of arrays and entries.
	unsigned int initNumArrays  = 32;
	unsigned int initNumEntries = 100;

	// Declare how much we multiply the number of entries each iteration.
	unsigned int multNumArrays  = 2;
	unsigned int multNumEntries = 10;

	// Declare our max number of entries.
	unsigned int maxNumArrays  = 512;
	unsigned int maxNumEntries = 10000;

	// Iterate over all the number of arrays we should use.
	for (unsigned int numArrays = initNumArrays; numArrays <= maxNumArrays; numArrays *= multNumArrays) {
		printf("========================================================================================\n");
		if (numArrays == initNumArrays) {
			printf("===                    All tests are averaged over %u run(s)                          ===\n", numTimesToAverage);
		}
		printf("========================================================================================\n");
		// Iterate over all the number of entries we should use.
		for (unsigned int numEntries = initNumEntries; numEntries <= maxNumEntries; numEntries *= multNumEntries) {
			printf("[Manually]    Num Arrays: %6u, Num Entries: %7u...", numArrays, numEntries);
			double manualRunTotal = 0.0f;
			for (unsigned int runIndex = 0; runIndex < numTimesToAverage; ++runIndex) {
				manualRunTotal += testQueueMathManual(numArrays, numEntries);
			}
			double manualRunAvg = manualRunTotal / ((double) numTimesToAverage);
			printf("%10.3f mS\n", manualRunAvg / 1000.0f);

			// Iterate over all the number of threads we should run.
			for (unsigned int numThreads = 1; numThreads <= maxNumThreads; ++numThreads) {
				printf("[%2u Thread%s]  %sNum Arrays: %6u, Num Entries: %7u...",
					numThreads,
					(numThreads == 1) ? "" : "s",
					(numThreads == 1) ? " " : "",
					numArrays,
					numEntries);

				double threadRunTotal = 0.0f;
				for (unsigned int runIndex = 0; runIndex < numTimesToAverage; ++runIndex) {
					threadRunTotal += testQueueMathThreads(numThreads, numArrays, numEntries);
				}
				double threadRunAvg = threadRunTotal / ((double) numTimesToAverage);
				printf("%10.3f mS, %s%.3fx speedup%s\n",
					threadRunAvg / 1000.0f,
					((threadRunAvg < manualRunAvg) ? Colors::pColorGreen : Colors::pColorRed),
					manualRunAvg / threadRunAvg,
					Colors::pColorReset);
			}
			if (numEntries != maxNumEntries) {
				printf("----------------------------------------------------------------------------------------\n");
			}
		}
	}
}
