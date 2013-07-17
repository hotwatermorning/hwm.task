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
    std::cout << ">>> wait before destructed" << std::endl;
    {
        //! タスクキュー
        //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
        //! 実行するスレッドの数をコンストラクタで指定する。
        hwm::task_queue tq(1, 5);
        for(int i = 0; i < 5; ++i) {
            tq.enqueue(
                //! タスクキュー内のスレッドで起動する関数
                [i]() {
                    std::cout << "<<< task[" << i << "]" << std::endl;
                    std::this_thread::sleep_for(
                        std::chrono::seconds(1)
                        );
                    std::cout << ">>> task[" << i << "]" << std::endl;
                }
            );
        }
    }
    std::cout << "<<< wait before destructed" << std::endl;

    std::cout << ">>> don't wait before destructed" << std::endl;
    {
        //! タスクキュー
        //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
        //! 実行するスレッドの数をコンストラクタで指定する。
        hwm::task_queue tq(1, 5);
        tq.set_wait_before_destructed(false);
        for(int i = 0; i < 5; ++i) {
            tq.enqueue(
                //! タスクキュー内のスレッドで起動する関数
                [i]() {
                    std::cout << "<<< task[" << i << "]" << std::endl;
                    std::this_thread::sleep_for(
                        std::chrono::seconds(1)
                        );
                    std::cout << ">>> task[" << i << "]" << std::endl;
                }
            );
        }
    }
    std::cout << "<<< don't wait before destructed" << std::endl;
}

