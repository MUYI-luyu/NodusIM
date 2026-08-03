#pragma once
#include <pthread.h>
#include <functional>
#include <memory>
#include <string>

// 任务队列（链表）类
class task {
public:
    std::function<void()> function;
    task *next;

    // 构造函数模板定义在头文件中
    template<typename Func, typename... Args>
    task(Func&& func, Args&&... args) {
    function = [func = std::forward<Func>(func), 
               args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        std::apply(func, args);
    };
}
};

// 线程池类
class pthread_pool {
private:
    pthread_mutex_t lock;
    pthread_cond_t cond;
    task *task_queue;
    pthread_t *pthreads;
    int pth_cnt;
    bool stop;

    static void* pthreadrun(void* arg);

public:
    void* consumer(pthread_pool* pool);
    pthread_pool(int ThreadCount = 16);

    // PushTask 模板成员函数定义在头文件中
    template<typename Func, typename... Args>
    void PushTask(Func&& func, Args&&... args) {
        task* NewTask = new task(std::forward<Func>(func), std::forward<Args>(args)...);
        NewTask->next = nullptr;
        pthread_mutex_lock(&this->lock);

        if (this->task_queue == nullptr) {
            this->task_queue = NewTask;
        } else {
            task* p = this->task_queue;
            while (p->next != nullptr) p = p->next;
            p->next = NewTask;
        }

        pthread_cond_signal(&this->cond);
        pthread_mutex_unlock(&this->lock);
    }

    ~pthread_pool();
};
