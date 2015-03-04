//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <hwm/task/task_queue.hpp>
#include "../utils/stream_mutex.hpp"

//! wait()メンバ関数でタスクが全て終了するのを待つサンプル

int main()
{
    hwm::task_queue tq(3);

    for(int i = 0; i < 3; ++i) {
        tq.enqueue([i]{
            hwm::mcout << ">>> Execute task[" << i << "]" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            hwm::mcout << "<<< Execute task[" << i << "]" << std::endl;
        });
    }

    hwm::mcout << ">>> waiting for 3 tasks to be finished" << std::endl;
    tq.wait();
    hwm::mcout << "<<< finished" << std::endl;
}

