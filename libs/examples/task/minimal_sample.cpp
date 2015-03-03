//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <hwm/task/task_queue.hpp>

int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(1);

    std::future<int> f =
        tq.enqueue(
            //! タスクキュー内のスレッドで起動する関数
            [](int x1, int x2) -> int {
                std::cout << (x1 + x2) << std::endl;
                return x1 + x2;
            },
            //! 関数に渡す引数
            10, 20
        );

    //! 引数に渡したラムダ式が非同期で実行される

    //! ラムダ式の実行完了を待って、結果を表示
    std::cout << "calculated value : " << f.get() << std::endl;
}
