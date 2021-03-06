//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_EXCEPT_HPP
#define BOOST_JSON_DETAIL_EXCEPT_HPP

#include <stdexcept>

namespace boost {
namespace json {
namespace detail {

inline
std::length_error
object_too_large_exception() noexcept
{
    return std::length_error(
        "object too large");
}

inline
std::length_error
array_too_large_exception() noexcept
{
    return std::length_error(
        "array too large");
}

inline
std::length_error
string_too_large_exception() noexcept
{
    return std::length_error(
        "string too large");
}

inline
std::out_of_range
string_pos_too_large() noexcept
{
    return std::out_of_range(
        "pos > size()");
}

inline
std::length_error
key_too_large_exception() noexcept
{
    return std::length_error(
        "key too large");
}

inline
std::length_error
stack_overflow_exception() noexcept
{
    return std::length_error(
        "stack overflow");
}

} // detail
} // json
} // boost

#endif
