//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_SPINLOCK_SPINLOCKGCC_HPP
#define HWM_SPINLOCK_SPINLOCKGCC_HPP

#include <boost/move/move.hpp>
#include <boost/swap.hpp>

namespace hwm {

struct spin_lock
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(spin_lock);

private:
    typedef spin_lock this_type;
    enum {
        UNLOCKED,
        LOCKED
    };

public:
    spin_lock()
    {
        __sync_lock_test_and_set(&is_locked, UNLOCKED);
    }

    spin_lock(BOOST_RV_REF(this_type) rhs)
        :   is_locked(rhs.is_locked)
    {
        rhs.is_locked = UNLOCKED;
    }

    spin_lock &
            operator=(BOOST_RV_REF(this_type) rhs)
    {
        spin_lock(boost::move(rhs)).swap(*this);
        return *this;
    }

    void    swap(this_type &rhs)
    {
        boost::swap(is_locked, rhs.is_locked);
    }

    void    lock()
    {
        //! SpinLock
        while(__sync_val_compare_and_swap(&is_locked, UNLOCKED, LOCKED) == LOCKED)
        {
            //::Sleep(0);
        }
    }

    bool    try_lock()
    {
        //! SpinLock
        return __sync_val_compare_and_swap(&is_locked, UNLOCKED, LOCKED) != LOCKED;
    }

    void unlock     ()
    {
        __sync_lock_test_and_set(&is_locked, UNLOCKED);
    }

private:
    int is_locked;
};

inline
void swap(spin_lock &lhs, spin_lock &rhs)
{
    lhs.swap(rhs);
}

}   //namespace hwm

#endif  //HWM_SPINLOCK_SPINLOCKGCC_HPP
