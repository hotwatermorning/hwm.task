//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_SPINLOCK_SPINLOCK_HPP
#define HWM_SPINLOCK_SPINLOCK_HPP

#if defined(_WIN32) && defined(_MSC_VER)
    #include "spin_lock_win.hpp"
#elif defined(__GNUC__)
    #include "spin_lock_gcc.hpp"
#else
    #error unspported
#endif

#endif  //HWM_SPINLOCK_SPINLOCK_HPP
