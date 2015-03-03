//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <deque>
#include <random>
#include <functional>
#include <hwm/task/task_queue.hpp>
#include "../utils/stream_mutex.hpp"

//! タスクキューのサイズを制限して、順次データが取り出されるまで、enqueueがブロックされる挙動のサンプル。

int main()
{
    auto random_delay = std::bind(std::uniform_int_distribution<int>(300, 1000), std::mt19937());

    int const num_threads = 2;
    int const num_queue = 2;
    int const num_tasks = 15;

    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(num_threads, num_queue);

    std::deque<std::future<void>> futures;

    for(int i = 0; i < num_tasks; ++i) {
        hwm::mcout << ">>> Want to enqueue [" << i << "]" << std::endl;
        auto delay = random_delay();

        auto future =
            tq.enqueue(
                [i, delay]() {
                    hwm::mcout << "--- execute [" << i << "]" << std::endl;

                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                }
            );

        futures.push_back(std::move(future));

        hwm::mcout << "<<< Enqueued [" << i << "]" << std::endl;
    }

    for( ; ; ) {
        if(futures.empty()) {
            break;
        }
        futures[0].wait();
        futures.pop_front();
    }
}

