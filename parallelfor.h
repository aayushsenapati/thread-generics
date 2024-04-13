#ifndef FOR_H
#define FOR_H

#include <iostream>
#include <pthread.h>

template <typename Func, typename Arg>
concept VoidFunctionWithIntegralArg = requires(Func f, Arg a) {
    {
        f(a)
    } -> std::same_as<void>;
    requires std::is_integral_v<Arg>;
};

template <typename Func>
concept SuitableFunction =
    VoidFunctionWithIntegralArg<Func, int> ||
    VoidFunctionWithIntegralArg<Func, long> ||
    VoidFunctionWithIntegralArg<Func, long long> ||
    VoidFunctionWithIntegralArg<Func, short>;

// Struct for passing arguments to thread function
template <typename Func>
struct ThreadArgs
{
    int start;
    int end;
    Func func;
};

// Thread function
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

// Parallel For Loop implementation using Pthreads and Function Pointers
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
            thread_args[i] = new ThreadArgs<Func>{start + i * step, start + (i + 1) * step, func};
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
