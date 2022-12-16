#include "Main.h"
using namespace std;
using namespace std::string_literals;
using namespace DispatchCPP;

// Default number of threads to use for each test.
#define DEFAULT_NUM_THREADS 12

// Entry point of our application.
int main(int numArgs, char ** ppArgs) {
	// Don't buffer stdout.
	setbuf(stdout, NULL);

	// The number of threads to use.
	unsigned int targetNumThreads = DEFAULT_NUM_THREADS;

	// Parse all our args, now.
	parseArgs(numArgs, ppArgs);

	// Determine what kind of test(s) to perform.
	bool testVectorSort = (argExists("tv"s) || argExists("test-vectors"s));
	bool testDownloads  = (argExists("td"s) || argExists("test-downloads"s));
	bool testFileIO     = (argExists("tf"s) || argExists("test-files"s));
	bool testMalloc     = (argExists("tm"s) || argExists("test-malloc"s));
	bool testThreads    = (argExists("tt"s) || argExists("test-threads"s));

	// Did the user specify a custom number of threads to use?
	auto testNumThreadsArg = pair<bool, size_t>(false, 0);
	if (argValueExists("j"s)) {
		testNumThreadsArg = getArgValueUInt("j"s);
	} else if (argValueExists("num-threads"s)) {
		testNumThreadsArg = getArgValueUInt("num-threads"s);
	}
	if (testNumThreadsArg.first && (testNumThreadsArg.second > 0)) {
		targetNumThreads = ((unsigned int) testNumThreadsArg.second);
	}

	// Call into each test we should perform.
	if (testVectorSort) { testQueueVectorSort(targetNumThreads); }
	if (testDownloads)  { testQueueDownloads(targetNumThreads);  }
	if (testFileIO)     { testQueueFileIO(targetNumThreads);     }
	if (testMalloc)     { testQueueMalloc(targetNumThreads);     }
	if (testThreads)    { testQueueThreads(targetNumThreads);    }

	return(EXIT_SUCCESS);
}
