//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_TASK_TASKQUEUE_HPP
#define HWM_TASK_TASKQUEUE_HPP

#include <future>
#include <thread>
#include <type_traits>

#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>

#include "../function_result_type.hpp"
#include "../make_unique.hpp"
#include "../spin_lock.hpp"

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
        :   terminated_flag_(false)
    {
        setup(std::thread::hardware_concurrency() || 1);
    }

    //! デフォルトコンストラクタ
    //! 引数に指定された値だけスレッドを起動する
    task_queue(int thread_limit)
        :   terminated_flag_(false)
    {
        BOOST_ASSERT(thread_limit >= 1);
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
            enqueue_sync(F f, Args... args)
    {
        std::cout << "sync enqueued" << std::endl;

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

    //! @brief タスクに新たな処理を追加
    //! @detail enqueue_asyncとの違いは、内部locked_queueが溢れているときにも、
    //! locked_queueへの追加は別スレッドを起動して、即座にfutureオブジェクトを返す点。
    //! @return タスクとshared stateを共有するstd::futureクラスのオブジェクト
    template<class F, class... Args>
    std::future<typename function_result_type<F, Args...>::type>
            enqueue_async(F f, Args... args)
    {
        std::cout << "async enqueued" << std::endl;

        typedef typename function_result_type<F, Args...>::type func_result_t;
        typedef std::promise<func_result_t> promise_t;

        promise_t promise;
        auto future(promise.get_future());

        //! ここで作成したスレッドは誰も知らなくなるが、
        //! futureオブジェクトが呼び出し元に返ることから、
        //! futureオブジェクトの待機によってこのスレッドの完了を保証できる。
        std::thread(
            [this]( promise_t &&promise,
                    F &&f,
                    Args &&... args ) mutable
            {
                task_queue_.enqueue(
                    hwm::make_unique<task_impl<F, Args...>>(
                        std::move(promise),
                        std::forward<F>(f),
                        std::forward<Args>(args)...
                        ) );
            },
            std::move(promise),
            std::forward<F>(f),
            std::forward<Args>(args)...
            ).detach();

        return future;
    }

private:
    locked_queue<task_ptr_t>    task_queue_;
    std::vector<std::thread>    threads_;
    hwm::spin_lock mutable      mutex_;
    bool                        terminated_flag_;

private:

    void    set_terminate_flag(bool state)
    {
        std::lock_guard<hwm::spin_lock> lock(mutex_);
        terminated_flag_ = state;
    }

    bool    is_terminated() const
    {
        std::lock_guard<hwm::spin_lock> lock(mutex_);
        return terminated_flag_;
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
                            std::cout << "popped by thread[" << i << "]" << std::endl;
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

