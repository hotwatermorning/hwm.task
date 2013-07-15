//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <functional>
#include <random>
#include <functional>

#include <hwm/task/task_queue.hpp>

//! hwm::task_queueのサンプル
int main()
{
    //! ランダム数列生成器
    auto random =
        std::bind(
            std::uniform_int_distribution<int>(0, 1000),
            std::mt19937() );

    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(std::thread::hardware_concurrency());

    //! 結果を取得するためのfutureを保持
    std::vector<std::future<int>> futures;

    //! 30個分のタスクを生成。
    for(int task_index = 0; task_index < 30; ++task_index) {
        std::cout << "Add task : " << task_index << std::endl;

        //! enqueueして、結果を取り出すためのfutureオブジェクトを受け取る。
        std::future<int> future =
            tq.enqueue_sync(
                //! キューから取り出され、呼び出される際に実行される関数
                [](int index, int delay) -> int
                {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(delay)
                        );

                    return index;
                },
                task_index,
                random()
                );

        //! futureオブジェクトを保持
        futures.push_back(std::move(future));
    }

    for( ; ; ) {
        if(futures.empty()) {
            break;
        }

        //! futureオブジェクトの配列をなめて、
        //! 処理が完了してる物はそのインデックスを表示して、配列から取り除く
        for(auto it = futures.begin(); it != futures.end(); ++it) {

            std::future<int> &future = *it;

            if(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                std::cout << "[" << future.get() << "]" << std::endl;
                futures.erase(it);
                break;
            }
        }
    }

    std::cout << "finished" << std::endl;
}
