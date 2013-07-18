#ifndef HWM_TASK_FUNCTION_HPP
#define HWM_TASK_FUNCTION_HPP

#include <utility>
#include <functional>

namespace hwm {
namespace detail { namespace ns_task {


template<class Signature>
struct mem_fun_ptr_return_type;

template<class Ret, class Class, class... Args>
struct mem_fun_ptr_return_type<Ret (Class::*)(Args...)>
{
    typedef Ret type;
};

template<class Ret, class Class, class... Args>
struct mem_fun_ptr_return_type<Ret (Class::*)(Args...) const>
{
    typedef Ret type;
};

template<class Ret, class Class, class... Args>
struct mem_fun_ptr_return_type<Ret (Class::*)(Args...) volatile>
{
    typedef Ret type;
};

template<class Ret, class Class, class... Args>
struct mem_fun_ptr_return_type<Ret (Class::*)(Args...) const volatile>
{
    typedef Ret type;
};

template<class F, class... Args>
struct function_result_type
{
    typedef typename std::result_of<F(Args...)>::type type;
};

template<class F, class Class, class... Args>
struct function_result_type<F Class::*, Args...>
{
    typedef typename mem_fun_ptr_return_type<F Class::*>::type type;
};

template<class F>
struct invokable
{
    typedef F function_type;

    explicit
    invokable(F&& f)
        :   f_(std::move(f))
    {}

    explicit
    invokable(F const& f)
        :   f_(f)
    {}

    template<class Ret, class... Args>
    Ret call(Args&&... args) const
    {
        return std::forward<F>(f_)(std::forward<Args>(args)...);
    }

private:
    function_type mutable f_;
};

template<class F, class Class>
struct invokable<F Class::*>
{
    typedef F Class::* function_ptr_t;

    explicit
    invokable(function_ptr_t f)
        :   f_(f)
    {}

    template<class Ret, class... Args>
    Ret call(Args&&... args) const
    {
        auto mf = std::mem_fn(f_);
        return mf(std::forward<Args>(args)...);
    }

    template<class Ret, class... Args>
    Ret call(std::reference_wrapper<Class> ref, Args&&... args) const
    {
        auto mf = std::mem_fn(f_);
        return mf(ref.get(), std::forward<Args>(args)...);
    }

    template<class Ret, class... Args>
    Ret call(std::reference_wrapper<Class const> ref, Args&&... args) const
    {
        auto mf = std::mem_fn(f_);
        return mf(ref.get(), std::forward<Args>(args)...);
    }

private:
    function_ptr_t mutable f_;
};

}}  //::detail::ns_task
}   //::hwm

#endif  //HWM_TASK_FUNCTION_HPP

