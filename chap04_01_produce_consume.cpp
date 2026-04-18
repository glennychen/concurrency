#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <queue>
#include <mutex>
#include <chrono>

using namespace std;


class producer_consumer{
    queue<int> q;
    mutex mtx;
    condition_variable cr;
    bool no_work=false;
    atomic<size_t> consumer_id=0;
public:
    void produce(){

        for(size_t i=0;i<50;++i){
            {
                unique_lock<mutex> lk(mtx);
                q.push(i);
            }
            cr.notify_one();
            this_thread::sleep_for(chrono::milliseconds(10)); //study this
        }
        //sleep(100);

        {
        lock_guard<mutex> lk(mtx);
        no_work=true;
        }
        cr.notify_all();

    }

    void consume(){
        size_t id=++consumer_id; //note: fetch and read atomic in one step
        while(true){
            unique_lock<mutex> lk(mtx);
            cr.wait(lk,[this](){
                return !q.empty() || no_work==true;
            });

            if(q.empty() && no_work){
                cout << "no work, this thread ID={} is done." <<id << endl;
                break;
            }
            int top =q.front();
            q.pop();
            cout << id << " consume: " << top << endl;
        }
    }
};



int main()
{
    producer_consumer pr;
    jthread produce([&pr](){pr.produce();});
    vector<jthread> v;

    int num_threads=thread::hardware_concurrency();
    for(int i=0;i<num_threads-1;++i){
        v.emplace_back([&pr](){pr.consume();});
    }


    return 0;
}
