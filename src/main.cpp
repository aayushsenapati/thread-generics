#include <iostream>
#include "Threading.h"
pthread_mutex_t cout_lock;

// ANSI escape codes for color and style
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

void task1()
{
    pthread_mutex_lock(&cout_lock);
    std::cout << GREEN << "Task 1 is running" << RESET << std::endl;
    pthread_mutex_unlock(&cout_lock);
}

void task2(int x, int y, std::string z)
{
    pthread_mutex_lock(&cout_lock);
    std::cout << GREEN << "Task 2 is running with arguments " << x << " and " << y << " and " << z << RESET << std::endl;
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

    pthread_mutex_lock(&cout_lock);

    // Print the thread ID and the thread-local variable
    std::cout << CYAN << "Thread ID: " << pthread_self() << ", Thread-Local Variable: " << value << RESET << std::endl;
    pthread_mutex_unlock(&cout_lock);

    return nullptr;
}


int main()
{
    try
    {
        std::cout << BOLD << BLUE << "*--------------------------------------------------------------------------------------------------------------------------*" << RESET << std::endl;
        std::cout << BOLD << YELLOW << "Testing Threadpool" << RESET << std::endl;

        pthread_mutex_init(&cout_lock, NULL);
        Threading::ThreadPool<4> pool;
        pool.enqueue(task1);
        pool.enqueue(task2, 42, 43, std::string("hello"));
        pool.enqueue([]()
            {
                pthread_mutex_lock(&cout_lock);
                std::cout << GREEN << "Lambda task is running"<< std::endl << RESET ;
                pthread_mutex_unlock(&cout_lock);
            });

        //pool.enqueue(someFunction, &nc);

        Threading::Master master;
        master.stopThreadPool(pool);

        while(master.get_num_tasks(pool));
        std::cout << BOLD << BLUE << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << RESET << std::endl;

        std::cout << BOLD << YELLOW << "Testing Parallel For" << RESET << std::endl;

        int x = 5;
        Threading::parallel_for_pthreads(0, 10, [x](int i)
                                         { std::cout << GREEN << "Task " << i << " finished by thread " << pthread_self() << ", x = " << x << RESET << std::endl; });

        std::cout << BOLD << BLUE << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << RESET << std::endl;

        std::cout << BOLD << YELLOW << "Testing Mutex and Lock" << RESET << std::endl;

        Threading::Mutex mtx;

        {
            Threading::Lock lock(mtx);
            std::cout << GREEN << "Inside critical section.\n" << RESET;
        }

        std::cout << "Outside critical section.\n";

        std::cout << BOLD << BLUE << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << RESET << std::endl;

        std::cout << BOLD << YELLOW << "Testing Thread Local Storage" << RESET << std::endl;
        Threading::ThreadLocalStorage<pthread_t> tls;

        pthread_t thread1, thread2;
        pthread_create(&thread1, nullptr, threadFunc, &tls);
        pthread_create(&thread2, nullptr, threadFunc, &tls);

        pthread_join(thread1, nullptr);
        pthread_join(thread2, nullptr);

        std::cout << BOLD << BLUE << "*-------COMPLETED----------------------------------------------------------------------------------------------------------*" << RESET << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << RED << "Exception: " << e.what() << RESET << std::endl;
        return 1;
    }
}