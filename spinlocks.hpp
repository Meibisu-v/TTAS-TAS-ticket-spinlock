#pragma once

#include <atomic>
#include <stdint.h>
#include <assert.h>
#include <emmintrin.h>
#include <sched.h>
#include <time.h>

// Base class
class spinlock
{
public:

    virtual void lock() = 0;
    virtual void unlock() = 0;

};

class spinlock_TAS: public spinlock
{
private:

    std::atomic_uint8_t m_spin;

public:

    spinlock_TAS(): m_spin(0) {};
    ~spinlock_TAS() {assert(m_spin.load() == 0);};

    void lock() override;
    void unlock() override;

};

class spinlock_TTAS: public spinlock
{
private:

    std::atomic_uint8_t m_spin;
    uint thread_count;

public:

    spinlock_TTAS(uint thread_count): m_spin(0), thread_count(thread_count) {};
    ~spinlock_TTAS() {assert(m_spin.load() == 0);};

    void lock() override;
    void unlock() override;

};

class ticket_lock: public spinlock
{
private:

    std::atomic_size_t now_serving;
    std::atomic_size_t next_ticket;

public:

    ticket_lock(): now_serving(0), next_ticket(0) {};
    ~ticket_lock() {assert(now_serving.load() == next_ticket.load());};

    void lock() override;
    void unlock() override;
};
