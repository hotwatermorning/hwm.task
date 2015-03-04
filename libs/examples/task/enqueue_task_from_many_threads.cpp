//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <vector>
#include <hwm/task/task_queue.hpp>
#include "../utils/stream_mutex.hpp"

int const kTaskPerThread = 1000;
int const kNumThreads = 30;

//! タスクキューにタスクを追加するワーカースレッド
void worker_thread(hwm::task_queue &tq, int thread_index)
{
    std::vector<std::future<void>> futures;

    for(int task_index = 0; task_index < kTaskPerThread; ++task_index) {

        auto future = tq.enqueue(
            [](int thread_index, int task_index) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if((task_index+1) % 100 == 0) {
                    hwm::mcout << "thread [" << thread_index << "]( " << (task_index+1) * 100 / (kTaskPerThread) << "% )" << std::endl;
                }
            },
            thread_index,
            task_index
            );

        futures.push_back(std::move(future));
    }

    hwm::mcout << "Finished to enqueue tasks[" << thread_index << "]" << std::endl;

    //! 全てのタスクが完了したらスレッドを終わる。
    for(int task_index = 0; task_index < kTaskPerThread; ++task_index) {
        futures[task_index].wait();
    }

    hwm::mcout << "Finish worker thread[" << thread_index << "]" << std::endl;
}

//! hwm::task_queueのサンプル
int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(20);

    std::vector<std::thread> workers;

    //! ワーカースレッドを生成。
    for(int thread_index = 0; thread_index < kNumThreads; ++thread_index) {
        hwm::mcout << "Create worker thread[" << thread_index << "]" << std::endl;

        std::thread worker(worker_thread, std::ref(tq), thread_index);
        workers.push_back(std::move(worker));
    }

    for(auto &worker: workers) {
        worker.join();
    }

    hwm::mcout << "finish" << std::endl;
}

