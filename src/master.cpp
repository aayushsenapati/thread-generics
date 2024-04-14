#ifndef MASTER_H
#define MASTER_H
#include "threadpool.h"
#include <pthread.h>


//friend master function of threadpool class to stop the threadpool

namespace Threading{
class Master {
public:
    template <size_t N>
    void stopThreadPool(Threading::ThreadPool<N>& pool) {
        pthread_mutex_lock(&pool.queue_bool_lock);
        pool.stop = true;
        pthread_mutex_unlock(&pool.queue_bool_lock);
        pthread_cond_broadcast(&pool.cond);
    }

    template <size_t N>
    size_t get_num_tasks(Threading::ThreadPool<N>& pool) {
        pthread_mutex_lock(&pool.task_executing_lock);
        size_t num_tasks = pool.task_exec_count;
        pthread_mutex_unlock(&pool.task_executing_lock);
        return num_tasks;
    }
};
#endif
}