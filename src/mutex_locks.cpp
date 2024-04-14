#ifndef MUTEX_LOCKS_CPP
#define MUTEX_LOCKS_CPP
#include <iostream>
#include <pthread.h>

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
        mutex.unlock(); // Unlock the mutex
    }
};

// Shared resource
int shared_resource = 0;

// Mutex for protecting shared resource
Mutex mutex;

// Function to be executed by multiple threads
void* thread_func(void* arg) {
    for (int i = 0; i < 10000; ++i) {
        Lock lock(mutex); // Lock the mutex

        // Critical section: Accessing and modifying shared resource
        shared_resource++;
    }
    return nullptr;
}

int main() {
    const int num_threads = 4;
    pthread_t threads[num_threads];

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, thread_func, nullptr);
    }

    // Join threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Print shared resource value
    std::cout << "Shared resource value: " << shared_resource << std::endl;

    return 0;
}

#endif