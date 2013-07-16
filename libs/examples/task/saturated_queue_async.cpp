//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <deque>
#include <mutex>
#include <hwm/task/task_queue.hpp>

int main()
{
    int const num_threads = 3;
    int const num_queue = 3;
    int const num_tasks = 30;

    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(num_threads, num_queue);

    std::deque<std::future<void>> fs;

    std::mutex m;

    for(int i = 0; i < num_tasks; ++i) {
        {
            std::lock_guard<std::mutex> lock(m);
            std::cout << ">>> enqueue [" << i << "]" << std::endl;
        }

        //! add a new task asynchroniously.
        auto future =
            tq.enqueue_async(
                [&m, i]() mutable {
                    {
                        std::lock_guard<std::mutex> lock(m);
                        std::cout << "--- run [" << i << "]" << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            );

        fs.push_back(boost::move(future));

        {
            std::lock_guard<std::mutex> lock(m);
            std::cout << "<<< enqueue [" << i << "]" << std::endl;
        }
    }

    for( ; ; ) {
        if(fs.empty()) {
            break;
        }
        fs[0].wait();
        fs.pop_front();
    }
}

