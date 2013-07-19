//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_TASK_TASKQUEUE_HPP
#define HWM_TASK_TASKQUEUE_HPP

#include <cassert>
#include <atomic>
#include <future>
#include <limits>
#include <thread>
#include <utility>

#include "./function.hpp"

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
        ,   wait_before_destructed_(true)
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
        ,   wait_before_destructed_(true)
    {
        assert(thread_limit >= 1);
        assert(queue_limit >= 1);
        setup(thread_limit);
    }

    //! デストラクタ
    //! スレッドの終了を待ってデストラクトする
    //! wait_before_destructed()がtrueの場合、
    //! キューに積まれたタスクをすべて実行してから終了する。
    //! falseの場合、キューに積まれたままのタスクは実行されない。
    //! wait_before_destructed()はデフォルトでtrue
    ~task_queue()
    {
        if(wait_before_destructed()) {
            wait();
        }
        set_terminate_flag(true);
        join_threads();
    }

    //! @brief タスクに新たな処理を追加
    //! @detail locked_queueが空くまで処理をブロックする
    //! @return タスクとshared stateを共有するstd::futureクラスのオブジェクト
    template<class F, class... Args>
    std::future<typename function_result_type<F, Args...>::type>
            enqueue(F&& f, Args&& ... args)
    {
        typedef typename function_result_type<F, Args...>::type result_t;
        typedef std::promise<result_t> promise_t;

        promise_t promise;
        auto future(promise.get_future());

        std::unique_ptr<task_base> ptask =
            make_task(std::move(promise), std::forward<F>(f), std::forward<Args>(args)...);

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

    //! @brief すべてのタスクが実行され終わるのを待機する
    //! @note 待機中にすべてのタスクが実行され、関数が返る場合でも、処理が呼び出し元に戻る間に任意のスレッドから新たなタスクが積まれる可能性がある。
    void    wait() const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        c_task_.wait(lock, [this]{ return task_count_ == 0; });
    }

    //! @brief 指定時刻まですべてのタスクが実行され終わるのを待機する
    //! @param [in] tp std::chrono::time_point型に変換可能な型。
    //! @return すべてのタスクが実行され終わった場合、trueが返る。
    //! @note 待機中にすべてのタスクが実行され、関数がtrueを返す場合でも、処理が呼び出し元に戻る間に任意のスレッドから新たなタスクが積まれる可能性がある。
    template<class TimePoint>
    bool    wait_until(TimePoint tp) const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        return c_task_.wait_until(lock, tp, [this]{ return task_count_ == 0; });
    }

    //! @brief 指定時間内ですべてのタスクが実行され終わるのを待機する
    //! @param [in] tp std::chrono::time_point型に変換可能な型。
    //! @return すべてのタスクが実行され終わった場合、trueが返る。
    //! @note 待機中にすべてのタスクが実行され、関数がtrueを返す場合でも、処理が呼び出し元に戻る間に任意のスレッドから新たなタスクが積まれる可能性がある。
    template<class Duration>
    bool    wait_for(Duration dur) const
    {
        task_count_lock_t lock(task_count_mutex_);
        scoped_add sa(waiting_count_);

        return c_task_.wait_for(lock, dur, [this]{ return task_count_ == 0; });
    }

    //! @brief デストラクタが呼び出された時に、積まれているタスクがすべて実行されるまで待機するかどうかを返す。
    //! @return デストラクタが呼び出された時にすべてのタスクの実行を終了を待機する場合はtrueが返る。
    bool        wait_before_destructed() const
    {
        return wait_before_destructed_.load();
    }

    //! @brief デストラクタが呼び出された時に、積まれているタスクがすべて実行されるまで待機するかどうかを設定する。
    //! @param [in] デストラクタが呼び出された時にすべてのタスクの実行を終了を待機するように設定する場合はtrueを渡す。
    void        set_wait_before_destructed(bool state)
    {
        wait_before_destructed_.store(state);
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
    std::atomic<bool>           wait_before_destructed_;

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
        assert(is_terminated());

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

