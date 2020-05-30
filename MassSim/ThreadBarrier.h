#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

/**
 * Also known as a rendezvous, a barrier is a synchronization point
 * between multiple threads.  The barrier is configured for a
 * particular number of threads (n), and as threads reach the barrier
 * they must wait until all n threads have arrived.  Once the n-th
 * thread has reached the barrier, all the waiting threads can
 * proceed, and the barrier is reset.
 */
class ThreadBarrier
{
public:
    /**
     * Construct a barrier for count threads.
     */
    ThreadBarrier(size_t count);

    /**
     * Block until count threads have called wait() on this.  When the
     * count-th thread calls wait(), the barrier is reset and all
     * waiting threads are unblocked.
     *
     * @param rel_time Relative expiration time.
     *
     * @return True if all threads reached the barrier; false if we timed out.
     */
    bool wait_for(std::chrono::nanoseconds const & rel_time);

private:
    /**
     * Protects data shared between threads.
     */
    std::mutex _mutex;

    /**
     * Used to block and signal the waiting threads.
     */
    std::condition_variable _condition;

    /**
     * This is the number that was passed to the constructor.  When we
     * reset after waking up the threads, we set _num_active to this.
     */
    size_t const _count;

    /**
     * Keeps track of how many threads haven't gotten to wait yet.
     */
    size_t _num_active;

    /**
     * Increments every time we wake up the threads.  This lets us
     * avoid race conditions where a thread checks the _num_active
     * when it wakes up after it has been reset.
     */
    size_t _generation;
};
