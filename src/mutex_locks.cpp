#ifndef MUTEX_LOCKS_CPP
#define MUTEX_LOCKS_CPP
#include <iostream>
#include <pthread.h>

namespace Threading{
// Mutex class
class Mutex {
private:
    pthread_mutex_t mutex;

public:
    Mutex() {
        pthread_mutex_init(&mutex, nullptr); // Initialize the mutex
    }

    ~Mutex() {
        pthread_mutex_destroy(&mutex); // Destroy the mutex
    }

    void lock() {
        pthread_mutex_lock(&mutex); // Lock the mutex
    }

    void unlock() {
        pthread_mutex_unlock(&mutex); // Unlock the mutex
    }
};

// Lock class for automatic locking and unlocking
class Lock {
private:
    Mutex& mutex;

public:
    Lock(Mutex& mtx) : mutex(mtx) {
        mutex.lock(); // Lock the mutex
    }

    ~Lock() {
        std::cout << "Unlocking mutex" << std::endl;
        mutex.unlock(); // Unlock the mutex
    }
};

}

#endif