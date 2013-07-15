//          Copyright hotwatermorning 2013 - 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef HWM_FUNCTIONRESULTYPTE_HPP
#define HWM_FUNCTIONRESULTYPTE_HPP

#include <type_traits>

namespace hwm {

//! 関数の戻り値を返すメタ関数
template<class F, class... Args>
struct function_result_type
{
    typedef typename std::result_of<F(Args...)>::type type;
};

}   //namespace hwm

#endif  //HWM_FUNCTIONRESULTYPTE_HPP

