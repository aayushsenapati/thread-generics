#include <iostream>
#include "Threading.h"
pthread_mutex_t cout_lock;

void task1()
{
    pthread_mutex_lock(&cout_lock);
    std::cout << "Task 1 is running" << std::endl;
    pthread_mutex_unlock(&cout_lock);
}

void task2(int x, int y, std::string z)
{
    pthread_mutex_lock(&cout_lock);
    std::cout << "Task 2 is running with arguments " << x << " and " << y << " and " << z << std::endl;
    pthread_mutex_unlock(&cout_lock);
}

// Function that each thread will run
void* threadFunc(void* arg) {
    // Cast the argument to ThreadLocalStorage<int>*
    Threading::ThreadLocalStorage<pthread_t>* tls = static_cast<Threading::ThreadLocalStorage<pthread_t>*>(arg);

    // Set a thread-local variable
    tls->set(pthread_self());

    // Get the thread-local variable
    pthread_t value = tls->get();

    // Print the thread ID and the thread-local variable
    std::cout << "Thread ID: " << pthread_self() << ", Thread-Local Variable: " << value << std::endl;

    return nullptr;
}

int main()
{
    try
    {


/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout << "Testing Threadpool" << std::endl;


        pthread_mutex_init(&cout_lock, NULL);
        Threading::ThreadPool<4> pool;
        pool.enqueue(task1);
        pool.enqueue(task2, 42, 43, std::string("hello"));
        pool.enqueue([]()
            {
                pthread_mutex_lock(&cout_lock);
                std::cout << "Lambda task is running" << std::endl;
                pthread_mutex_unlock(&cout_lock);
            });

        Threading::Master master;
        master.stopThreadPool(pool);
        std::cout << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << std::endl;

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout << "Testing Parallel For" << std::endl;

        int x = 5;
        Threading::parallel_for_pthreads(0, 10, [x](int i)
                                         { std::cout << "Task " << i << " finished by thread " << pthread_self() << ", x = " << x << std::endl; });

        std::cout << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << std::endl;

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout << "Testing Mutex and Lock" << std::endl;

        Threading::Mutex mtx;

        {
            Threading::Lock lock(mtx);
            // Critical section goes here.
            // The mutex is locked at this point.
            std::cout << "Inside critical section.\n";
            // The mutex will be automatically unlocked
            // when the lock object goes out of scope.
        }

        std::cout << "Outside critical section.\n";

        std::cout << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << std::endl;
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        std::cout << "Testing Thread Local Storage" << std::endl;
        // Create a ThreadLocalStorage instance
        Threading::ThreadLocalStorage<pthread_t> tls;

        // Create two threads
        pthread_t thread1, thread2;
        pthread_create(&thread1, nullptr, threadFunc, &tls);
        pthread_create(&thread2, nullptr, threadFunc, &tls);

        // Wait for the threads to finish
        pthread_join(thread1, nullptr);
        pthread_join(thread2, nullptr);

        std::cout << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << std::endl;
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}