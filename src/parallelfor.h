#ifndef FOR_H
#define FOR_H

#include <iostream>
#include <pthread.h>

/*
* Concept for checking if a function takes an integral type and returns void.
* This is used to ensure that the function passed to parallel_for_pthreads meets these requirements.
*/

template <typename Func, typename Arg>
concept VoidFunctionWithIntegralArg = requires(Func f, Arg a) {
    {
        f(a)
    } -> std::same_as<void>;
    requires std::is_integral_v<Arg>;
};


/*
* Concept for checking if a function is suitable for parallel_for_pthreads.
* A suitable function is one that takes an integral type and returns void.
*/

template <typename Func>
concept SuitableFunction =
    VoidFunctionWithIntegralArg<Func, int> ||
    VoidFunctionWithIntegralArg<Func, long> ||
    VoidFunctionWithIntegralArg<Func, long long> ||
    VoidFunctionWithIntegralArg<Func, short>;

/*
* Struct for passing arguments to the thread function.
* This is necessary because pthread_create can only pass one argument to the thread function,
* so we need to pack all our arguments into a single struct.
*/
template <typename Func>
struct ThreadArgs
{
    int start;
    int end;
    Func func;
};




/*
* Function that each thread will run. It takes a ThreadArgs struct, unpacks the arguments,
* and calls the function for each integer in the range [start, end).
*/
template <typename Func>
void *thread_func(void *arg)
{
    ThreadArgs<Func> *args = static_cast<ThreadArgs<Func> *>(arg);
    for (int i = args->start; i < args->end; ++i)
    {
        args->func(i);
    }
    return nullptr;
}

/*
* Parallel For Loop implementation using Pthreads and Function Pointers.
* A parallel for loop is a loop where each iteration is run in parallel on a separate thread.
* This function takes a range [start, end), a function to run on each integer in the range,
* and the number of threads to use. It creates the threads, starts them, and then waits for them to finish.
*/
namespace Threading
{
    template <typename Func>
        requires SuitableFunction<Func>
    void parallel_for_pthreads(int start, int end, Func func, int num_threads = 4)
    {
        pthread_t threads[num_threads];
        ThreadArgs<Func> *thread_args[num_threads];
        int step = (end - start) / num_threads;

        for (int i = 0; i < num_threads; ++i)
        {
            int thread_start = start + i * step;
            int thread_end = (i == num_threads - 1) ? end : thread_start + step;
            thread_args[i] = new ThreadArgs<Func>{thread_start, thread_end, func};
            pthread_create(&threads[i], nullptr, thread_func<Func>, thread_args[i]);
        }

        for (int i = 0; i < num_threads; ++i)
        {
            pthread_join(threads[i], nullptr);
            delete thread_args[i]; // delete the dynamically allocated memory
        }
    }
}

#endif
