//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_TASK_INVOKETASK_HPP
#define HWM_TASK_INVOKETASK_HPP

namespace hwm {

namespace detail { namespace ns_task {

template<class F, class... Args>
void invoke_task(std::promise<void> &promise, F &&f, Args &&... args)
{
    try {
        f(std::forward<Args>(args)...);
    } catch(...) {
        promise.set_exception(std::current_exception());
    }
}

template<class Ret, class F, class... Args>
void invoke_task(std::promise<Ret> &promise, F &&f, Args &&... args)
{
    try {
        promise.set_value(f(std::forward<Args>(args)...));
    } catch(...) {
        promise.set_exception(std::current_exception());
    }
}

}}  //namespace detail::ns_task

}   //namespace hwm

#endif  //HWM_TASK_INVOKETASK_HPP
