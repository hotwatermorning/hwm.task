#include <vector>
#include <hwm/task/task_queue.hpp>
#include <functional>
#include <random>
#include <functional>

void thread_process(hwm::task_queue &tq, int task_index, int delay)
{
    std::vector<std::future<int>> futures;

    for(int sub_index = 0; sub_index < 10; ++sub_index) {
        auto future = tq.enqueue_async(
            [](int task_index, int sub_index, int delay) -> int {
                std::cout << "run new task [" << task_index << "." << sub_index << "]" << std::endl;

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(delay)
                    );

                return task_index;
            },
            task_index,
            sub_index,
            delay );

        futures.push_back(std::move(future));
    }

    for(int sub_index = 0; sub_index < 10; ++sub_index) {
        futures[sub_index].wait();
    }
}

//! hwm::task_queueのサンプル
void enqueue_task_from_many_threads()
{
    //! ランダム数列生成器
    auto random =
        std::bind(
            std::uniform_int_distribution<int>(0, 1000),
            std::mt19937() );

    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(20 + std::thread::hardware_concurrency());

    //! 結果を取得するためのfutureを保持
    std::vector<std::future<int>> futures;
    std::vector<std::thread> ths;

    //! 30個分のタスクを生成。
    for(int task_index = 0; task_index < 30; ++task_index) {
        std::cout << "Add task : " << task_index << std::endl;

        std::thread th(
            thread_process,
            std::ref(tq),
            task_index,
            random() );

        ths.push_back(std::move(th));
    }

    for(auto &th: ths) {
        th.join();
    }

    std::cout << "finished" << std::endl;
}
