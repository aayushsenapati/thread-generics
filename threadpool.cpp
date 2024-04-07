#include <pthread.h>
#include <stdexcept>
#include <iostream>

// Index sequence implementation
template <int... Is>
struct index_sequence
{
};

template <int N, int... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...>
{
};

template <int... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...>
{
};

// Tuple-like structure using template recursion
template <typename... Args>
struct Tuple
{
};

// Forward declare the empty Tuple specialization
template <>
struct Tuple<>
{
    Tuple() {}
};

template <typename T, typename... Args>
struct Tuple<T, Args...> : public Tuple<Args...>
{
    T head;
    Tuple<Args...> tail;

    Tuple(T h, Args... args) : head(h), tail(args...) {}
    Tuple() : head(T()), tail() {}

    template <int N>
    auto get() -> typename std::conditional<N == 0, T &, decltype(tail.template get<N - 1>())>::type
    {
        if constexpr (N == 0)
            return head;
        else
            return tail.template get<N - 1>();
    }
};

template <typename T>
struct Tuple<T> : public Tuple<>
{
    T head;

    Tuple(T h) : head(h) {}
    Tuple() : head(T()) {}

    template <int N>
    auto get() -> typename std::conditional<N == 0, T &, void>::type
    {
        static_assert(N == 0, "Index out of bounds in Tuple::get");
        return head;
    }
};

// Abstract base class for tasks
class AbstractTask
{
public:
    virtual ~AbstractTask() {}
    virtual void execute() = 0;
};

// Concrete task implementation
template <typename... Args>
class ConcreteTask : public AbstractTask
{
public:
    using FunctionPtr = void (*)(Args...);

    ConcreteTask(FunctionPtr f, Args... args)
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
    FunctionPtr func;
    Tuple<Args...> *arguments;

    template <typename F, typename... ArgsT, int... Is>
    void callWithArguments(F func, Tuple<ArgsT...> *t, index_sequence<Is...>)
    {
        func(t->template get<Is>()...);
    }
};

class Queue
{
private:
    struct Node
    {
        AbstractTask *task;
        Node *next;
    };
    Node *head;
    Node *tail;

public:
    Queue() : head(nullptr), tail(nullptr) {}

    void push(AbstractTask *task)
    {
        Node *newNode = new Node{task, nullptr};
        if (!head)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    bool empty()
    {
        return head == nullptr;
    }

    AbstractTask *front()
    {
        return head ? head->task : nullptr;
    }

    void pop()
    {
        if (!head)
            return;
        Node *temp = head;
        head = head->next;
        // delete temp->task; // delete task
        delete temp;
    }
};

class ThreadPool
{
private:
    pthread_t *workers;
    size_t numThreads;
    Queue tasks;
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
    ThreadPool(size_t threads) : stop(false), numThreads(threads)
    {
        workers = new pthread_t[threads];
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
        for (size_t i = 0; i < threads; ++i)
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
        for (size_t i = 0; i < numThreads; ++i)
        {
            pthread_join(workers[i], NULL);
        }
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
        delete[] workers;
    }

    // Overload for tasks without arguments
    template <typename Func>
    void enqueue(Func f)
    {
        pthread_mutex_lock(&lock);
        tasks.push(new ConcreteTask<>(f));
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }

    // Existing template for tasks with arguments
    template <typename... Args>
    void enqueue(void (*f)(Args...), Args... args)
    {
        pthread_mutex_lock(&lock);
        tasks.push(new ConcreteTask<Args...>(f, args...));
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
};

void task1()
{
    std::cout << "Task 1 is running" << std::endl;
}

void task2(int x, int y,std::string z)
{
    std::cout << "Task 2 is running with arguments " << x << " and " << y<<"and"<<z << std::endl;
}

int main()
{
    try
    {
        ThreadPool pool(2);
        pool.enqueue(task1);
        pool.enqueue(task2, 42, 43,std::string("hello"));

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}