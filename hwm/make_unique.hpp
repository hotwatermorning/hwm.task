//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_MAKEUNIQUE_HPP
#define HWM_MAKEUNIQUE_HPP

#include <memory>
#include <type_traits>
#include <utility>

//! http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
//! unique_ptrの構築処理をラップし、newを隠蔽する
//! これによって、呼び出し順に関連するメモリリークの危険を回避できる

namespace hwm {

namespace detail { namespace ns_make_unique {

    template <class T, class... Args>
    std::unique_ptr<T> make_unique_helper(std::false_type, Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <class T, class... Args>
    std::unique_ptr<T> make_unique_helper(std::true_type, Args&&... args) {
         static_assert(std::extent<T>::value == 0,
             "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

         typedef typename std::remove_extent<T>::type U;
         return std::unique_ptr<T>(new U[sizeof...(Args)]{std::forward<Args>(args)...});
    }

}}  //namespace detail::ns_make_unique

template <class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
     return detail::ns_make_unique::make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
}

}   //namespace hwm

#endif  //HWM_MAKEUNIQUE_HPP
