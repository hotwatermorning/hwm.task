#ifndef HWM_SPINLOCK_SPINLOCKWIN_HPP
#define HWM_SPINLOCK_SPINLOCKWIN_HPP

#include <boost/move/move.hpp>

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
        _InterlockedExchange(&is_locked, UNLOCKED);
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
        while(_InterlockedCompareExchange(&is_locked, LOCKED, UNLOCKED) == LOCKED)
        {
            //::Sleep(0);
        }
    }

    bool    try_lock()
    {
        //! SpinLock
        return _InterlockedCompareExchange(&is_locked, LOCKED, UNLOCKED) != LOCKED;
    }

    void unlock     ()
    {
        _InterlockedExchange(&is_locked, UNLOCKED);
    }

private:
    long    is_locked;
};

inline
void swap(spin_lock &lhs, spin_lock &rhs)
{
    lhs.swap(rhs);
}

}   //namespace hwm

#endif  //HWM_SPINLOCK_SPINLOCKWIN_HPP
