//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <iomanip>
#include <iostream>
#include <hwm/task/task_queue.hpp>

int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(std::thread::hardware_concurrency());

    std::future<void> f =
        tq.enqueue(
            //! タスクキュー内のスレッドで起動する関数
            []() {
                std::this_thread::sleep_for(
                    std::chrono::seconds(3)
                    );
            }
        );

    std::cout << ">>> waiting" << std::endl;
    bool const result = tq.wait_for(std::chrono::seconds(1));
    std::cout << "wait result (false is expected.) : " << std::boolalpha << result << std::endl;
}

