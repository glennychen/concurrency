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



class compare_and_swap_inrement{
    atomic<int> cnt=0;
    atomic<int> retries=0;
public:
    void case_increment(int iterations){
        for(int i=0;i<iterations;++i){
            int expected=cnt.load();
            while(!cnt.compare_exchange_strong(expected, expected+1)){
                ++retries;
            }
        }
    }
    int get_value() const {
        return cnt.load();
    }
    int get_retries() const {
        return retries;
    }
};



int main()
{    
    vector<jthread> v;
    int num_threads=thread::hardware_concurrency()-1;
    compare_and_swap_inrement cas;
    const int iterations = 1000;
    for(int i=0;i<num_threads;++i){
        v.emplace_back([&cas](){
            cas.case_increment(iterations);
        });
    }
    v.clear();

    int final_value=cas.get_value();
    int retries=cas.get_retries();
    cout << "final value: " << final_value << endl;
    cout << "retries: " << retries << endl;
    return 0;
}
