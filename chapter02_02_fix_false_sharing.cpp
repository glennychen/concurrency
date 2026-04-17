#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
using namespace std;

struct alignas(64) padded_result {
    long long value = 0;
};

void add(padded_result &sum, vector<long long>& v, size_t start, size_t end){
    for(size_t i=start;i<end;++i){
        sum.value+=v[i];
    }
}

int main()
{
    vector<long long> v(1000000);
    iota(v.begin(), v.end(), 1);

    int num_threads=thread::hardware_concurrency();

    size_t chunk=v.size()/num_threads;
    vector<padded_result> res(num_threads);
    long long total=0;

    {
        //ensure main thread will not finish(or the sum will not be ready), before jthreads enclosing in the block
        vector<jthread> jt;
        for(int i=0;i<num_threads-1;++i){
            jt.emplace_back(add, ref(res[i]), ref(v), i*chunk, (i+1)*chunk);
        }

        //the main thread add happens while other threads running in the for loop
        //watch out the remtainder chunk start, "the -1"
        add(res[num_threads-1], v,chunk*(num_threads-1), v.size());
    }

    for(const auto& elem: res){
        total+=elem.value;
    }

    cout << total;
    return total;
}
