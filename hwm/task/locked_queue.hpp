//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_TASK_LOCKEDQUEUE_HPP
#define HWM_TASK_LOCKEDQUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

#include <boost/optional.hpp>

namespace hwm {

namespace detail { namespace ns_task {

//! Producer/Comsumerパターンを実現する
template <class T>
struct locked_queue {

    //! デフォルトコンストラクタ
    locked_queue()
        :   capacity((std::numeric_limits<size_t>::max)())
    {}

    //! コンストラクタ
    //! @param capacity 同時にキュー可能な最大要素数
    explicit
    locked_queue(capacity capacity)
        :   capacity(capacity)
    {}

    //! @brief キューに要素を追加する。
    //! @detail キューの要素数がcapacityを越えている場合は、
    //! dequeueの呼び出しによって要素が取り除かれるまで処理をブロックする
    //! @param x キューに追加する要素。
    void enqueue(T x) {
        std::unique_lock<std::mutex> lock(m);
        c_enq.wait(lock, [this] { return data.size() != capacity; });
        data.push_back(std::move(x));
        c_deq.notify_one();
    }

    //! @brief キューから値を取り出せるか、指定時刻まで試行する。
    //! @param t キューから取り出した値をムーブ代入で受け取るオブジェクト
    //! @param tp いつまでdequeue処理を試行するかを指定するオブジェクト。std::chrono::time_point型に変換可能でなければならない。
    //! @return 取り出しに成功した場合はtrueが帰る。
    template<class TimePoint>
    bool try_dequeue_until(T &t, TimePoint tp)
    {
        std::unique_lock<std::mutex> lock(m);
        bool const succeeded = 
            c_deq.wait_until(lock, tp, [this] { return !data.empty(); });

        if(succeeded) {
            t = std::move(data.front());
            data.pop_front();
            c_enq.notify_one();
        }

        return succeeded;
    }

    //! @brief キューから値を取り出せるか、指定時間だけ試行する。
    //! @param t キューから取り出した値をムーブ代入で受け取るオブジェクト
    //! @param tp いつまでdequeue処理を試行するかを指定するオブジェクト。std::chrono::duration型に変換可能でなければならない。
    //! @return 取り出しに成功した場合はtrueが帰る。
    template<class Duration>
    bool try_dequeue_for(T &t, Duration dur)
    {
        return try_dequeue_until(
                t,
                std::chrono::steady_clock::now() + dur);
    }

    //! @brief キューから値を取り出す。
    //! @detail キューが空の場合は、要素が取得できるまで処理をブロックする。
    T dequeue() {
        std::unique_lock<std::mutex> lock(m);
        c_deq.wait(lock, [this] { return !data.empty(); });

        T ret = data.front();
        data.pop_front();
        c_enq.notify_one();

        return ret;
    }

private:
    std::mutex      m;
    std::deque<T>   data;
    size_t          capacity;
    std::condition_variable c_enq;
    std::condition_variable c_deq;
};

}}  //namespace detail::ns_task

}   //namespace hwm

#endif  //HWM_TASK_LOCKEDQUEUE_HPP
