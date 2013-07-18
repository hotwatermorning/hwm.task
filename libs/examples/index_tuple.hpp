#ifndef HWM_INDEXTUPLE_HPP
#define HWM_INDEXTUPLE_HPP

#include <cstddef>

template<size_t... Indecies>
struct index_tuple
{};

template<size_t Index, size_t... Indecies>
struct make_index_tuple_helper
{
    typedef
        typename make_index_tuple_helper<Index-1, Index-1, Indecies...>::type
    type;
};

template<size_t... Indecies>
struct make_index_tuple_helper<0, Indecies...>
{
    typedef
        index_tuple<Indecies...>
    type;
};

template<size_t Size>
struct make_index_tuple
{
    typedef
       typename make_index_tuple_helper<Size>::type
    type;
};

#endif  //HWM_INDEXTUPLE_HPP
