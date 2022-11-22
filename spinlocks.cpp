#include "spinlocks.hpp"
#include <unistd.h>
#include <algorithm>
#include <thread>

void spinlock_TAS::lock()
{
    uint8_t expected_zero;
    size_t curr_attempt = 0;
    do
    {
        curr_attempt++;

        if (curr_attempt != 1) 
        {
            sched_yield();
        }
        expected_zero = 0;
    } while(!m_spin.compare_exchange_weak(expected_zero, 1, std::memory_order_acquire));
}

void spinlock_TAS::unlock()
{
    m_spin.store(0, std::memory_order_release);
}

void spinlock_TTAS::lock()
{
    // size_t max_time = 6553;
    uint8_t expected_zero;
    do
    {
        // Initialize time for sleeping
        size_t timer = 1;
        // While spinlock is locked (m_spin == 1)
        while(m_spin.load(std::memory_order_acquire)) {
            // If there are not so many threads in queue, 
            // so context change after yield operation will last too long, 
            // sleep for an exponential time
            if (thread_count > sysconf(_SC_NPROCESSORS_ONLN)*50) {
                sleep(timer);
                timer = timer << 1;
                // timer = std::min(max_time, timer<<1);
            } else {
                __asm volatile ("pause" ::: "memory");
                // sched_yield();
            }
        }
        expected_zero = 0;
    } while(!m_spin.compare_exchange_weak(expected_zero, 1));
}

void spinlock_TTAS::unlock()
{
    m_spin.store(0, std::memory_order_release);
}

void ticket_lock::lock()
{
    size_t curr_attempt = 0;
    const auto ticket = next_ticket.fetch_add(1, std::memory_order_relaxed);
    while(ticket != now_serving.load(std::memory_order_acquire))
    {
        curr_attempt++;
        if (curr_attempt != 0)
            sched_yield();
    }
}

void ticket_lock::unlock()
{
    const auto curr = now_serving.load(std::memory_order_acquire) + 1;
    now_serving.store(curr, std::memory_order_release);
}
