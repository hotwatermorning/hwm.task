//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <iostream>
#include <sstream>

#include <hwm/task/task_queue.hpp>
#include "../utils/stream_mutex.hpp"

//! $B%?%9%/%-%e!<$,GK4~$5$l$k:]$K!"@Q$^$l$F$$$k%?%9%/$,$9$Y$F<B9T$5$l$k$N$rBT$D$+!"(B
//! $B$^$@<h$j=P$5$l$F$$$J$$%?%9%/$OGK4~$7$F!"$=$N$^$^%?%9%/%-%e!<$rGK4~$9$k$+$r@_Dj$9$k!"(B
//! set_wait_before_destructed()$B%a%s%P4X?t$N%5%s%W%k(B

int main()
{
    hwm::mcout << ">>> wait before destructed" << std::endl;

    {
        hwm::task_queue tq(1);

        tq.enqueue([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
            });

        tq.enqueue([]() {
            hwm::mcout << "this task may be executed" << std::endl;
            });
    }

    hwm::mcout << "<<< wait before destructed" << std::endl;

    hwm::mcout << ">>> don't wait before destructed" << std::endl;
    {
        hwm::task_queue tq(1);

        tq.enqueue([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
            });

        tq.enqueue([]() {
            hwm::mcout << "this task may not be executed" << std::endl;
            });

        tq.set_wait_before_destructed(false);
    }
    hwm::mcout << "<<< don't wait before destructed" << std::endl;
}

