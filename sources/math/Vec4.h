#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

#include "Vec2.h"
#include "Vec3.h"

namespace gszauer {

template <typename T>
struct TVec4 {

    static constexpr size_t SIZE = 4;

    union {
        T v[SIZE]{};
        TVec2<T> xy, st, rg;
        TVec3<T> xyz, stp, rgb;
        struct {
            union { T x, s, r; };
            union {
                TVec2<T> yz;
                TVec3<T> yzw;
                struct {
                    union { T y, t, g; };
                    union {
                        TVec2<T> zw, pq, ba;
                        struct { T z, w; };
                        struct { T p, q; };
                        struct { T b, a; };
                    };
                };
            };
        };
    };

    constexpr TVec4() : x(0.0), y(0.0), z(0.0), w(0.0) {}
    template <typename A>
    constexpr TVec4(A x) 
        : v{ T(x), T(x), T(x), T(x) } {}
    template <typename A, typename B, typename C, typename D>
    constexpr TVec4(A x, B y, C z, D w) 
        : x(T(x)), y(T(y)), z(T(z)), w(T(w)) {}
    template <typename A>
    constexpr TVec4(A* fv) 
        : x(T(fv[0])), y(T(fv[1])), z(T(fv[2])), w(T(fv[3])) {}

    inline constexpr const T& operator[](size_t i) const noexcept {
        assert(i < SIZE);
        return v[i];
    }

    inline constexpr T& operator[](size_t i) noexcept {
        assert(i < SIZE);
        return v[i];
    }

    template <typename U>
    constexpr TVec4<T>& operator+=(const TVec4<U>& u) {
        TVec4<T>& lhs = *this;
        for (size_t i = 0; i < 4; i++)
            lhs[i] += u[i];
        return lhs;
    }

    template <typename U>
    constexpr TVec4<T>& operator-=(const TVec4<U>& u) {
        TVec4<T>& lhs = *this;
        for (size_t i = 0; i < 4; i++)
            lhs[i] -= u[i];
        return lhs;
    }

    template <typename U>
    constexpr TVec4<T>& operator*=(const TVec4<U>& u) {
        TVec4<T>& lhs = *this;
        for (size_t i = 0; i < 4; i++)
            lhs[i] *= u[i];
        return lhs;
    }
};

template <typename T>
inline constexpr 
TVec4<T> operator+(const TVec4<T>& l, const TVec4<T>& r)
{
    TVec4<T> res(l);
    res += r;
    return res;
}

template <typename T>
inline constexpr 
TVec4<T> operator-(const TVec4<T>& l, const TVec4<T>& r)
{
    TVec4<T> res(l);
    res -= r;
    return res;
}

template <typename T>
inline constexpr 
TVec4<T> operator*(const TVec4<T>& l, const TVec4<T>& r)
{
    TVec4<T> res(l);
    res *= r;
    return res;
}

template <typename T>
class Bezier {
public:
    T P1; // Point 1
    T C1; // Control 1
    T P2; // Point 2
    T C2; // Control 2
};

template <typename T>
inline T interpolate(const Bezier<T>& curve, float t) {
    T A = lerp(curve.P1, curve.C1, t);
    T B = lerp(curve.C2, curve.P2, t);
    T C = lerp(curve.C1, curve.C2, t);
    T D = lerp(A, C, t);
    T E = lerp(C, B, t);
    T R = lerp(D, E, t);
    return R;
}

} // namespace gszauer

template<typename T> using vector4 = gszauer::TVec4<T>;
using ivec4 = vector4<int>;
using vec4 = vector4<float>;
using float4 = vector4<float>;
using double4 = vector4<double>;

