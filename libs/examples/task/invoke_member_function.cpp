//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <hwm/task/task_queue.hpp>

struct multiply
{
    multiply(int m)
        :   m_(m)
    {}

    int    do_calculate(int n) const { return n * m_; }

private:
    int m_;
};

struct multiply_uncopyable
{
    multiply_uncopyable(int m)
        :   m_(m)
    {}

    multiply_uncopyable(multiply_uncopyable const &) = delete;
    multiply_uncopyable & operator=(multiply_uncopyable const &) = delete;

#if defined(_MSC_VER)
    multiply_uncopyable(multiply_uncopyable &&rhs)
        :   m_(rhs.m_)
    {}

    multiply_uncopyable & operator=(multiply_uncopyable &&rhs)
    {
        m_ = rhs.m_;
        return *this;
    }
#else
    multiply_uncopyable(multiply_uncopyable &&) = default;
    multiply_uncopyable & operator=(multiply_uncopyable &&) = default;
#endif

    int    do_calculate(int n) const { return n * m_; }

private:
    int m_;
};

//! メンバ関数呼び出しのサンプル

int main()
{
    hwm::task_queue tq(1);

    // copy
    {
        std::future<int> f;
        {
            multiply m(3);

            f = tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &multiply::do_calculate,
                //! 第二引数にそのメンバ関数を呼び出すオブジェクトをコピーして渡せる
                m,
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // pointer
    {
        multiply m(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &multiply::do_calculate,
                //! 第二引数にそのメンバ関数を呼び出すオブジェクトのポインタを渡せる
                &m,
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // ref
    {
        multiply m(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &multiply::do_calculate,
                //! 第二引数にそのメンバ関数を呼び出すオブジェクトの参照を渡せる
                //! 参照は直接渡せないので、std::ref()にくるむ
                std::ref(m),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // cref
    {
        multiply m(3);

        std::future<int> f =
            tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &multiply::do_calculate,
                //! 第二引数にそのメンバ関数を呼び出すオブジェクトのconst参照を渡せる
                //! const参照は直接渡せないので、std::cref()にくるむ
                std::cref(m),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    // move
    {
        std::future<int> f;

        {
            multiply_uncopyable m(3);
            f = tq.enqueue(
                //! 第一引数にメンバ関数を渡す
                &multiply_uncopyable::do_calculate,
                //! 第二引数にそのメンバ関数を呼び出すオブジェクトをmoveして渡せる
                std::move(m),
                //! 続く引数はメンバ関数の呼び出し時に適用される
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }
}

