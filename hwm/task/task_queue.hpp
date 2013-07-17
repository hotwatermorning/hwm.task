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
        ,   task_count_(0)
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
        ,   task_count_(0)
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

        {
            task_count_lock_t lock(task_count_mutex_);
            ++task_count_;
        }

        try {
            task_queue_.enqueue(std::move(ptask));
        } catch(...) {
            task_count_lock_t lock(task_count_mutex_);
            --task_count_;
            if(task_count_ == 0) {
                c_task_.notify_all();
            }
            throw;
        }

        return future;
    }

    void    wait() const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        c_task_.wait(lock, [this]{ return task_count_ == 0; });
    }

    template<class TimePoint>
    bool    wait_until(TimePoint tp) const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        return c_task_.wait_until(lock, tp, [this]{ return task_count_ == 0; });
    }

    template<class Duration>
    bool    wait_for(Duration dur) const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        return c_task_.wait_for(lock, dur, [this]{ return task_count_ == 0; });
    }

private:
    locked_queue<task_ptr_t>    task_queue_;
    std::vector<std::thread>    threads_;
    std::atomic<bool>           terminated_flag_;
    std::mutex mutable          task_count_mutex_;
    typedef std::unique_lock<std::mutex> task_count_lock_t;
    size_t                      task_count_;
    std::atomic<size_t> mutable waiting_count_;
    std::condition_variable mutable c_task_;

    struct scoped_add
    {
        scoped_add(std::atomic<size_t> &value)
            :   v_(value)
        {
            v_.fetch_add(1);
        }

        ~scoped_add()
        {
            v_.fetch_sub(1);
        }

        scoped_add(scoped_add const &) = delete;
        scoped_add& operator=(scoped_add const &) = delete;

    private:
        std::atomic<size_t> &v_;
    };

private:

    void    set_terminate_flag(bool state)
    {
        terminated_flag_.store(state);
    }

    bool    is_terminated() const
    {
        return terminated_flag_.load();
    }

    bool    is_waiting() const
    {
        return waiting_count_.load() != 0;
    }

    void    setup(int num_threads)
    {
        for(int i = 0; i < num_threads; ++i) {
            std::thread th(
                [this](int /*thread_index (currently unused)*/) {
                    for( ; ; ) {
                        if(is_terminated()) {
                            break;
                        }

                        task_ptr_t task;
                        bool const popped = task_queue_.try_dequeue_for(task, std::chrono::seconds(1));

                        bool should_notify = false;

                        if(popped) {
                            task->run();
                            task_count_lock_t lock(task_count_mutex_);
                            --task_count_;

                            if(is_waiting() && task_count_ == 0) {
                                should_notify = true;
                            }
                        } else {
                            if(is_waiting()) {
                                task_count_lock_t lock(task_count_mutex_);
                                if(task_count_ == 0) {
                                    should_notify = true;
                                }
                            }
                        }

                        if(should_notify) {
                            c_task_.notify_all();
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

