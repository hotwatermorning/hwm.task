#include <thread>
#include <mutex>
#include <iostream>
#include "../hwm/spin_lock/spin_lock_gcc.hpp"

hwm::spin_lock sl;
int count;
static int const num_threads = 200;
static int const num_invokation_per_thread = 100;
static int const expected = num_threads * num_invokation_per_thread;

void foo()
{
    {
        std::lock_guard<hwm::spin_lock> lock(sl);
        count += 1;
    }


    {
        std::lock_guard<hwm::spin_lock> lock(sl);
        count *= 2;

        int i = 0;
        int j = 0;
        for(int i = 0; i < 10000; ++i) {
            j += i;
        }

        count += j;

        count -= 1;
        count -=(j-1);

        count /= 2;
    }
}

void bar()
{
    for(int i = 0; i < num_invokation_per_thread; ++i) {
        foo();
    }
}

int main()
{
    std::vector<std::thread> ths;
    for(int i = 0; i < num_threads; ++i) {
        ths.push_back(
            std::thread(bar)
            );
    }

    for(int i = 0; i < num_threads; ++i) {
        std::cout << "joining : " << i << std::endl;
        ths[i].join();
    }


    std::cout << "expected : " << expected << ", count : " << count << std::endl;
}

