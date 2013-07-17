//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_TASK_TASKQUEUE_HPP
#define HWM_TASK_TASKQUEUE_HPP

#include <atomic>
#include <future>
#include <limits>
#include <thread>
#include <type_traits>
#include <utility>

#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>

#include "../function_result_type.hpp"
#include "../make_unique.hpp"

#include "./locked_queue.hpp"
#include "./task_impl.hpp"

namespace hwm {

namespace detail { namespace ns_task {

//! @class タスクキュークラス
struct task_queue
{
    typedef std::unique_ptr<task_base>  task_ptr_t;

    //! デフォルトコンストラクタ
    //! std::thread::hardware_concurrency()分だけスレッドを起動する
    task_queue()
        :   task_queue_()
        ,   terminated_flag_(false)
    {
        setup(std::thread::hardware_concurrency() || 1);
    }

    //! @brief コンストラクタ
    //! @detail 引数に指定された値だけスレッドを起動する
    //! @param thread_limit [in] 起動する引数の数
    //! @param queue_limit [in] キューに保持できるタスク数の限界
    task_queue(size_t thread_limit, size_t queue_limit = ((std::numeric_limits<size_t>::max)()))
        :   task_queue_(queue_limit)
        ,   terminated_flag_(false)
    {
        BOOST_ASSERT(thread_limit >= 1);
        BOOST_ASSERT(queue_limit >= 1);
        setup(thread_limit);
    }

    //! デストラクタ
    //! スレッドの終了を待ってデストラクトする
    //! キューに積まれたままのタスクは実行されない。
    //! すべて実行してから終了する版は未実装
    ~task_queue()
    {
        set_terminate_flag(true);
        join_threads();
    }

    //! @brief タスクに新たな処理を追加
    //! @detail enqueue_asyncとの違いは、内部locked_queueが溢れているときには、
    //! locked_queueが空くまで処理をブロックする点。
    //! @return タスクとshared stateを共有するstd::futureクラスのオブジェクト
    template<class F, class... Args>
    std::future<typename function_result_type<F, Args...>::type>
            enqueue(F f, Args... args)
    {
//         std::cout << "sync enqueued" << std::endl;

        typedef typename function_result_type<F, Args...>::type func_result_t;
        typedef std::promise<func_result_t> promise_t;

        promise_t promise;
        auto future(promise.get_future());

        std::unique_ptr<task_base> ptask(
            new task_impl<F, Args...>(
                std::move(promise),
                std::forward<F>(f),
                std::forward<Args>(args)... ) );

        task_queue_.enqueue(std::move(ptask));

        return future;
    }

private:
    locked_queue<task_ptr_t>    task_queue_;
    std::vector<std::thread>    threads_;
    std::atomic<bool>           terminated_flag_;

private:

    void    set_terminate_flag(bool state)
    {
        terminated_flag_.store(state);
    }

    bool    is_terminated() const
    {
        return terminated_flag_.load();
    }

    void    setup(int num_threads)
    {
        for(int i = 0; i < num_threads; ++i) {
            std::thread th(
                [this](int i) {
                    for( ; ; ) {
                        if(is_terminated()) {
                            break;
                        }

                        task_ptr_t task;
                        bool popped = task_queue_.try_dequeue_for(task, std::chrono::seconds(1));

                        if(popped) {
//                             std::cout << "popped by thread[" << i << "]" << std::endl;
                            task->run();
                        }
                    }
                },
                i );
            threads_.push_back(std::move(th));
        }
    }

    void    join_threads()
    {
        BOOST_ASSERT(is_terminated());

        for(auto &th: threads_) {
            th.join();
        }
    }
};

}}  //detail::ns_task

//! hwm::detail::ns_task内のtask_queueクラスをhwm名前空間で使えるように
using detail::ns_task::task_queue;

}   //namespace hwm

#endif  //HWM_TASK_TASKQUEUE_HPP

