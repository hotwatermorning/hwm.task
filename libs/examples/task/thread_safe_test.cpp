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
    int const num_threads = 30;
    int const num_tasks = 10000;

    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(num_threads);

    std::deque<std::future<int>> fs;

    std::mutex m;
    int count = 0;

    for(int i = 0; i < num_tasks; ++i) {
        std::future<int> f =
            tq.enqueue(
                [&m, &count](int x1, int x2) -> int {
                    {
                        std::lock_guard<std::mutex> lock(m);
                        count += 1;

                        count *= 2;
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                        count /= 2;

                    }
                    return x1 + x2;
                },
                10, 20
            );
        fs.push_back(std::move(f));
    }

    for( ; ; ) {
        if(fs.empty()) {
            break;
        }
        fs[0].wait();
        fs.pop_front();
    }

    std::cout << "expected : " << num_tasks << ", result : " << count << std::endl;
}

