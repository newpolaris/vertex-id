#ifndef __EL_MATH_TYPES_H__
#define __EL_MATH_TYPES_H__

#include "el_vec4.h"

namespace el {

    template <template <typename T> class VECTOR, typename T>
    VECTOR<T> linearBezier(const VECTOR<T>& p0, const VECTOR<T>& p1, T t)
    {
        return mix(p0, p1, VECTOR<T>(t));
    }

    template <template <typename T> class VECTOR, typename T>
    VECTOR<T> quadraticBezier(const VECTOR<T>& p0, const VECTOR<T>& p1, const VECTOR<T>& p2, T t)
    {
        T q = T(1.0) - t;
        return q * q * p0 + T(2) * t * q * p1 + t * t * p2;
    }

    template <template <typename T> class VECTOR, typename T>
    VECTOR<T> cubicBezier(const VECTOR<T>& p0, const VECTOR<T>& p1, const VECTOR<T>& p2, const VECTOR<T>& p3, T t)
    {
        T q = T(1) - t;
        T w0 = q * q * q;
        T w1 = T(3) * q * q * t;
        T w2 = T(3) * q * t * t;
        T w3 = t * t * t;
        return w0 * p0 + w1 * p1 + w2 * p2 + w3 * p3;
    }

    template <template <typename T> class VECTOR, typename T>
    VECTOR<T> cubicBezierSlope(const VECTOR<T>& p0, const VECTOR<T>& p1, const VECTOR<T>& p2, const VECTOR<T>& p3, T t)
    {
        T q = T(1.0) - t;
        return T(3) * q * q * (p1 - p0) + T(6) * q * t * (p2 - p1) + T(3) * t * t * (p3 - p2);
    }

} // namespace el

#endif // __EL_MATH_TYPES_H__
