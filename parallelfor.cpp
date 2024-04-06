#include <iostream>
#include <pthread.h>

// Struct for passing arguments to thread function
struct ThreadArgs {
    int start;
    int end;
    void (*func)(int);
};

// Thread function
void* thread_func(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->start; i < args->end; ++i) {
        args->func(i);
    }
    return nullptr;
}

// Parallel For Loop implementation using Pthreads and Function Pointers
void parallel_for_pthreads(int start, int end, void (*func)(int), int num_threads = 4) {
    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];
    int step = (end - start) / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        thread_args[i] = {start + i * step, start + (i + 1) * step, func};
        pthread_create(&threads[i], nullptr, thread_func, &thread_args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
}

// Example usage
void example_task(int i) {
    std::cout << "Task " << i << " finished by thread " << pthread_self() << std::endl;
}

int main() {
    parallel_for_pthreads(0, 10, example_task);
    return 0;
}
