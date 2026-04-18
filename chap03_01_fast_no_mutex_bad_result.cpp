#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
using namespace std;


void increment_n_times(long long& total, size_t n){
    for(size_t i=0;i<n;++i){
        ++total;
    }
}

int main()
{
    long long total=0;
    {
        vector<jthread> v;
        int n=1000000;
        for(int i=0;i<4;++i){
            v.emplace_back(increment_n_times, ref(total), n);
        }
    }
    cout << total;
    return 0;
}
