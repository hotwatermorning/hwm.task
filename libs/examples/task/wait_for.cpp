//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <iomanip>
#include <iostream>
#include <hwm/task/task_queue.hpp>

//! wait_for()メンバ関数で指定時間内だけタスクが全て終了するのを待つサンプル

int main()
{
    hwm::task_queue tq(1);

    tq.enqueue([]{
        std::this_thread::sleep_for(std::chrono::seconds(3));
    });

    std::cout << ">>> waiting for a task to be finished" << std::endl;
    bool const result = tq.wait_for(std::chrono::seconds(1));
    std::cout << "wait result (false is expected.) : " << std::boolalpha << result << std::endl;
}

