#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

#define VEC3_EPSILON 0.000001f

namespace gszauer {

template <typename T> struct TVec2;
template <typename T> struct TVec3;

template <typename T>
struct TVec2 {
    union {
        struct {
            float x;
            float y;
        };
        T v[2]{};
    };

    constexpr TVec2() : x(T(0)), y(T(0)) {}
    TVec2(T x) : x(x), y(x) {}
    TVec2(T x, T y) : x(x), y(y) {}
    TVec2(T* fv) : x(fv[0]), y(fv[1]) {}
    TVec2(const TVec3<T>& vec) 
        : v { vec[0], vec[1] }
    {}
};
        
template <typename T> 
constexpr inline TVec2<T> operator+(const TVec2<T>& l, const TVec2<T>& r)
{
    return TVec2<T>(l.x + r.x, l.y + r.y);
}

template <typename T>
constexpr inline TVec2<T> operator-(const TVec2<T>& l, const TVec2<T>& r)
{
    return TVec2<T>{l.x - r.x, l.y - r.y};
}

template <typename T>
constexpr inline TVec2<T> operator*(const TVec2<T>& l, const TVec2<T>& r)
{
    return TVec2<T>{l.x * r.x, l.y * r.y};
}

template <typename T>
constexpr inline TVec2<T> operator/(const TVec2<T>& l, const TVec2<T>& r)
{
    return TVec2<T>{l.x / r.x, l.y / r.y};
}
}

template<typename T> using vector2 = gszauer::TVec2<T>;

using ivec2 = vector2<int>;
using vec2 = vector2<float>;
using float2 = vector2<float>;
using double2 = vector2<double>;
