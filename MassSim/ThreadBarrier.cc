#include "ThreadBarrier.h"

ThreadBarrier::ThreadBarrier(size_t count):
    _count(count),
    _num_active(count),
    _generation(0)
{
}

bool ThreadBarrier::wait_for(std::chrono::nanoseconds const & rel_time)
{
    if (_count < 2)
    {
        return true;
    }

    std::unique_lock<std::mutex> guard(_mutex);
    size_t this_generation = _generation;
    if (--_num_active == 0)
    {
        /*
         * Increment the generation and reset the count.
         */
        _generation++;
        _num_active = _count;

        _condition.notify_all();
    }
    else
    {
        while (this_generation == _generation)
        {
            if (_condition.wait_for(guard, rel_time) == std::cv_status::timeout)
            {
                return false;
            }
        }
    }
    return true;
}
