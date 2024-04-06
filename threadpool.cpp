#include <pthread.h>
#include <stdexcept>
#include <iostream>

// Tuple-like structure using template recursion
template <typename... Args>
struct Tuple {};

template <typename T, typename... Args>
struct Tuple<T, Args...> {
    T head;
    Tuple<Args...> tail;
};

template <>
struct Tuple<> {};

class Queue {
private:
    struct Node {
        void (*data)();
        void* args;
        Node* next;
    };

    Node* head;
    Node* tail;

public:
    Queue() : head(nullptr), tail(nullptr) {}

    void push(void (*func)(), void* arguments) {
        Node* newNode = new Node{func, arguments, nullptr};
        if (!head) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    bool empty() {
        return head == nullptr;
    }

    void pop() {
        if (!head)
            return;
        Node* temp = head;
        head = head->next;
        delete temp;
    }

    Node* front() {
        return head;
    }
};

class ThreadPool {
private:
    pthread_t* workers;
    size_t numThreads;
    Queue tasks;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool stop;

    static void* worker(void* arg) {
        ThreadPool* pool = static_cast<ThreadPool*>(arg);
        while (true) {
            pthread_mutex_lock(&(pool->lock));
            while (!(pool->stop) && pool->tasks.empty()) {
                pthread_cond_wait(&(pool->cond), &(pool->lock));
            }
            if ((pool->stop) && pool->tasks.empty()) {
                pthread_mutex_unlock(&(pool->lock));
                break;
            }
            auto frontNode = pool->tasks.front();
            void (*task)() = frontNode->data;
            void* args = frontNode->args;
            pool->tasks.pop();
            pthread_mutex_unlock(&(pool->lock));
            task();
        }
        pthread_exit(NULL);
        return NULL;
    }

public:
    ThreadPool(size_t threads) : stop(false), numThreads(threads) {
        workers = new pthread_t[threads];
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
        for (size_t i = 0; i < threads; ++i) {
            if (pthread_create(&workers[i], NULL, worker, this) != 0) {
                throw std::runtime_error("Failed to create thread");
            }
        }
    }

    ~ThreadPool() {
        pthread_mutex_lock(&lock);
        stop = true;
        pthread_mutex_unlock(&lock);
        pthread_cond_broadcast(&cond);
        for (size_t i = 0; i < numThreads; ++i) {
            pthread_join(workers[i], NULL);
        }
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
        delete[] workers;
    }

    template<typename... Args>
    void enqueue(void (*f)(Args...), Args... args) {
        auto arguments = new Tuple<Args...>{args...};
        pthread_mutex_lock(&lock);
        tasks.push(reinterpret_cast<void (*)()>(f), arguments);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
};

void task1() {
    std::cout << "Task 1 is running" << std::endl;
}

void task2(int x, int y) {
    std::cout << "Task 2 is running with arguments " << x << " and " << y << std::endl;
}

int main() {
    ThreadPool pool(2);
    pool.enqueue(task1);
    pool.enqueue(task2, 42, 43);
    return 0;
}
