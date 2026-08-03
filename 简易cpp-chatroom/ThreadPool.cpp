#include "ThreadPool.h"
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "./Logger.h"

void sys_err(int ret, std::string str) {
    if (ret != 0) {
        std::cout << str;
        fprintf(stderr, " error:%s\n", strerror(ret));
        exit(1);
    }
}



void* pthread_pool::pthreadrun(void* arg) {
    pthread_pool* pool = static_cast<pthread_pool*>(arg);
    pool->consumer(pool);
    return nullptr;
}

void* pthread_pool::consumer(pthread_pool* pool) {
    while (1) {
        int key = 0;
        pthread_mutex_lock(&pool->lock);

        while (key == 0 || (pool->task_queue == NULL && pool->stop == false)) {
            key = 1;
            pthread_cond_wait(&pool->cond, &pool->lock);
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        task* mytask = pool->task_queue;
        if (mytask == NULL) {
            pthread_mutex_unlock(&pool->lock);
            continue;
        }
        pool->task_queue = pool->task_queue->next;
        pthread_mutex_unlock(&pool->lock);

        mytask->function();
        delete mytask;
    }
    return NULL;
}

pthread_pool::pthread_pool(int ThreadCount) {
    int ret = pthread_mutex_init(&lock, NULL);
    sys_err(ret, "mutex_init");

    ret = pthread_cond_init(&cond, NULL);
    sys_err(ret, "cond_init");

    task_queue = NULL;
    pthreads = (pthread_t*)malloc(ThreadCount * sizeof(pthread_t));
    pth_cnt = ThreadCount;

    for (int i = 0; i < ThreadCount; i++) {
        ret = pthread_create(&pthreads[i], NULL, pthreadrun, this);
        sys_err(ret, "pthread_create");
    }

    stop = false;
}

pthread_pool::~pthread_pool() {
    pthread_mutex_lock(&this->lock);
    this->stop = true;
    pthread_cond_broadcast(&this->cond);
    pthread_mutex_unlock(&this->lock);

    for (int i = 0; i < this->pth_cnt; i++) {
        pthread_join(this->pthreads[i], NULL);
    }

    free(this->pthreads);

    task* pt;
    while (this->task_queue) {
        pt = this->task_queue;
        this->task_queue = pt->next;
        delete pt;
    }

    pthread_mutex_destroy(&this->lock);
    pthread_cond_destroy(&this->cond);
}
