#include <iostream>
#include <hwm/task/task_queue.hpp>

int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(std::thread::hardware_concurrency());

    std::future<int> f =
        tq.enqueue_sync(
            //! タスクキュー内のスレッドで起動する関数
            [](int x1, int x2) -> int {
                std::cout << (x1 + x2) << std::endl;
                return x1 + x2;
            },
            //! 関数に渡す引数
            10, 20
        );

    std::cout << "calculated value : " << f.get() << std::endl;
}
