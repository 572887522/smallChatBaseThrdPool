#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>


//单例类
class ThreadPool
{
public:
    ThreadPool(int num) : stop(false) {
        // std::cout << "thread Pool construction" << std::endl; 
        for(int i = 0; i < num; i++){
            works.emplace_back([this]{
                for(;;){
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock, [this]{return stop || !tasks.empty();});
                    if(stop && tasks.empty())
                        return;
                    std::function<void()> task(std::move(tasks.front()));
                    tasks.pop();
                    lock.unlock();

                    task();
                }
            });
        }
    }

    template<class T, class... Args>
    void enequeue(T &&t, Args&&... args){
        std::function<void()> task(std::bind(std::forward<T>(t), std::forward<Args>(args)...));
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
    }

    ~ThreadPool(){
        // std::cout << "thread Pool deconstruction" << std::endl; 
        stop = true;
        condition.notify_all();
        for(std::thread& thread : works){
            thread.join();
        }
    }

private:
    ThreadPool() = delete;
    std::vector<std::thread> works;
    std::queue<std::function<void()>> tasks;

    std::mutex mtx;
    std::condition_variable condition;
    bool stop;
};


void threadPoolTest()
{
    ThreadPool pool(10);
    for(int i = 0; i < 100; i++)
    {
        pool.enequeue([i]{
            std::cout << "task " << i << " is running " << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "task " << i << " is done " << std::endl << std::endl;
        });
    }
}


#endif // THREAD_POOL_HH