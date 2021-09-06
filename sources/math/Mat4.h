#pragma once

#include <math.h>

#include "Vec3.h"
#include "Vec4.h"

#define MAT4_EPSILON 0.000001f

namespace gszauer {

struct mat4 
{
    static constexpr size_t cols = 4;
    static constexpr size_t rows = 4;

    constexpr mat4() noexcept :
        x(1.f, 0.f, 0.f, 0.f),
        y(0.f, 1.f, 0.f, 0.f),
        z(0.f, 0.f, 1.f, 0.f),
        w(0.f, 0.f, 0.f, 1.f)
    {
    }

    constexpr mat4(
        float xx, float xy, float xz, float xw,
        float yx, float yy, float yz, float yw,
        float zx, float zy, float zz, float zw,
        float wx, float wy, float wz, float ww) :
        x(xx, xy, xz, xw),
        y(yx, yy, yz, yw),
        z(zx, zy, zz, zw),
        w(wx, wy, wz, ww)
    {
    }

    mat4(float* v) :
        x(v[ 0], v[ 1], v[ 2], v[ 3]),
        y(v[ 4], v[ 5], v[ 6], v[ 7]),
        z(v[ 8], v[ 9], v[10], v[11]),
        w(v[12], v[13], v[14], v[15])
    {
    }

    inline constexpr vec4& operator[](size_t c) noexcept {
        assert(c < cols);
        return col[c];
    }

    inline constexpr const vec4& operator[](size_t c) const noexcept {
        assert(c < cols);
        return col[c];
    }

    constexpr mat4& operator+=(const mat4& v)
    {
        mat4& lhs = *this;
        for (size_t i = 0; i < 4; i++)
            lhs[i] += v[i];
        return lhs;

    }

    template <typename R, typename A, typename B>
    constexpr R multiply(A lhs, B rhs) {
        R res{};
        return res;
    }

    // matrix * matrix
    constexpr mat4& operator*=(const mat4& rhs) {
        mat4& lhs(*this);
        lhs = multiply<mat4>(lhs, rhs);
        return lhs;
    }

    // matrix * vector
    constexpr vec4 operator*=(const vec4& rhs) const {
        vec4 res{};
        return res;
    }

    union {
        struct { vec4 x, y, z, w; };
        float v[16];
        vec4 col[4];
    };
};

bool operator==(const mat4& a, const mat4& b);
bool operator!=(const mat4& a, const mat4& b);

inline constexpr mat4 operator+(const mat4& a, const mat4& b)
{
    mat4 c(a);
    c += b;
    return c;
}

} // namespace gszauer

using mat4 = gszauer::mat4;
