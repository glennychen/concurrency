#include <iostream>
#include <thread>
#include <vector>
#include <assert.h>

using namespace std;

struct Node{
    Node* next;
    int val;
    Node(int v): val{v}, next(nullptr) {}
};

class lock_free_stack{
    atomic<int> cnt=0;
    atomic<int> push_retries=0;
    atomic<int> pop_retries=0;
    atomic<Node*> top{nullptr};

    int actual_total=0;
public:
    int pop(){
        Node* old_top=top.load();
        //Node* next=n->next; //but next coulld change already! how do i load and read ?
        while(old_top && !top.compare_exchange_weak(old_top,old_top->next)){
            ++pop_retries;
        }
        if(old_top){
            return old_top->val;
        }
        delete old_top; //not safe
        return -1;
    }
    void push(int val){
        Node* n = new Node(val);
        n->next=top.load();
        while(!top.compare_exchange_weak(n->next, n)){
            ++push_retries;
        }
    }

    int get_value() const {
        return cnt.load();
    }

    int get_push_retries() const {
        return push_retries;
    }

    int get_pop_retries() const {
        return pop_retries;
    }

    int get_actual_total() const {
        return actual_total;
    }

    void clean_up(){
        Node* curr=top.exchange(nullptr);
        while(curr){
            Node* next=curr->next;
            delete curr;
            ++actual_total;
            curr=next;
        }
    }
};



int main()
{    
    vector<jthread> v;
    int num_threads=thread::hardware_concurrency()-1;
    lock_free_stack lfs;
    const int iterations = 1000;
    int expected_total=num_threads*iterations;
    for(int i=0;i<num_threads;++i){
        v.emplace_back([&lfs](){
            for(int i=0;i<iterations;++i){
                lfs.push(i);
            }
        });
    }


    v.clear();
    cout << "push retries:" << lfs.get_push_retries() << endl;

    lfs.clean_up();
    assert(lfs.get_actual_total()==expected_total);

    for(int i=0;i<num_threads;++i){
        v.emplace_back([&lfs](){
            for(int i=0;i<iterations;++i){
                lfs.push(i);
            }
        });
    }

    for(int i=0;i<num_threads;++i){
        v.emplace_back([&lfs](){
            for(int i=0;i<iterations;++i){
                lfs.pop();
            }
        });
    }
    v.clear();
    cout << "pop retries:" << lfs.get_pop_retries() << endl;

    return 0;
}
