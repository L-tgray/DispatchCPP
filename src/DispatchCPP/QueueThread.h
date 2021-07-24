#ifndef __QUEUE_THREAD_H__
#define __QUEUE_THREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

// Defines
#define THREAD_INIT_LOOP_WAIT_TIME_US         1
#define THREAD_INIT_LOOP_MAX_WAIT_TIME_US     500
#define THREAD_TEARDOWN_LOOP_WAIT_TIME_US     1
#define THREAD_TEARDOWN_LOOP_MAX_WAIT_TIME_US 5000
#define THREAD_TEARDOWN_LOOP_ENABLE_NOTIFY

// This header file uses the standard namespace.
using namespace std;

// Forward declaration of our class within the DispatchCPP namespace.
namespace DispatchCPP { class QueueThread; };

// Declare the QueueThread within our DispatchCPP namespace.
namespace DispatchCPP {
    class QueueThread {
        private:
            // Our thread object, itself.
            thread * pThread;

            // Initializes the thread.
            inline void initializeThread() {
                // Make sure we're not already running.
                if (!this->isRunning) {
                    // Start the thread.
                    this->keepGoing = true;
                    this->pThread   = new thread(this->queueThreadFunc, this);

                    // Did we successfully create our thread?
                    bool hitError = false;
                    if (this->pThread) {
#if defined(THREAD_INIT_LOOP_WAIT_TIME_US) && (THREAD_INIT_LOOP_WAIT_TIME_US > 0)
                        // Wait once, initially.
                        usleep(THREAD_INIT_LOOP_WAIT_TIME_US);
#endif // THREAD_INIT_LOOP_WAIT_TIME_US

                        // Wait until our thread's started up.
                        unsigned int totalWaitTimeUS = 0;
                        while (!this->isRunning) {
#if defined(THREAD_INIT_LOOP_WAIT_TIME_US) && (THREAD_INIT_LOOP_WAIT_TIME_US > 0)
                            usleep(THREAD_INIT_LOOP_WAIT_TIME_US);
                            totalWaitTimeUS += THREAD_INIT_LOOP_WAIT_TIME_US;
                            if (totalWaitTimeUS > THREAD_INIT_LOOP_MAX_WAIT_TIME_US) {
                                hitError = true;
                                break;
                            }
#endif // THREAD_INIT_LOOP_WAIT_TIME_US
                        }

                    // Failed to create our thread object!
                    } else {
                        hitError = true;
                    }

                    // Did we hit an error?
                    if (hitError) {
                        // We did! Must cleanup after ourselves, now.
                        this->teardownThread();
                    }
                }
            };

            // Stops and cleans up after the thread, blocking until the thread stops and we're able to clean up after it.
            inline void teardownThread() {
                // Is the thread running?
                if (this->isRunning) {
                    // Tell the thread to stop running.
                    this->keepGoing = false;

#if defined(THREAD_INIT_LOOP_WAIT_TIME_US) && (THREAD_INIT_LOOP_WAIT_TIME_US > 0)
                    // Wait once, initially.
                    usleep(THREAD_INIT_LOOP_WAIT_TIME_US);
#endif // THREAD_INIT_LOOP_WAIT_TIME_US

                    this->pWorkVar->notify_all();

#if defined(THREAD_INIT_LOOP_WAIT_TIME_US) && (THREAD_INIT_LOOP_WAIT_TIME_US > 0)
                    // Wait once, again, after we notified all the waiting threads.
                    usleep(THREAD_INIT_LOOP_WAIT_TIME_US);
#endif // THREAD_INIT_LOOP_WAIT_TIME_US

                    // Wait for the thread to stop running.
                    unsigned int totalWaitTimeUS = 0;
                    while (this->isRunning) {
#if defined(THREAD_TEARDOWN_LOOP_WAIT_TIME_US) && (THREAD_TEARDOWN_LOOP_WAIT_TIME_US > 0)
                        usleep(THREAD_TEARDOWN_LOOP_WAIT_TIME_US);
                        totalWaitTimeUS += THREAD_TEARDOWN_LOOP_WAIT_TIME_US;
                        if (totalWaitTimeUS > THREAD_TEARDOWN_LOOP_MAX_WAIT_TIME_US) {
                            break;
                        }
#endif // THREAD_TEARDOWN_LOOP_WAIT_TIME_US

                        // Continue to notify the threads to help ensure our thread knows it needs to end.
                        this->pWorkVar->notify_all();
                    }
                }

                // Do we have a valid thread object to deallocate?
                if (this->pThread) {
                    // Join the thread back. Blocks until the thread has finished execution.
                    this->pThread->join();

                    // Delete the thread object, now.
                    delete(this->pThread);
                }
            };

        public:
            // Flags we use for interacting with the thread's execution.
            volatile bool keepGoing;
            volatile bool isRunning;
            volatile bool isIdle;

            // A pointer to our mutex protecting the deque of work.
            mutex * pWorkLock;

            // A pointer to our semaphore used to efficiently wake for work.
            condition_variable * pWorkVar;

            // A pointer to our deque of work.
            deque<function<void()>> * pWorkQueue;

            // Constructor.
            inline QueueThread(mutex * pNewWorkLock, condition_variable * pNewWorkVar, deque<function<void()>> * pNewWorkQueue) {
                // Initialize our class members.
                this->keepGoing  = true;
                this->isRunning  = false;
                this->isIdle     = false;
                this->pWorkLock  = pNewWorkLock;
                this->pWorkVar   = pNewWorkVar;
                this->pWorkQueue = pNewWorkQueue;

                // Initialize our thread, now.
                this->initializeThread();
            };

            // Destructor.
            inline ~QueueThread() {
                // Tear down our thread, blocking until it joins back.
                this->teardownThread();
            };

        private:
            function<void(DispatchCPP::QueueThread *)> queueThreadFunc = [](DispatchCPP::QueueThread * pThis) {
                // Indicate that we're running, now.
                pThis->isRunning = true;

                // Keep going until we're told to stop.
                while (pThis->keepGoing) {
                    // Indicate that we're currently idle.
                    pThis->isIdle = true;

                    // Wait until there's work to do.
                    unique_lock<mutex> tempLock(*(pThis->pWorkLock));
                    pThis->pWorkVar->wait(tempLock, [pThis] {
                        bool hasNoWork         = (pThis->pWorkQueue->size() == 0);
                        bool shouldKeepWaiting = (pThis->keepGoing && hasNoWork);
                        return(!shouldKeepWaiting);
                    });

                    // Indicate that we're no longer idle.
                    pThis->isIdle = false;

                    // Are we being told to stop working? (after being woken up)
                    if (!pThis->keepGoing) {
                        // Release the lock we have on the work queue and break.
                        tempLock.unlock();
                        break;
                    }

                    // There's work to do! Grab the lock on the array of work, now.
                    function<void()> newWork = nullptr;

                    // Do we have any work?
                    if (pThis->pWorkQueue->size() > 0) {
                        newWork = pThis->pWorkQueue->front();
                        pThis->pWorkQueue->pop_front();
                    }

                    // Release the lock we have on the work queue.
                    tempLock.unlock();

                    // Did we get some work to do?
                    if (newWork != nullptr) {
                        newWork();
                    }

                    // Indicate that we're idle, now.
                    pThis->isIdle = true;
                }

                // Indicate that we're no longer running, and that we're idle.
                pThis->isRunning = false;
                pThis->isIdle    = true;
            };
    };
};

#endif // __QUEUE_THREAD_H__