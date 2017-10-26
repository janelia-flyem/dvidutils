#ifndef DVIDUTILS_UTILS_HPP
#define DVIDUTILS_UTILS_HPP

#include <cstdint>
#include "xtensor/xvectorize.hpp"

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

typedef float float32_t;
typedef double float64_t;


template <typename R, typename T>
inline R cast_element(T x)
{
    return static_cast<R>(x);
}

// Casts an xexpression to a different dtype
// https://github.com/QuantStack/xtensor-python/issues/116#issuecomment-339661536
template <typename result_value_type, typename xpr_t>
auto xcast(xpr_t && xpr)
{
    auto cast = cast_element<result_value_type, typename xpr_t::value_type>;
    return xt::vectorize(cast)(xpr);
}

#endif // DVIDUTILS_UTILS_HPP
