#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

#include "Vec2.h"

namespace gszauer {

template <typename T> struct TVec3;
template <typename T> struct TVec4;

template <typename T> 
struct TVec3
{
    static constexpr size_t SIZE = 3;
    union {
        T v[SIZE]{};
        struct {
            T x, y, z;
        };
    };
    constexpr TVec3() : v{ T(0.0), T(0.0), T(0.0) } {}
    template <typename A>
    constexpr TVec3(A a) : v{ T(a), T(a), T(a) } {}
    TVec3(T x, T y, T z) :
        x(x), y(y), z(z) {}
    TVec3(T * fv) :
        x(fv[0]), y(fv[1]), z(fv[2]) {}

    inline constexpr T& operator[](size_t i) noexcept {
        assert(i < SIZE);
        return v[i];
    }

    inline constexpr const T& operator[](size_t i) const noexcept {
        assert(i < SIZE);
        return v[i];
    }

    constexpr size_t size() const { return SIZE; }
};

template <typename T>
TVec3<T> operator+(const TVec3<T>& l, const TVec3<T>& r)
{
    return TVec3<T>(l.x + r.x, l.y + r.y, l.z + r.z);
}

template <typename T>
TVec3<T> operator-(const TVec3<T>& l, const TVec3<T>& r)
{
    return TVec3<T>(l.x - r.x, l.y - r.y, l.z - r.z);
}

template <typename T>
TVec3<T> operator*(const TVec3<T>& l, float f)
{
    return TVec3<T>(l.x * f, l.y * f, l.z * f);
}

template <typename T>
TVec3<T> operator*(const TVec3<T>& l, const TVec3<T>& r)
{
    return TVec3<T>(l.x * r.x, l.y * r.y, l.z * r.z);
}

template <typename T>
TVec3<T> operator/(const TVec3<T>& l, const TVec3<T>& r)
{
    return TVec3<T>(l.x / r.x, l.y / r.x, l.z / r.x);
}

template <typename T>
TVec3<T> operator/(const TVec3<T>& l, float f)
{
    return TVec3<T>(l.x / f, l.y / f, l.z / f);
}

template <typename T>
TVec3<T> operator*(float f, const TVec3<T>& l)
{
    return l * f;
}

template <typename T>
TVec3<T> lerp(const TVec3<T>& s, const TVec3<T>& e, float t)
{
    return e * t + s * (1 - t);
}

template <typename T>
TVec3<T> slerp(const TVec3<T>& s, const TVec3<T>& e, float t) {
    TVec3<T> from = normalized(s);
    TVec3<T> to = normalized(e);
    float theta = angle(from, to);
    float sin_theta = sinf(theta);
    float a = sinf((1.0f - t) * theta) / sin_theta;
    float b = sinf(t * theta) / sin_theta;

    return from * a + to * b;
}

template <typename T>
TVec3<T> nlerp(const TVec3<T>& s, const TVec3<T>& e, float t) {
    return normalized(lerp(s, e, t));
}

template <typename T>
T dot(const TVec3<T>& l, const TVec3<T>& r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z;
}

template <typename T>
T lenSq(const TVec3<T>& v) {
    return dot(v, v);
}

template <typename T>
T len(const TVec3<T>& v) {
    float sq = lenSq(v);
    if (sq < VEC3_EPSILON) {
        return 0.0f;
    }
    return sqrtf(sq);
}

template <typename T>
TVec3<T> normalized(const TVec3<T>& v) {
    T invLen = (T)1.0 / len(v);
    return v * invLen;
}

template <typename T>
void normalize(TVec3<T>& v) {
    v = normalized(v);
}

template <typename T>
T angle(const TVec3<T>& l, const TVec3<T>& r)
{
    T lenL = len(l);
    T lenR = len(r);
    if (lenL < VEC3_EPSILON || lenR < VEC3_EPSILON)
        return 0.f;
    T o = dot(l, r);
    return acosf(o / lenL / lenR);
}

template <typename T>
TVec3<T> project(const TVec3<T>& a, const TVec3<T>& b)
{
    float magBSq = lenSq(b);
    if (magBSq < VEC3_EPSILON) {
        return TVec3<T>();
    }
    float scale = dot(a, b) / magBSq;
    return b * scale;
}

template <typename T>
TVec3<T> reject(const TVec3<T>& a, const TVec3<T>& b) {
    TVec3<T> projection = project(a, b);
    return a - projection;
}

template <typename T>
TVec3<T> reflect(const TVec3<T>& a, const TVec3<T>& b) {
    float magBSq = lenSq(b);
    if (magBSq < VEC3_EPSILON) {
        return TVec3<T>();
    }
    float scale = dot(a, b) / magBSq;
    TVec3<T> proj2 = b * (scale * 2);
    return a - proj2;
}

template <typename T>
TVec3<T> cross(const TVec3<T>& l, const TVec3<T>& r) {
    return TVec3<T>(
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x);
}

template <typename T>
bool operator==(const TVec3<T>& l, const TVec3<T>& r)
{
    TVec3<T> diff(l - r);
    return lenSq(diff) < VEC3_EPSILON;
}

template <typename T>
bool operator!=(const TVec3<T>& l, const TVec3<T>& r)
{
    return !(l == r);
}

} // namespace gszauer

template<typename T> using vector3 = gszauer::TVec3<T>;

using ivec3 = vector3<int>;
using vec3 = vector3<float>;
using float3 = vector3<float>;
using double3 = vector3<double>;
