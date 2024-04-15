# Threading API

## Overview

This project is a lightweight threading library designed to simplify the process of creating, managing, and synchronizing threads in POSIX-compliant systems. It offers a user-friendly interface with a set of predefined methods that abstract away the complexities of working directly with POSIX threads. The library is structured around several core components, including a thread pool, parallel for loop implementation, mutex locks, thread local storage, and utility functions for tasks such as index sequences and tuple implementations.

## Features

### Thread Pool

The thread pool is a group of pre-initialized threads that stand ready to execute tasks. This allows the program to avoid the overhead of creating a new thread for each task. Tasks are represented by a function and its arguments, stored in a queue, and executed by one of the threads in the pool when it becomes available.

### Parallel For Loop

The parallel for loop implementation allows for the execution of a function across a range of integers in parallel, using multiple threads. This is particularly useful for tasks that can be executed independently and in parallel, such as processing elements of an array or performing computations on a large dataset.

### Mutex Locks

Mutex locks are used to ensure that only one thread can access a shared resource at a time, preventing race conditions and ensuring data integrity. The library provides a simple interface for creating and managing mutex locks, as well as a lock class for automatic locking and unlocking.

### Thread Local Storage

Thread local storage allows each thread to have its own instance of a variable, which can be useful for storing thread-specific data. The library provides a simple interface for creating and managing thread local storage.

### Utility Functions

The library includes utility functions for tasks such as index sequences and tuple implementations, which are used internally to implement some of the core features.

## Usage

### Thread Pool

To use the thread pool, create an instance of the `ThreadPool` class with the desired number of threads. Then, enqueue tasks using the `enqueue` method, which takes a function and its arguments. The thread pool will execute these tasks in parallel using its worker threads.

```c++
Threading::ThreadPool<4> pool;
pool.enqueue(task1);
pool.enqueue(task2, 42, 43, std::string("hello"));
```


### Parallel For Loop

To use the parallel for loop, call the `parallel_for_pthreads` function with a range, a function to run on each integer in the range, and the number of threads to use.

```c++
Threading::parallel_for_pthreads(0, 10, [](int i) {
    std::cout << "Task " << i << " finished by thread " << pthread_self() << std::endl;
});
```



### Mutex Locks

To use mutex locks, create an instance of the `Mutex` class and use the `lock` and `unlock` methods to control access to shared resources.


```c++
Threading::Mutex mtx;
mtx.lock();
// Critical section
mtx.unlock();
```


### Thread Local Storage

To use thread local storage, create an instance of the `ThreadLocalStorage` class and use the `set` and `get` methods to store and retrieve thread-specific data.

```c++
Threading::ThreadLocalStorage<int> tls;
tls.set(42);
int value = tls.get();
```

## Contributors

- Aayush Senapati
- Ananya Mahishi

## Known Bugs

- No error handling for `pthread_create` in some files.
- No mutex locks for some `std::cout` functions leading to occasional garbled output.

## Limitations

- Yet to implement nuanced functionalities (additional features to improve ease of use of the API).
- Can prevent unnecessary copies by using move semantics.
- Include usage information for API features.
- Haven’t performance benchmarked our API with other existing well-known APIs.