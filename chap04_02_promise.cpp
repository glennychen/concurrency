#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include <future>
#include <assert.h>

using namespace std;


class producer_consumer{
    queue<int> q;
    mutex mtx;
    condition_variable cr;
    bool no_work=false;
    atomic<size_t> consumer_id=0;

public:
    static const int total_work=50;
    void produce(){

        for(size_t i=0;i<total_work;++i){
            {
                unique_lock<mutex> lk(mtx);
                q.push(i);
            }
            cr.notify_one();
            this_thread::sleep_for(chrono::milliseconds(100)); //study this
        }
        //sleep(100);

        {
        lock_guard<mutex> lk(mtx);
        no_work=true;
        }
        cr.notify_all();

    }

    void consume(promise<int> p){
        size_t id=++consumer_id; //note: fetch and read atomic in one step
        int cnt=0;
        while(true){
            unique_lock<mutex> lk(mtx);
            cr.wait(lk,[this](){
                return !q.empty() || no_work==true;
            });

            if(q.empty() && no_work){
                cout << "no work, this thread ID={} is idel." <<id << endl;
                p.set_value(cnt); //use a promise to get back data from the thread without global variable
                break;
            }
            int top =q.front();
            q.pop();
            ++cnt;
            cout << id << " consume: " << top << endl;
        }
    }
};



int main()
{
    producer_consumer pr;
    jthread produce([&pr](){pr.produce();});
    vector<jthread> v;

    int num_threads=thread::hardware_concurrency()-1;
    vector<future<int>> vfuture;
    for(int i=0;i<num_threads;++i){
        promise<int> p;
        vfuture.push_back(p.get_future());
        v.emplace_back([&pr](promise<int> p_from_backpack){
            pr.consume(std::move(p_from_backpack));
        }, std::move(p));
    }

    int work_done=0;
    for(int i=0;i<vfuture.size();++i){
        work_done+=vfuture[i].get();
    }
    assert(work_done==producer_consumer::total_work);

    return 0;
}
