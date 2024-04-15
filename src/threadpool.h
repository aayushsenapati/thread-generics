#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <stdexcept>
#include <iostream>
#include <concepts>
#include "utilities.h"

// Concept for checking if a function is callable with the given arguments and returns void.
template <typename Func, typename... Args>
concept Callable = requires(Func f, Args... args) {
    {
        f(args...)
    } -> std::same_as<void>;
};

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Abstract base class for tasks. This allows us to store different types of tasks
 * in the same queue in our ThreadPool class.
 */
class AbstractTask
{
public:
    virtual ~AbstractTask() {}
    virtual void execute() = 0;
};

/*
 * Concrete task implementation. This class takes a function and its arguments,
 * stores them, and provides a method to execute the function with the arguments.
 */
template <typename Func, typename... Args>
class ConcreteTask : public AbstractTask
{
public:
    ConcreteTask(Func f, Args... args)
        : func(f), arguments(new Tuple<Args...>(args...)) {}

    ~ConcreteTask()
    {
        delete arguments;
    }

    void execute() override
    {
        callWithArguments(func, arguments, make_index_sequence<sizeof...(Args)>{});
    }

private:
    Func func;
    Tuple<Args...> *arguments;

    template <typename F, typename... ArgsT, int... Is>
    void callWithArguments(F func, Tuple<ArgsT...> *t, index_sequence<Is...>)
    {
        // forcefully inserted lambda function as I didnt have any use for it :)
        auto lambda = []<typename FuncLambda, typename... ArgL, int... Indices>(FuncLambda f, Tuple<ArgL...> *t, index_sequence<Indices...>)
        {
            f(t->template get<Indices>()...);
        };
        lambda(func, t, index_sequence<Is...>{});
    }
};

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

namespace Threading
{
    /*
     * Thread pool class. A thread pool is a group of pre-initialized threads that
     * stand ready to execute tasks. This allows the program to avoid the overhead of
     * creating a new thread for each task.
     */
    template <size_t N>
    class ThreadPool
    {
        friend class Master;

    private:
        pthread_t workers[N];
        Queue<AbstractTask> tasks;
        pthread_mutex_t queue_bool_lock;
        pthread_cond_t cond;
        bool stop;
        inline static size_t task_exec_count = 0;
        inline static pthread_mutex_t task_executing_lock = PTHREAD_MUTEX_INITIALIZER;

        /*
         * Worker function for the threads in the pool. This function is static because
         * it's used as the start routine for pthread_create, which requires a function
         * with C linkage.
         */
        static void *worker(void *arg)
        {
            ThreadPool *pool = static_cast<ThreadPool *>(arg);
            while (true)
            {
                pthread_mutex_lock(&(pool->queue_bool_lock)); // lock to access queue
                while (!(pool->stop) && pool->tasks.empty())
                {
                    pthread_cond_wait(&(pool->cond), &(pool->queue_bool_lock)); // switches off the thread(until signal) and unlocks the mutex
                }
                if ((pool->stop) && pool->tasks.empty())
                {
                    pthread_mutex_unlock(&(pool->queue_bool_lock));
                    break;
                }
                auto frontTask = pool->tasks.front();
                pool->tasks.pop();
                pthread_mutex_unlock(&(pool->queue_bool_lock));

                pthread_mutex_lock(&task_executing_lock); // Lock before incrementing
                task_exec_count++;
                pthread_mutex_unlock(&task_executing_lock); // Unlock after incrementing

                frontTask->execute();

                pthread_mutex_lock(&task_executing_lock); // Lock before decrementing
                task_exec_count--;
                pthread_mutex_unlock(&task_executing_lock); // Unlock after decrementing

                delete frontTask; // delete task after execution
            }
            pthread_exit(NULL);
            return NULL;
        }

    public:
        ThreadPool() : stop(false)
        {
            // initialize mutexes and condition variables
            pthread_cond_init(&cond, NULL);
            pthread_mutex_init(&queue_bool_lock, NULL);
            for (size_t i = 0; i < N; ++i)
            {
                if (pthread_create(&workers[i], NULL, worker, this) != 0)
                {
                    throw std::runtime_error("Failed to create thread");
                }
            }
        }

        ~ThreadPool()
        {
            pthread_mutex_lock(&queue_bool_lock);
            stop = true;
            pthread_mutex_unlock(&queue_bool_lock);
            pthread_cond_broadcast(&cond);
            for (size_t i = 0; i < N; ++i)
            {
                pthread_join(workers[i], NULL);
            }
            pthread_mutex_destroy(&queue_bool_lock);
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&task_executing_lock);
        }

        /*
         * Enqueue a new task in the thread pool. The task is represented by a function
         * and its arguments. The function will be executed by one of the threads in the
         * pool when it becomes available.
         */
        template <typename Func, typename... Args>
            requires Callable<Func, Args...>
        void enqueue(Func f, Args... args)
        {
            if (stop)
            {
                throw std::runtime_error("Thread pool is stopping");
            }

            // Fold expression to check if all arguments are copy-constructible
            static_assert((std::is_copy_constructible_v<Args> && ...), "All arguments must be copy-constructible");

            auto task = new ConcreteTask<Func, Args...>(f, args...);
            pthread_mutex_lock(&queue_bool_lock);
            tasks.push(task);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&queue_bool_lock);
        }
    };
}

#endif // UTILITIES_H
