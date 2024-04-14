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
        pthread_mutex_lock(&pool.lock);
        pool.stop = true;
        pthread_mutex_unlock(&pool.lock);
        pthread_cond_broadcast(&pool.cond);
    }
};
#endif
}