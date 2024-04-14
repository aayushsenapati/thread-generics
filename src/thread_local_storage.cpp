#ifndef THREAD_LOCAL_CPP
#define THREAD_LOCAL_CPP
#include <iostream>
#include <pthread.h>


namespace Threading{
    
// Thread Local Storage class

template <typename T>
class ThreadLocalStorage {
private:
    pthread_key_t key; // Key for accessing thread-specific data

    // Destructor function for the thread-specific data
    static void destructor(void* data) {
        delete static_cast<T*>(data); // Cleanup the data
    }

public:
    ThreadLocalStorage() {
        pthread_key_create(&key, destructor); // Create the thread-specific key
    }

    ~ThreadLocalStorage() {
        pthread_key_delete(key); // Delete the thread-specific key
    }

    // Set thread-local variable
    void set(const T& value) {
        T* ptr = new T(value); // Allocate memory for a new instance of T
        pthread_setspecific(key, ptr); // Set the thread-specific value
    }

    // Get thread-local variable
    T get() const {
        void* ptr = pthread_getspecific(key); // Get the thread-specific value
        return (ptr != nullptr) ? *static_cast<T*>(ptr) : T(); // Return the value (or default value if null)
    }
};
}

#endif