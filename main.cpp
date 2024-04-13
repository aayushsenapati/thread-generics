#include <iostream>
#include "threadpool.h"
#include "parallelfor.h"
#include "master.cpp"

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

int main()
{
    try
    {
        pthread_mutex_init(&cout_lock, NULL);
        Threading::ThreadPool<4> pool;
        pool.enqueue(task1);
        pool.enqueue(task2, 42, 43, std::string("hello"));
        pool.enqueue([]()
                     {
            pthread_mutex_lock(&cout_lock);
            std::cout << "Lambda task is running" << std::endl;
            pthread_mutex_unlock(&cout_lock); });

        int x = 5;
        Threading::parallel_for_pthreads(0, 10, [x](int i)
                              { std::cout << "Task " << i << " finished by thread " << pthread_self() << ", x = " << x << std::endl; });

        Threading::Master master;
        master.stopThreadPool(pool);
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}