#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>

#include "QueueFunction.h"
#include "QueueThread.h"

// When we wait for threads to wrap up work, we do this in two steps:
//   1. Wait for the deque of work to be empty (doesn't mean all threads have stopped yet, though).
//   2. Wait for all the threads to go idle (means all existing work has been flushed out). Wait a maximum of 5 seconds here.
#define QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US      5 * 1000 * 1000

// The amount of time we wait in the loops for both #1 and #2 above^.
//   100us => ~2.5% CPU usage in blocking call to hasWorkLeft()
//    50us => ~5.0% CPU usage in blocking call to hasWorkLeft()
#define QUEUE_THREAD_POLLING_TIME_US                    50

// This header file uses the standard namespace.
using namespace std;

// Declare the Queue within our DispatchCPP namespace.
namespace DispatchCPP {
    template <class RType, typename ...Args> class Queue {
        private:
            // The number of threads this queue will use to execution our QueueFunction object's invocations.
            unsigned int numThreads;

            // The queue function, itself.
            QueueFunction<typename RValue<RType>::type, Args...> * pQueueFunction;

            // Should we deallocate the QueueFunction space?
            bool deallocateQueueFunc;

            // The vector of functions this thread will be dispatching.
            deque<function<void()>> queueWork;

            // The lock on the queue of work.
            mutex queueWorkLock;

            // The conditional variable on the queue of work.
            condition_variable queueWorkVar;

            // Our vector of threads.
            vector<QueueThread *> allThreads;

            // Initializes all the threads.
            inline void initializeThreads() {
                // Create all of our queue thread objects, now.
                for (unsigned int index = 0; index < this->numThreads; ++index) {
                    this->allThreads.push_back(new QueueThread(&(this->queueWorkLock), &(this->queueWorkVar), &(this->queueWork)));
                }
            };

            // Terminates and cleans up after all threads.
            inline void teardownThreads() {
                // Tell each of the threads to stop.
                for (unsigned int index = 0; index < this->allThreads.size(); ++index) {
                    this->allThreads[index]->keepGoing = false;
                }

                // Tell all of the threads to start tearing down, now.
                this->queueWorkVar.notify_all();

                // Iterate over all our threads, deleting them as we go.
                while (this->allThreads.size() > 0) {
                    QueueThread * pCurrentThread = this->allThreads.back();
                    if (pCurrentThread) {
                        delete(pCurrentThread);
                    }
                    this->allThreads.pop_back();
                }
            };

        public:
            Queue(QueueFunction<typename RValue<RType>::type, Args...> * pNewQueueFunction, unsigned int newNumThreads = 1, bool deallocateQueueFunction = false) {
                // Initialize our class members.
                this->numThreads          = ((newNumThreads != 0) ? newNumThreads : 1);
                this->pQueueFunction      = pNewQueueFunction;
                this->deallocateQueueFunc = deallocateQueueFunction;
                this->queueWork           = deque<function<void()>>();
                this->allThreads          = vector<QueueThread *>();

                // Initialize all our threads, now.
                this->initializeThreads();
            };
            ~Queue() {
                this->queueWorkLock.lock();
                this->queueWork.clear();
                this->queueWorkLock.unlock();

                this->teardownThreads();

                // Should we deallocate our queue function?
                if (this->deallocateQueueFunc && this->pQueueFunction) {
                    delete(this->pQueueFunction);
                }
            };

            // Add some work to the queue, to be executed by the Queue's QueueFunction object.
            void dispatchWork(Args... args) {
                // Declare our new piece of work we'll be adding to the queue's execution.
                function<void()> newWork = [this, args...](void) {
                    if (this->pQueueFunction != nullptr) {
                        this->pQueueFunction->runFunctions(args...);
                    }
                };

                // Append this to our queue of work.
                this->queueWorkLock.lock();
                this->queueWork.push_back(newWork);
                this->queueWorkLock.unlock();
                // this->queueWorkVar.notify_one();
                this->queueWorkVar.notify_all();
            };

            // This function returns whether there's still pending work (or not).
            bool hasWorkLeft(bool blockUntilDone = false) {
                // Declare our return value up front.
                bool returnValue = false;

                // Should we be blocking until all work is finished?
                if (blockUntilDone) {
                    // Wait until there's no work left.
                    while (true) {
                        // Variable to store whether there's work left or not.
                        bool hasWorkLeft = false;

                        // Is there no work left?
                        this->queueWorkLock.lock();
                        hasWorkLeft = (this->queueWork.size() > 0);
                        this->queueWorkLock.unlock();

                        // Do we have any work left?
                        if (!hasWorkLeft) {
                            // We don't! Break from the loop!
                            break;
                        }

#if defined(QUEUE_THREAD_POLLING_TIME_US) && (QUEUE_THREAD_POLLING_TIME_US > 0)
                        // Sleep for a small period of time.
                        usleep(QUEUE_THREAD_POLLING_TIME_US);
#endif // QUEUE_THREAD_POLLING_TIME_US
                    }

#if defined(QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US) && (QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US > 0)
                    // Once there's no work left, wait until each thread is idle.
                    unsigned int totalTimeNotIdleWait = 0;
                    while (true) {
                        // Iterate over all the threads to make sure they're all idle.
                        bool oneThreadNotIdle = false;
                        for (unsigned int threadIndex = 0; threadIndex < ((unsigned int) this->allThreads.size()); ++threadIndex) {
                            if (!(this->allThreads[threadIndex]->isIdle)) {
                                oneThreadNotIdle = true;
                                break;
                            }
                        }

                        // Are all the threads idle?
                        if (!oneThreadNotIdle) {
                            break;
                        }

#if defined(QUEUE_THREAD_POLLING_TIME_US) && (QUEUE_THREAD_POLLING_TIME_US != 0)
                        // Sleep for a small period of time.
                        usleep(QUEUE_THREAD_POLLING_TIME_US);

#if defined(QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US) && (QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US > 0)
                        totalTimeNotIdleWait += QUEUE_THREAD_POLLING_TIME_US;
                        if (totalTimeNotIdleWait > QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US) {
                            break;
                        }
#endif // QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US
#endif // QUEUE_THREAD_POLLING_TIME_US
                    }
#endif // QUEUE_THREAD_MAX_TIME_WAIT_FOR_NOT_IDLE_US

                // We shouldn't block until all work is finished!
                } else {
                    // Acquire the lock on the work queue.
                    this->queueWorkLock.lock();
                    returnValue = (this->queueWork.size() > 0);
                    this->queueWorkLock.unlock();

                    // Is there no more work left in the queue?
                    if (!returnValue) {
                        // We must now make sure all threads are idle, to actually say there is no more work being performed.
                        bool oneThreadNotIdle = false;
                        for (unsigned int threadIndex = 0; threadIndex < ((unsigned int) this->allThreads.size()); ++threadIndex) {
                            if (!(this->allThreads[threadIndex]->isIdle)) {
                                oneThreadNotIdle = true;
                                break;
                            }
                        }

                        // If one thread is not idle, we *do* have work still being performed.
                        returnValue = oneThreadNotIdle;
                    }
                }

                // Return whether there's still work left (or not)!
                return(returnValue);
            }
    };
};

#endif // __QUEUE_H__
