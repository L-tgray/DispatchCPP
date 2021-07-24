# DispatchCPP
A header-only library for asynchronously dispatching C++ code both in parallel and serially. This easy-to-use library allows a variable (zero, or more than zero) amount of arguments to your parallel/serial work you dispatch.

Full, compilable examples at the bottom:
- [Example 1 - Dispatching Simple Things](#full-example-1)
- [Example 1 - Dispatching Simple Things (Condensed)](#full-example-1-condensed)
- [Example 2 - Capturing and Using Variables](#full-example-2-capturing-and-using-variables)
- [Example 3 - Serializing Work Done In Parallel](#full-example-3-serializing-work-done-in-parallel)

## To install:
1. `$ git clone https://github.com/L-tgray/DispatchCPP.git`
2. `$ cd DispatchCPP`
3. `$ make install`

This will install DispatchCPP into `/usr/local/include`.

## As an initial recap, to use this async library, you simply:
1. **Create a function** to be executed by the queue for each dispatch. This is where you declare the return value and parameters (if any).
```c++
// Example of a function returning void, and having zero parameters.
QueueFunction<void> * pFuncVoid = new QueueFunction<void>(
    []() {                           // Main function.
        printf("Hello, world!\n");
    },
    nullptr,                         // No pre function specified (optional, defaults to nullptr).
    nullptr                          // No post function specified (optional, defaults to nullptr).
);

// Example of a function returning void, taking in two int arguments.
QueueFunction<void, int, int> * pFuncAdd = new QueueFunction<void, int, int>(
    [](int x, int y) {               // Main function.
        printf("%d + %d = %d\n", x, y, x + y);
    },
    nullptr,                         // No pre function specified (optional, defaults to nullptr).
    nullptr                          // No post function specified (optional, defaults to nullptr).
);
```
2. **Create a work queue** with our queue functions, with a specific number of threads and params.
```c++
// Create our void work queue with 1 thread.
Queue<void> * pQueueVoid = new Queue<void>(pFuncVoid, 1);

// Create our work queue for performing addition with 1 thread.
Queue<void, int, int> * pQueueAdd = new Queue<void, int, int>(pFuncAdd, 1);
```
3. **Dispatch work**, using the same number of params you specified when creating the queue.
```c++
// Dispatch 2 hello world printf's.
pQueueVoid->dispatchWork();
pQueueVoid->dispatchWork();

// Expect to see:
// Hello, world!
// Hello, world!

// If true is provided, this function will block until all work is finished.
pQueueVoid->hasWorkLeft(true);

// --------------------------------------

// Dispatch 3 additions.
pQueueAdd->dispatchWork(1, 2);
pQueueAdd->dispatchWork(3, 4);
pQueueAdd->dispatchWork(5, 6);

// Expect to see:
// 1 + 2 = 3
// 3 + 4 = 7
// 5 + 6 = 11

// If true is provided, this function will block until all work is finished.
pQueueAdd->hasWorkLeft(true);
```
4. **Clean up** after ourselves.
```c++
delete(pQueueVoid);
delete(pQueueAdd);
delete(pFuncVoid);
delete(pFuncAdd);
```

If you only specify a single thread for the queue, the queue will be a serial work queue.

When dispatching work to a queue, your work will be handled by one of the N threads you specified during queue creation, handing off the exact parameters you've passed into the dispatch call, itself.

Simply copy the folder `./src/DispatchCPP` into your headers/include directory, and then include this header-only library with one statement: `#include <DispatchCPP/DispatchCPP.h>`.

# Pre/Post Queue Functions
It is possible to specify pre and/or post functions for your QueueFunction. The pre function is ran before your main function, having the same dispatched params passed into it, allowing one to preprocess/filter dispatches. The post function is ran after your main function, having the main function's return value (if it has one) passed into it.

By default, QueueFunction's have no pre or post functions. A main function is required, however.

If a pre function determines the main function should not be run, the default behavior is for the post function to not be run, either.
> This can be changed, however, by manipulating the `QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED` define in `DispatchCPP/QueueFunction.h` at the top. Even when defined, though, there is no indication given to the post function as to whether the main func was ran or not.

## Pre Functions
Pre functions make it possible to screen/preprocess incoming dispatches, and this function returns a `bool` to indicate whether the queue's main function should be executed or not. All parameters from the main function will be passed into the pre function.

Example:
```c++
// Our queue function to perform addition, however we only want to perform the addition if both numbers are even.
QueueFunction<void, int, int> * pFuncAdd = new QueueFunction<void, int, int>(
    [](int x, int y) {                          // Main function.
        printf("%d + %d = %d\n", x, y, x + y);
    },
    [](int x, int y) {                          // Pre function.
        return(!(x % 2) && !(y % 2));
    },
    nullptr                                     // No post function specified (optional, defaults to nullptr).
);
```

## Post Functions
Post functions make it possible to analyze the return value of your main function. This function does not return anything (void), and accepts either 0 or 1 parameters. If your main function returns void, this function will have no params. If your main function does return a value, this will be the only incoming parameter.

Example:
```c++
// Our queue function to perform addition, however we only want to output the addition if the resulting number is odd.
QueueFunction<int, int, int> * pFuncAdd = new QueueFunction<int, int, int>(
    [](int x, int y) {      // Main function.
        return(x + y);
    },
    nullptr,                // No pre function specified (required, in this case, to specify the post function).
    [](int result) {        // Post function.
        if (result % 2) {
            printf("Result = %d\n", result);
        }
    }
);
``` 

# Full Example 1

Main.cpp:
```c++
#include <stdio.h>
#include "DispatchCPP/DispatchCPP.h"

using namespace DispatchCPP;

int main() {
    // Declare our Queue's function. Specify a pre function to make sure the incoming pResult parameter isn't NULL.
    QueueFunction<void, int, int, int *> * pFuncAdd = new QueueFunction<void, int, int, int *>(
        [](int x, int y, int * pResult) { // Main function.
            *pResult = x + y;
        },
        [](int x, int y, int * pResult) { // Pre function.
            return(pResult != NULL);
        },
        nullptr                           // No post function (optional, defaults to nullptr).
    );

    // Create our Queue using this function, with 2 threads.
    Queue<void, int, int, int *> * pQueue = new Queue<void, int, int, int *>(pFuncAdd, 2);

    // Let's create our data to dispatch in parallel.
    int data[5][3] = {
        { 0, 1, 0 },
        { 2, 3, 0 },
        { 4, 5, 0 },
        { 6, 7, 0 },
        { 8, 9, 0 },
    };

    // Dispatch all our work, now, sending in the first 2 ints as params, and the address of the third to store the result.
    pQueue->dispatchWork(data[0][0], data[0][1], &(data[0][2]));
    pQueue->dispatchWork(data[1][0], data[1][1], &(data[1][2]));
    pQueue->dispatchWork(data[2][0], data[2][1], &(data[2][2]));
    pQueue->dispatchWork(data[3][0], data[3][1], &(data[3][2]));
    pQueue->dispatchWork(data[4][0], data[4][1], &(data[4][2]));

    // Wait for the work to finish, now.
    pQueue->hasWorkLeft(true);

    // Print out all the results.
    printf("[%u] %d + %d = %d\n", 0, data[0][0], data[0][1], data[0][2]);
    printf("[%u] %d + %d = %d\n", 1, data[1][0], data[1][1], data[1][2]);
    printf("[%u] %d + %d = %d\n", 2, data[2][0], data[2][1], data[2][2]);
    printf("[%u] %d + %d = %d\n", 3, data[3][0], data[3][1], data[3][2]);
    printf("[%u] %d + %d = %d\n", 4, data[4][0], data[4][1], data[4][2]);

    // Clean up after ourselves.
    delete(pQueue);
    delete(pFuncAdd);
    return(EXIT_SUCCESS);
}
```

Make sure you have either [installed](#to-install) or copied the [src/DispatchCPP](https://github.com/L-tgray/DispatchCPP/tree/main/src/DispatchCPP) folder into the same directory as this `Main.cpp` file, and compile it with at least c++11 specified:
```
$ g++ -std=c++17 Main.cpp -o Main.out
```

Output:
```
$ ./Main.out
[0] 0 + 1 = 1
[1] 2 + 3 = 5
[2] 4 + 5 = 9
[3] 6 + 7 = 13
[4] 8 + 9 = 17
```

# Full Example 1 (Condensed)

Main.cpp:
```c++
#include <stdio.h>
#include "DispatchCPP/DispatchCPP.h"

using namespace DispatchCPP;

int main() {
    // Create our Queue inline, creating the QueueFunction within our call to the Queue's constructor.
    Queue<void, int, int, int *> * pQueue = new Queue<void, int, int, int *>(
        // Declare our Queue's function. Specify a pre function to make sure the incoming pResult parameter isn't NULL.
        new QueueFunction<void, int, int, int *>(
            [](int x, int y, int * pResult) { // Main function.
                *pResult = x + y;
            },
            [](int x, int y, int * pResult) { // Pre function.
                return(pResult != NULL);
            },
            nullptr                           // No post function (optional, defaults to nullptr).
        ),
        2,   // This queue will have 2 threads.
        true // Passing in true indicates for the Queue to deallocate the `QueueFunction *` passed in during creation.
    );

    // Let's create our data to dispatch in parallel.
    int data[5][3] = {
        { 0, 1, 0 },
        { 2, 3, 0 },
        { 4, 5, 0 },
        { 6, 7, 0 },
        { 8, 9, 0 },
    };

    // Dispatch all our work, now, sending in the first 2 ints as params, and the address of the third to store the result.
    for (unsigned int index = 0; index < 5; ++index) {
        pQueue->dispatchWork(data[index][0], data[index][1], &(data[index][2]));
    }

    // Wait for the work to finish, now.
    pQueue->hasWorkLeft(true);

    // Print out all the results.
    for (unsigned int index = 0; index < 5; ++index) {
        printf("[%u] %d + %d = %d\n", index, data[index][0], data[index][1], data[index][2]);
    }

    // Clean up after ourselves.
    delete(pQueue);
    return(EXIT_SUCCESS);
}
```

Make sure you have either [installed](#to-install) or copied the [src/DispatchCPP](https://github.com/L-tgray/DispatchCPP/tree/main/src/DispatchCPP) folder into the same directory as this `Main.cpp` file, and compile it with at least c++11 specified:
```
$ g++ -std=c++17 Main.cpp -o Main.out
```

Output:
```
$ ./Main.out
[0] 0 + 1 = 1
[1] 2 + 3 = 5
[2] 4 + 5 = 9
[3] 6 + 7 = 13
[4] 8 + 9 = 17
```

# Full Example 2 (Capturing and Using Variables)

Main.cpp:
```c++
#include <stdio.h>
#include "DispatchCPP/DispatchCPP.h"

using namespace DispatchCPP;

int main() {
    // Let's create our data to dispatch in parallel.
    int data[5][3] = {
        { 0, 1, 0 },
        { 2, 3, 0 },
        { 4, 5, 0 },
        { 6, 7, 0 },
        { 8, 9, 0 },
    };

    // Construct our Queue, inlining the creation of the Queue's function.
    Queue<void, int> * pQueue = new Queue<void, int>(
        // Create our QueueFunction, capturing the data array.
        new QueueFunction<void, int>(
            [&data](int index) {                 // Main function.
                data[index][2] = data[index][0] + data[index][1];
            },
            nullptr,                             // No pre function (optional, defaults to nullptr).
            nullptr                              // No post function (optional, defaults to nullptr).
        ),
        2,       // Use 2 threads to dispatch work from this queue.                            
        true     // Deallocate the QueueFunction passed into this Queue constructor.
    );

    // Dispatch all our work, now, sending in each of the indices to compute.
    for (unsigned int index = 0; index < 5; ++index) {
        pQueue->dispatchWork(index);
    }

    // Wait for the work to finish, now.
    pQueue->hasWorkLeft(true);

    // Print out all the results.
    for (unsigned int index = 0; index < 5; ++index) {
        printf("[%u] %d + %d = %d\n", index, data[index][0], data[index][1], data[index][2]);
    }

    // Clean up after ourselves.
    delete(pQueue);
    return(EXIT_SUCCESS);
}
```

Make sure you have either [installed](#to-install) or copied the [src/DispatchCPP](https://github.com/L-tgray/DispatchCPP/tree/main/src/DispatchCPP) folder into the same directory as this `Main.cpp` file, and compile it with at least c++11 specified:
```
$ g++ -std=c++17 Main.cpp -o Main.out
```

Output:
```
$ ./Main.out
[0] 0 + 1 = 1
[1] 2 + 3 = 5
[2] 4 + 5 = 9
[3] 6 + 7 = 13
[4] 8 + 9 = 17
```

# Full Example 3 (Serializing Work Done In Parallel)

Main.cpp:
```c++
#include <stdio.h>
#include "DispatchCPP/DispatchCPP.h"

using namespace DispatchCPP;

int main() {
    // Let's create our data to dispatch in parallel.
    int data[5][3] = {
        { 0, 1, 0 },
        { 2, 3, 0 },
        { 4, 5, 0 },
        { 6, 7, 0 },
        { 8, 9, 0 },
    };

    // Construct our serial Queue, used to serialize calls to printf.
    Queue<void, int> * pQueuePrintf = new Queue<void, int>(
        // Create our QueueFunction, capturing the data array.
        new QueueFunction<void, int>(
            [&data](int index) {                 // Main function.
                printf("[%d] %d + %d = %d\n", index, data[index][0], data[index][1], data[index][2]);
            },
            nullptr,                             // No pre function (optional, defaults to nullptr).
            nullptr                              // No post function (optional, defaults to nullptr).
        ),
        1,       // Use only a single thread to dispatch work from this queue.
        true     // Deallocate the QueueFunction passed into this Queue constructor.
    );

    // Construct our parallel Queue to be used for addition.
    Queue<void, int> * pQueueAdd = new Queue<void, int>(
        // Create our QueueFunction, capturing the data array and the serial Queue.
        new QueueFunction<void, int>(
            [&data, pQueuePrintf](int index) {   // Main function.
                // Perform the addition.
                data[index][2] = data[index][0] + data[index][1];

                // Dispatch the printf to a serialized queue, so only one result is printed at a time.
                pQueuePrintf->dispatchWork(index);
            },
            nullptr,                             // No pre function (optional, defaults to nullptr).
            nullptr                              // No post function (optional, defaults to nullptr).
        ),
        4,       // Use 4 threads to dispatch work from this queue.
        true     // Deallocate the QueueFunction passed into this Queue constructor.
    );

    // Dispatch all our work, now, sending in each of the indices to compute.
    for (unsigned int index = 0; index < 5; ++index) {
        pQueueAdd->dispatchWork(index);
    }

    // Wait for the work to finish, now.
    pQueueAdd->hasWorkLeft(true);    // Wait for all the additions to be dispatched, first.
    pQueuePrintf->hasWorkLeft(true); // Now wait for all the printf's to be dispatched.

    // Clean up after ourselves.
    delete(pQueuePrintf);
    delete(pQueueAdd);
    return(EXIT_SUCCESS);
}
```

Make sure you have either [installed](#to-install) or copied the [src/DispatchCPP](https://github.com/L-tgray/DispatchCPP/tree/main/src/DispatchCPP) folder into the same directory as this `Main.cpp` file, and compile it with at least c++11 specified:
```
$ g++ -std=c++17 Main.cpp -o Main.out
```

Possible output:
```
$ ./Main.out
[0] 0 + 1 = 1
[2] 4 + 5 = 9
[1] 2 + 3 = 5
[4] 8 + 9 = 17
[3] 6 + 7 = 13
```

# Full Example 4 (Thread Init/Close Funcs)

Main.cpp:
```c++
#include <stdio.h>
#include <vector>
#include <map>
#include "DispatchCPP/DispatchCPP.h"

using namespace std;
using namespace DispatchCPP;

int main() {
    // Declare our thread-specific map of data, which gets initialized right after the thread's created.
    map<pthread_t, vector<int>> threadData = map<pthread_t, vector<int>>();

    // Construct our parallel Queue to be used for generating arrays of random numbers.
    Queue<void> * pQueueRand = new Queue<void>(
        // Create our QueueFunction, capturing the data array and the serial Queue.
        new QueueFunction<void>(
            [&threadData]() {                    // Main function.
                // Grab our thread ID. 
                pthread_t tid = pthread_self();

                // Generate a random number, and add it to this thread's vector of rand's.
                threadData[tid].push_back(rand() % 100);
            },  
            nullptr,                             // No pre function (optional, defaults to nullptr).
            nullptr,                             // No post function (optional, defaults to nullptr).
            [&threadData]() {                    // Init function.
                // Grab our thread ID. 
                pthread_t tid = pthread_self();

                // Initialize our thread's vector.
                threadData[tid] = vector<int>();

                // Let the user know what's up. 
                printf("Thread %llu starting...\n", (unsigned long long int) tid);
            },  
            []() {                               // Close function.
                // Grab our thread ID. 
                pthread_t tid = pthread_self();

                // Let the user know what's up. 
                printf("Thread %llu stopping...\n", (unsigned long long int) tid);
            }   
        ),  
        4,    // Use 4 threads to dispatch work from this queue.
        true  // Deallocate the QueueFunction passed into this Queue constructor.
    );  

    // Dispatch our work, informing the thread pool to generate 50 random numbers across the 4 threads.
    for (unsigned int index = 0; index < 50; ++index) {
        pQueueRand->dispatchWork();
    }   

    // Wait for the work to finish, now.
    pQueueRand->hasWorkLeft(true);

    // Iterate over all entries in the map.
    for (auto const& entry : threadData) {
        // Grab each part of the map.
        pthread_t   entryThread = entry.first;
        vector<int> entryVector = entry.second;

        // Print it all out.
        printf("Thread %llu numbers =>", (unsigned long long int) entryThread);
        for (unsigned int index = 0; index < ((unsigned int) entryVector.size()); ++index) {
            printf(" %d", entryVector[index]);
        }   
        printf("\n");
    }   

    // Clean up after ourselves.
    delete(pQueueRand);
    return(EXIT_SUCCESS);
}
```

Make sure you have either [installed](#to-install) or copied the [src/DispatchCPP](https://github.com/L-tgray/DispatchCPP/tree/main/src/DispatchCPP) folder into the same directory as this `Main.cpp` file, and compile it with at least c++11 specified:
```
$ g++ -std=c++17 Main.cpp -o Main.out
```

Possible output:
```
$ ./Main.out
Thread 123145504194560 starting...
Thread 123145504731136 starting...
Thread 123145505267712 starting...
Thread 123145505804288 starting...
Thread 123145504194560 numbers => 7 78 92 16 35 26 79 45 28
Thread 123145504731136 numbers => 49 73 58 30 72 65 42 3 27 40 12 3 57 33 99 10 33
Thread 123145505267712 numbers => 23 9 40 87 69 67 49 21 67 93 36 85 91 94
Thread 123145505804288 numbers => 44 29 9 60 78 97 12 79 72 57
Thread 123145504194560 stopping...
Thread 123145505804288 stopping...
Thread 123145504731136 stopping...
Thread 123145505267712 stopping...
```
