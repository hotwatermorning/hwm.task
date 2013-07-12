#ifndef HWM_TASK_TASKIMPL_HPP
#define HWM_TASK_TASKIMPL_HPP

#include <future>
#include <boost/assert.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/iteration/local.hpp>

#include "../function_result_type.hpp"
#include "./task_base.hpp"

namespace hwm {

namespace detail { namespace ns_task {


#define HWM_TASK_template_parameters(z, n, unused)              BOOST_PP_CAT(class Arg, n) = void
#define HWM_TASK_template_parameters_specialized(z, n, unused)  BOOST_PP_CAT(class Arg, n)
#define HWM_TASK_define_member_variables(z, n, unused)          BOOST_PP_CAT(Arg, n) BOOST_PP_CAT(BOOST_PP_CAT(arg, n), _);
#define HWM_TASK_initialize_member_variables(z, n, unused)      BOOST_PP_CAT(BOOST_PP_CAT(arg, n), _)( std::forward<BOOST_PP_CAT(Arg, n)>(BOOST_PP_CAT(arg, n)) )
#define HWM_TASK_apply_member_variables(z, n, unused)           std::forward<BOOST_PP_CAT(Arg, n)>(BOOST_PP_CAT(BOOST_PP_CAT(arg, n), _))
#define HWM_TASK_default_params(z, n, unused)                   void

//! タスクの実体クラス
//! Boost.Preprocessorを用いて、10引数を取るタスクまでをサポート
template<class F BOOST_PP_ENUM_TRAILING(11, HWM_TASK_template_parameters, unused)>
struct task_impl;

#define BOOST_PP_LOCAL_MACRO(iteration_value) \
    template<class F BOOST_PP_ENUM_TRAILING(iteration_value, HWM_TASK_template_parameters_specialized, unused)>\
    struct task_impl<F BOOST_PP_ENUM_TRAILING_PARAMS(iteration_value, Arg) /*BOOST_PP_ENUM_TRAILING(BOOST_PP_SUB(10, iteration_value), HWM_TASK_default_params, unused) */>\
        :  task_base \
    {\
        typedef typename function_result_type<F BOOST_PP_ENUM_TRAILING_PARAMS(iteration_value, Arg)>::type result_type;\
        typedef std::promise<result_type> promise_type;\
        task_impl(promise_type && promise, F && f BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(iteration_value, Arg, &&arg))\
            :   promise_(boost::move(promise))\
            ,   f_(std::forward<F>(f))\
            BOOST_PP_ENUM_TRAILING(iteration_value, HWM_TASK_initialize_member_variables, unused)\
        {}\
    private:\
        task_impl(task_impl const &) = delete;\
        task_impl &  operator=(task_impl const &) = delete;\
        promise_type    promise_;\
        F               f_;\
        BOOST_PP_REPEAT(iteration_value, HWM_TASK_define_member_variables, unused)\
        virtual\
        void    run() override final\
        {\
            try {\
                promise_.set_value(f_(BOOST_PP_ENUM(iteration_value, HWM_TASK_apply_member_variables, unused)));\
            } catch(...) {\
                promise_.set_exception(std::current_exception());\
            }\
        }\
    };\
    /**/

#define BOOST_PP_LOCAL_LIMITS (0, 10)

#include BOOST_PP_LOCAL_ITERATE()

#undef HWM_TASK_template_parameters
#undef HWM_TASK_template_parameters_specialized
#undef HWM_TASK_define_member_variables
#undef HWM_TASK_initialize_member_variables
#undef HWM_TASK_apply_member_variables
#undef HWM_TASK_default_params

}}  //namespace detail::ns_task

}   //namespace hwm

#endif  //HWM_TASK_TASKIMPL_HPP

