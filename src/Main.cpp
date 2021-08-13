#include "Main.h"
using namespace std;
using namespace std::string_literals;
using namespace DispatchCPP;

// Default number of threads to use for each test.
#define DEFAULT_NUM_THREADS 10

// Entry point of our application.
int main(int numArgs, char ** ppArgs) {
	// Don't buffer stdout.
	setbuf(stdout, NULL);

	// Flags for what we should test.
	bool testVectorSort = false;
	bool testDownloads  = false;
	bool testFileIO     = false;
	bool testMalloc     = false;
	bool testThreads    = false;

	// Iterate over all the incoming arguments.
	for (int argIndex = 1; argIndex < numArgs; ++argIndex) {
		// Convert the argument into a string.
		string currentArg(ppArgs[argIndex]);

		// Parse the incoming arg, now.
		if ((currentArg == "-tv"s) || (currentArg == "--test-vectors"s)) {
			testVectorSort = true;
		} else if ((currentArg == "-td"s) || (currentArg == "--test-downloads"s)) {
			testDownloads = true;
		} else if ((currentArg == "-tf"s) || (currentArg == "--test-files"s)) {
			testFileIO = true;
		} else if ((currentArg == "-tm"s) || (currentArg == "--test-malloc"s)) {
			testMalloc = true;
		} else if ((currentArg == "-tt"s) || (currentArg == "--test-threads"s)) {
			testThreads = true;
		}
	}

	// Call into each test we should perform.
	if (testVectorSort) {
		testQueueVectorSort(DEFAULT_NUM_THREADS);
	}
	if (testDownloads) {
		testQueueDownloads(DEFAULT_NUM_THREADS);
	}
	if (testFileIO) {
		testQueueFileIO(DEFAULT_NUM_THREADS);
	}
	if (testMalloc) {
		testQueueMalloc(DEFAULT_NUM_THREADS);
	}
	if (testThreads) {
		testQueueThreads(DEFAULT_NUM_THREADS);
	}

/*
	// Create our post-test function.
	function<void()> testQueuePostFunc = []() {
		printf("OMG Thanks!\n");
	};

	// Create a test function.
	QueueFunction<void> * pTestQueueFunction = new QueueFunction<void>(
		[]() {
			printf("Hello, world %s!\n", (rand() % 2) ? "TRUE" : "FALSE");
		},
		nullptr,
		&testQueuePostFunc
	);

	// Try calling the test function of our QueueFunction object.
	printf("Test function has return value = %s\n", pTestQueueFunction->hasReturnValue() ? "TRUE" : "FALSE");

	// Create our test queue.
	Queue<void> testQueue(pTestQueueFunction, 2, true);

	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();
	testQueue.dispatchWork();

	testQueue.hasWorkLeft(true);
*/

	return(EXIT_SUCCESS);
}
