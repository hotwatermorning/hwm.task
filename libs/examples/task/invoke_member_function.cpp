//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <hwm/task/task_queue.hpp>

struct the_multiplication_man
{
    the_multiplication_man(int m)
        :   m_(m)
    {}

    int    let_me_calculate(int n) const { return n * m_; }

private:
    int m_;
};

struct the_uncopyable_multiplication_man
{
    the_uncopyable_multiplication_man(int m)
        :   m_(m)
    {}

    the_uncopyable_multiplication_man(the_uncopyable_multiplication_man const &) = delete;
    the_uncopyable_multiplication_man & operator=(the_uncopyable_multiplication_man const &) = delete;

#if defined(_MSC_VER)
    the_uncopyable_multiplication_man(the_uncopyable_multiplication_man &&rhs)
        :   m_(rhs.m_)
    {}

    the_uncopyable_multiplication_man & operator=(the_uncopyable_multiplication_man &&rhs)
    {
        m_ = rhs.m_;
        return *this;
    }
#else
    the_uncopyable_multiplication_man(the_uncopyable_multiplication_man &&) = default;
    the_uncopyable_multiplication_man & operator=(the_uncopyable_multiplication_man &&) = default;
#endif

    int    let_me_calculate(int n) const { return n * m_; }

private:
    int m_;
};

//! メンバ関数呼び出しのサンプル
int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(std::thread::hardware_concurrency());

    // copy
    {
        std::future<int> f;
        {
            the_multiplication_man tm(3);

            f = tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &the_multiplication_man::let_me_calculate,
                //! 第二引数に呼び出す対象となるオブジェクトを渡す
                tm,
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // pointer
    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &the_multiplication_man::let_me_calculate,
                //! 第二引数に呼び出す対象となるオブジェクトを渡す
                &tm,
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // ref
    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &the_multiplication_man::let_me_calculate,
                //! 第二引数に呼び出す対象となるオブジェクトを渡す
                std::ref(tm),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // cref
    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &the_multiplication_man::let_me_calculate,
                //! 第二引数に呼び出す対象となるオブジェクトを渡す
                std::cref(tm),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // move
    {
        std::future<int> f;

        {
            the_uncopyable_multiplication_man tm(3);
            f = tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &the_uncopyable_multiplication_man::let_me_calculate,
                //! 第二引数に呼び出す対象となるオブジェクトを渡す
                std::move(tm),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }
}

