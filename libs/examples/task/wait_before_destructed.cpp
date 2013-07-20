//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <hwm/task/task_queue.hpp>

int main()
{
    std::cout << ">>> wait before destructed" << std::endl;
    std::future<int> f_wait;
    {
        hwm::task_queue tq(1, 2);

        tq.enqueue([]() { std::this_thread::sleep_for(std::chrono::seconds(1)); });
        f_wait = tq.enqueue([](int a, int b) { return a + b; }, 10, 20);
    }
    std::cout << "<<< wait before destructed" << std::endl;

    try {
        std::cout << "10 + 20 = " << f_wait.get() << std::endl;
    } catch(std::future_error &e) {
        std::cout << e.what() << std::endl;
    }

    std::cout << ">>> don't wait before destructed" << std::endl;
    std::future<int> f_dont_wait;
    {
        hwm::task_queue tq(1, 2);

        tq.enqueue([]() { std::this_thread::sleep_for(std::chrono::seconds(1)); });
        f_dont_wait = tq.enqueue([](int a, int b) { return a + b; }, 10, 20);

        tq.set_wait_before_destructed(false);
    }
    std::cout << "<<< don't wait before destructed" << std::endl;

    try {
        std::cout << "10 + 20 = " << f_dont_wait.get() << std::endl;
    } catch(std::future_error &e) {
        std::cout << e.what() << std::endl;
    }
}

