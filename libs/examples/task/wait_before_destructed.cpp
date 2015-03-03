//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <iostream>
#include <sstream>

#include <hwm/task/task_queue.hpp>
#include "../utils/stream_mutex.hpp"

//! タスクキューが破棄される際に、積まれているタスクがすべて実行されるのを待つか、
//! まだ取り出されていないタスクは破棄して、そのままタスクキューを破棄するかを設定する、
//! set_wait_before_destructed()メンバ関数のサンプル

int main()
{
    hwm::mcout << ">>> wait before destructed" << std::endl;

    {
        hwm::task_queue tq(1);

        tq.enqueue([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
            });

        tq.enqueue([]() {
            hwm::mcout << "this task may be executed" << std::endl;
            });
    }

    hwm::mcout << "<<< wait before destructed" << std::endl;

    hwm::mcout << ">>> don't wait before destructed" << std::endl;
    {
        hwm::task_queue tq(1);

        tq.enqueue([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
            });

        tq.enqueue([]() {
            hwm::mcout << "this task may not be executed" << std::endl;
            });

        tq.set_wait_before_destructed(false);
    }
    hwm::mcout << "<<< don't wait before destructed" << std::endl;
}

