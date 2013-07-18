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

    the_uncopyable_multiplication_man(the_uncopyable_multiplication_man &&) = default;
    the_uncopyable_multiplication_man & operator=(the_uncopyable_multiplication_man &&) = default;

    int    let_me_calculate(int n) const { return n * m_; }

private:
    int m_;
};

int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(std::thread::hardware_concurrency());

    {
        std::future<int> f;
        {
            the_multiplication_man tm(3);

            f = tq.enqueue(
                //! pass a member function.
                &the_multiplication_man::let_me_calculate,
                //! 2nd argument takes an object and it will be used to
                //! invoke a function passed as 1st argument of euqueue.
                tm,
                //! 関数に渡す引数
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! pass a member function.
                &the_multiplication_man::let_me_calculate,
                //! 2nd argument takes an object and it will be used to
                //! invoke a function passed as 1st argument of euqueue.
                &tm,
                //! 関数に渡す引数
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! pass a member function.
                &the_multiplication_man::let_me_calculate,
                //! 2nd argument takes an object and it will be used to
                //! invoke a function passed as 1st argument of euqueue.
                std::ref(tm),
                //! 関数に渡す引数
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    {
        the_multiplication_man tm(3);

        std::future<int> f =
            tq.enqueue(
                //! pass a member function.
                &the_multiplication_man::let_me_calculate,
                //! 2nd argument takes an object and it will be used to
                //! invoke a function passed as 1st argument of euqueue.
                std::cref(tm),
                //! 関数に渡す引数
                10
            );

        std::cout << "calculated value : " << f.get() << std::endl;
    }

    {
        std::future<int> f;

        {
            the_uncopyable_multiplication_man tm(3);
            f = tq.enqueue(
                //! pass a member function.
                &the_uncopyable_multiplication_man::let_me_calculate,
                //! 2nd argument takes an object and it will be used to
                //! invoke a function passed as 1st argument of euqueue.
                std::move(tm),
                //! 関数に渡す引数
                10
            );
        }

        std::cout << "calculated value : " << f.get() << std::endl;
    }
}

