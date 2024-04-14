
#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <stdexcept>
#include <iostream>
#include <concepts>
#include "utilities.h"



//class Threading::Master;


// Define a concept for callable functions
template <typename Func, typename... Args>
concept Callable = requires(Func f, Args... args) {
    {
        f(args...)
    } -> std::same_as<void>;
};

// Abstract base class for tasks
class AbstractTask
{
public:
    virtual ~AbstractTask() {}
    virtual void execute() = 0;
};

// Concrete task implementation
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
        auto lambda = []<typename FuncLambda, typename... ArgL, int... Indices>(FuncLambda f, Tuple<ArgL...> *t, index_sequence<Indices...>)
        {
            f(t->template get<Indices>()...);
        };
        lambda(func, t, index_sequence<Is...>{});
    }
};

namespace Threading
{
    // Thread pool class
    template <size_t N>
    class ThreadPool
    {
        friend class Master;

    private:
        pthread_t workers[N];
        Queue<AbstractTask> tasks;
        pthread_mutex_t lock;
        pthread_cond_t cond;
        bool stop;

        static void *worker(void *arg)
        {
            ThreadPool *pool = static_cast<ThreadPool *>(arg);
            while (true)
            {
                pthread_mutex_lock(&(pool->lock));
                while (!(pool->stop) && pool->tasks.empty())
                {
                    pthread_cond_wait(&(pool->cond), &(pool->lock));
                }
                if ((pool->stop) && pool->tasks.empty())
                {
                    pthread_mutex_unlock(&(pool->lock));
                    break;
                }
                auto frontTask = pool->tasks.front();
                pool->tasks.pop();
                pthread_mutex_unlock(&(pool->lock));
                frontTask->execute();
                delete frontTask; // delete task after execution
            }
            pthread_exit(NULL);
            return NULL;
        }

    public:
        ThreadPool() : stop(false)
        {
            pthread_cond_init(&cond, NULL);
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
            pthread_mutex_lock(&lock);
            stop = true;
            pthread_mutex_unlock(&lock);
            pthread_cond_broadcast(&cond);
            for (size_t i = 0; i < N; ++i)
            {
                pthread_join(workers[i], NULL);
            }
            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&cond);
        }

        // Existing template for tasks with arguments
        template <typename Func, typename... Args>
            requires Callable<Func, Args...>
        void enqueue(Func f, Args... args)
        {
            auto task = new ConcreteTask<Func, Args...>(f, args...);
            pthread_mutex_lock(&lock);
            tasks.push(task);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
        }
    };
}

#endif // UTILITIES_H